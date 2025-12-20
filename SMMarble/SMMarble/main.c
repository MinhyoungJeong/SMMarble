//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"
#include <stdio.h>
#include <locale.h> //X-code 문자깨짐 이슈로 설정
#define BOARDFILEPATH "marbleBoardConfig.txt" // 보드(노드) 설정 파일 경로
#define FOODFILEPATH "marbleFoodConfig.txt"  // 음식 카드 설정 파일 경로
#define FESTFILEPATH "marbleFestivalConfig.txt" // 축제 카드 설정 파일 경로


//board configuration parameters : static 이므로 main.c 파일 내부에서만 접근 가능!
static int board_nr; // 보드 칸(노드) 총 개수
static int food_nr; // 음식 카드 총 개수
static int festival_nr; // 축제 카드 총 개수
static int player_nr; // 플레이어 수



// actionNode: 플레이어가 현재 밟은 칸(node type)에 따라 행동을 실행
void actionNode(
    smm_player_t* players, int player,
    int* experimenting, int* expSuccess,
    int labPos
);
int rolldie(int player, smm_player_t* players); // rolldie: 플레이어 주사위 굴리기
smm_player_t* generatePlayers(int n, int initEnergy); // generatePlayers: n명의 플레이어를 생성, 메모리 동적할당하고 초기 에너지 설정
void printPlayerStatus(smm_player_t* players, int playerCount, int* experimenting); // printPlayerStatus: 현재 모든 플레이어의 상태(이름,위치,학점,에너지,실험중) 출력
int goForward(smm_player_t* players, int player, int step); // goForward: 주사위 step만큼 한 칸씩 이동시키고, 추후 학점 다 채우고 마지막으로 HOME을 지나갔는지 체크를 위해 (passedHome) 반환
static float gradeToScore(GradeType grade); // gradeToScore: 성적(enum)을 GPA 점수(ex. 4.5)로 변환하는 내부 함수
int isGraduated(smm_player_t* players, int player, int passedHome); // 졸업 조건 확인
void printGrades(int player); // 플레이어 성적표 출력
float calcAverageGrade(int player); // 플레이어 GPA 계산
GradeType takeLecture(smm_player_t* players, int player,
                      const char* lectureName, int credit, int energy); // takeLecture: 강의 노드에서 수강 시도(에너지 있는지/중복으로 들은건 아닌지 체크) 후 성적 생성 및 DB 저장
void* findGrade(int player, const char *lectureName);// findGrade: 이미 수강한 강의인지 확인(있으면 해당 gradeObj 포인터 반환, 없으면 NULL)



/* 함수 시작
GPA 계산 함수
특정 player의 수강 기록(grade 객체들)을 DB에서 꺼내 (점수 x 학점) 을 누적하고 총 학점으로 나눠 평균(GPA)을 구함
*/
float calcAverageGrade(int player)
{
    int count = smmdb_len(LISTNO_OFFSET_GRADE + player);
    if (count == 0) return 0.0f;

    float totalPoints = 0.0f;
    int totalCredits = 0;

    for (int i = 0; i < count; i++) {
        void* gradeObj = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        if (!gradeObj) continue;

        GradeType grade = smmObj_getGrade(gradeObj);
        float point = gradeToScore(grade);          // 학점을 숫자로 변환
        int credit = smmObj_getNodeCredit(gradeObj); // 그 과목 학점

        totalPoints += point * credit;
        totalCredits += credit;
    }

    if (totalCredits == 0) return 0.0f;
    return totalPoints / (float)totalCredits;
}

//static float gradeToScore: 점수 변환을 위한 함수 성적(enum) -> GPA 점수 변환 함수
static float gradeToScore(GradeType grade)
{
    switch (grade) {
        case GRADE_A_PLUS: return 4.5f;
        case GRADE_A_ZERO: return 4.0f;
        case GRADE_A_MINUS: return 3.7f;
        case GRADE_B_PLUS: return 3.5f;
        case GRADE_B_ZERO: return 3.0f;
        case GRADE_B_MINUS: return 2.7f;
        case GRADE_C_PLUS: return 2.5f;
        case GRADE_C_ZERO: return 2.0f;
        case GRADE_C_MINUS: return 1.7f;
        case GRADE_D_PLUS: return 1.5f;
        case GRADE_D_ZERO: return 1.0f;
        case GRADE_D_MINUS: return 0.7f;
        case GRADE_F: return 0.0f;
        default: return 0.0f;
    }
}

/*
강의 수강 함수
강의 노드에서 호출됨
에너지 부족하거나 중복 수강이면 실패(GRADE_INVALID)
성공이면 학점 증가, 에너지 감소, 성적 생성 후 DB에 저장
*/

 GradeType takeLecture(smm_player_t* players, int player,
                      const char* lectureName, int credit, int energy)
{
    // 수강 실패: 에너지 부족
    if (players[player].energy < energy) {
        printf("[LECTURE] Not enough energy: %s\n", lectureName);
        return GRADE_INVALID;
    }

    // 수강 실패: 이미 수강
    if (findGrade(player, lectureName) != NULL) {
        printf("[LECTURE] Already taken: %s\n", lectureName);
        return GRADE_INVALID;
    }

    // 수강 성공
    players[player].credit += credit;
    players[player].energy -= energy;
    GradeType grade = (GradeType)(rand() % (GRADE_C_MINUS + 1));
    void* gradeObj = smmObj_genObject(
        (char*)lectureName, OBJTYPE_GRADE, 0, credit, 0, grade
    );
    smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradeObj);
    printf("[LECTURE] Took %s! grade=%s\n", lectureName, smmObj_getGradeName(grade));
    return grade;
}


/*졸업 체크함수
조건: 학점이 기준 이상 + 이번 이동에서 HOME을 지나가면 (passedHome=1 을 만족하면) 플레이어 인덱스 반환, 아니면 -1
*/
 int isGraduated(smm_player_t* players, int player, int passedHome)
{
    if (players[player].credit >= GRADUATE_CREDIT && passedHome) return player;
    return -1;
}


/*
성적표 출력 함수
해당 플레이어의 LISTNO_OFFSET_GRADE+player 리스트를 돌면서
과목명,학점,성적을 성적표 형태로 출력
마지막에 GPA도 출력
 */

void printGrades(int player)
{
    int len = smmdb_len(LISTNO_OFFSET_GRADE + player);

    printf("============================================================\n");
    printf("| %-18s | %-6s | %-6s |\n", "LECTURE", "CREDIT", "GRADE");
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < len; i++) {
        void* g = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        if (!g) continue;

        GradeType grade = smmObj_getGrade(g);

        printf("| %-18s | %-6d | %-6s |\n",
               smmObj_getObjectName(g),
               smmObj_getNodeCredit(g),
               smmObj_getGradeName(grade));
    }
    printf("============================================================\n");

    float gpa = calcAverageGrade(player);
    printf("GPA (Average): %.2f\n", gpa);
}

/* 실험실 위치찾기
보드 전체를 훑어서 "LABORATORY 노드"가 있는 칸의 인덱스를 반환
없으면 -1 반환 (에러/비정상 상황)
*/

int findLabPosition(void)
{
    for (int i = 0; i < board_nr; i++) {
        void* node = smmdb_getData(LISTNO_NODE, i);
        if (smmObj_getNodeType(node) == SMMNODE_TYPE_LABORATORY) {
            return i;
        }
    }
    return -1; // 없으면 에러
}

/*보드 칸 이름 얻기 함수
pos(칸 인덱스)를 주면 DB에서 해당 노드 오브젝트를 꺼내고 이름 반환
node가 NULL이면 null 문자열 반환
*/

static const char* getNodeNameByPos(int pos)
{
    void* node = smmdb_getData(LISTNO_NODE, pos);
    return node ? smmObj_getObjectName(node) : "(null)";
}

/*
중복 수강 검사 함수
player의 성적 리스트를 돌면서 lectureName과 같은 과목명이 있는지 찾음
있으면 그 grade object 포인터 반환, 없으면 NULL 반환

*/
void* findGrade(int player, const char *lectureName)
{
    int size = smmdb_len(LISTNO_OFFSET_GRADE+player);
    int i;
    for (i=0; i<size; i++) {
        void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);
        if (strcmp(smmObj_getObjectName(ptr), lectureName) == 0) {
            return ptr; //strcmp로 문자열 비교 + const char* 사용 + 못 찾으면 NULL 반환
        }
    }
    return NULL;
}

/* 주사위 개수만큼 보드 위를 한칸씩 이동시키는 함수
step(주사위 눈)만큼 한 칸씩 이동
이동 중 HOME을 밟으면 에너지 회복 + passedHome=1 기록  반환값: 이번 이동에서 HOME을 밟았는지 체크 (추후 학점 다 채우고 졸업시 활용)
*/

int goForward(smm_player_t* players, int player, int step) // 플레이어정보배열, player의 인덱스번호, 주사위 결과
{
    int passedHome = 0;
    printf("start from %i(%s) (%i)\n",
           players[player].pos, // 이동 전 상태 출력: 현재 인덱스, 칸이름, 주사위 눈
           getNodeNameByPos(players[player].pos),
           step);
    
    for (int i = 0; i < step; i++) {
        players[player].pos =
        (players[player].pos + 1) % board_nr; // 루프를 돌 때마다 기존 위치에서 한 칸씩 앞으로 간 위치를 저장
        
        void* node = smmdb_getData(LISTNO_NODE, players[player].pos);
        // 현재 이동한 위치(pos)에 해당하는 노드 객체를 DB(linkedlist)에서 가져옴
        int type = smmObj_getNodeType(node);
        // 해당 노드가 HOME, FOOD, FESTIVAL 등 어떤 타입인지 확인
        
        // HOME 패스 : 에너지 회복 + passedHome 기록
        if (type == SMMNODE_TYPE_HOME) {
            players[player].energy += smmObj_getNodeEnergy(node);
            passedHome = 1;
        }
        
        printf("  => moved to %i(%s)\n",
               players[player].pos,
               getNodeNameByPos(players[player].pos));
    }
    
    return passedHome;  // 이번 이동에서 HOME을 밟았으면 1 (졸업조건 위해 추가 12/15)
    
}

// 플레이어 상태 출력 함수
void printPlayerStatus(smm_player_t* players, int playerCount, int* experimenting)
{
    printf("============================================================\n");
    printf("| %-10s | %-15s | %-11s | %-6s | %-6s |\n",
           "NAME", "POSITION", "EXPERIMENT", "CREDIT", "ENERGY");
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < playerCount; i++) {
        printf("| %-10s | %-15s | %-11s | %-6d | %-6d |\n",
               players[i].name,
               getNodeNameByPos(players[i].pos),
               experimenting[i] ? "YES" : "NO",
               players[i].credit,
               players[i].energy);
    }

    printf("============================================================\n");
}

/* 플레이어 생성 함수
n명의 플레이어 배열을 동적 할당(malloc)으로 만들고
초기 상태(pos=0, credit=0, energy=initEnergy) 세팅
각 플레이어 이름을 입력받아서 저장
완성된 players 배열의 시작 주소를 반환
*/

smm_player_t* generatePlayers(int n, int initEnergy)
{
    smm_player_t* players =
        (smm_player_t*)malloc(sizeof(smm_player_t) * n); // [변경]
    memset(players, 0, sizeof(smm_player_t) * n);   // 쓰레기값이 붙어서 추가
    
    for (int i =0; i < n; i++) {
        players[i].pos = 0;
        players[i].credit = 0;
        players[i].energy = initEnergy;

        printf("Input %i-th player name: ", i);
        scanf("%s", players[i].name);

    }

    return players;
}


//주사위 굴리기 함수
int rolldie(int player, smm_player_t* players)
{
    int c;

    printf("\n ['.' %s's TURN] Press Enter to roll a die (press g then Enter to see grade): ",players[player].name);

    c = getchar();

    //엔터만 눌렀다면 OK
    //g를 눌렀다면 나머지 줄(\n)까지 버퍼 비우기
    if (c == 'g') {
        // g 뒤에 남아있는 입력버퍼 비우기
        while ((c = getchar()) != '\n' && c != EOF) {}
    } else {
        // g가 아니면, 사용자가 뭘 눌렀든 그 줄의 나머지 입력을 비워서 다음 입력에 영향 없게 함
        while (c != '\n' && c != EOF) {
            c = getchar();
        }
    }

    return (rand() % MAX_DIE + 1);
}


/* action code when a player stays at a node
플레이어가 현재 칸에 도착했을 때 실행되는 액션 함수
현재 플레이어 위치(players[player].pos)에 해당하는 노드를 DB에서 꺼냄
그 노드의 type에 따라 강의/식당/실험/카드뽑기/축제 등을 switch 문으로 선택
experimenting[player]: 현재 플레이어가 실험중인지(1 or 0) 체크
expSuccess[player]: 실험 성공 기준(목표 주사위 값)
labPos: 실험실 노드의 위치
*/


void actionNode(smm_player_t* players, int player, int* experimenting, int* expSuccess,int labPos){
    
    // 현재 플레이어 위치에 해당하는 보드노드 오브젝트를 DB에서 꺼내옴
    void* node = smmdb_getData(LISTNO_NODE, players[player].pos);
    
    if (node == NULL) return;
    
    // node에서 필요한 정보 꺼내오기 (getter 사용)
    int type   = smmObj_getNodeType(node); // 노드 타입(강의/식당 등등)
    int credit = smmObj_getNodeCredit(node); // 그 노드에서 얻는 학점
    int energy = smmObj_getNodeEnergy(node); // 그 노드에서 소모하는 에너지
    const char* nodeName = smmObj_getObjectName(node);

    switch (type) // 노드 타입별로 다른 행동 실행
    {
        case SMMNODE_TYPE_LECTURE:
        {
            char choice; // 사용자 선택(y/n) 저장
            printf("###########################################################\n");
            printf("[LECTURE] Are u gonna take this lecture? %s(y/n):\n", nodeName);
            printf("###########################################################\n");
            
            
            scanf(" %c", &choice);

            // y 또는 Y면 수강 진행
            if (choice == 'y' || choice == 'Y') {
                takeLecture(players, player, nodeName, credit, energy);
            } else {
                printf("[LECTURE] dropped.\n"); // 수강 안 함(드랍) -> 학점 변동 없는지 확인 완료
            }
            break;
        }

        case SMMNODE_TYPE_RESTAURANT:
        {
            // 식당: 보충 에너지만큼 더함
            players[player].energy += energy;
            printf("[RESTAURANT] energy +%d\n", energy);
            printf("############################################################\n");
            printf("#                 Wow Delicious!!!!!! >.<                  #\n");
            printf("#                   [RESTAURANT] energy + %d                #\n", energy);
            printf("############################################################\n");
            break;
        }

        case SMMNODE_TYPE_GOTOLAB:
        {
            // 실험 노드: 실험중 전환 + 성공기준값 설정 + 실험실이동
            experimenting[player] = 1; // 실험중 ON
            expSuccess[player] = (rand() % MAX_DIE) + 1; // 목표값 1~6 사이로 설정

            printf("############################################################\n");
            printf("hohoho..!! [EXPERIMENT] get started! goal=%d, move to lab.\n", expSuccess[player]);
            printf("############################################################\n");
            players[player].pos = labPos;
            break;
        }

        case SMMNODE_TYPE_LABORATORY:
        {
            // 실험실: 실험중일 때만
            if (!experimenting[player]) {
                printf("[LAB] not experimenting. nothing happens.\n");
                break;
            }

            // 실험 시도마다 에너지 소모
            players[player].energy -= energy;

            int die = rolldie(player,players);
            printf("[LAB] roll=%d (goal=%d), energy -%d\n", die, expSuccess[player], energy);

            // 성공 조건 : 주사위 값이 목표 이상
            if (die >= expSuccess[player]) {
                experimenting[player] = 0;
                printf("[LAB] success! experiment finished.\n");
            } else {
                printf("[LAB] failed. stay experimenting.\n");
            }
            break;
        }

        case SMMNODE_TYPE_FOODCHANGE:
        {
            // 보충찬스: 음식카드 랜덤 1장 뽑아 에너지 더함
            int len = smmdb_len(LISTNO_FOODCARD);
            if (len <= 0) {
                printf("[FOOD] no food cards loaded.\n"); // 카드가 없으면 아무 것도 할 수 없음
                break;
            }

            int idx = rand() % len;
            void* food = smmdb_getData(LISTNO_FOODCARD, idx);
            int addE = smmObj_getNodeEnergy(food);

            players[player].energy += addE;
            printf("############################################################\n");
            printf("#                 Wow Delicious!!!!!! >.<                  #\n");
            printf("#                [FOOD] got '%s' energy  ~('.')~ +%d      #\n", smmObj_getObjectName(food), addE);
            printf("############################################################\n");
            
            break;
        }

        case SMMNODE_TYPE_FESTIVAL:
        {
            // 축제: 축제카드 랜덤 1장 뽑기
            int len = smmdb_len(LISTNO_FESTCARD);
            if (len <= 0) {
                printf("no festival cards\n"); // 위에랑 똑같이 카드가 없으면 아무 것도 할 수 없음
                break;
            }

            int idx = rand() % len;
            void* fest = smmdb_getData(LISTNO_FESTCARD, idx);
            printf("############################################################\n");
            printf("Festival time ~(~.~)~ Please complete the misson! : %s\n", smmObj_getObjectName(fest));
            printf("############################################################\n");
            break;
        }

        default:
            
            break;
    }
}


int main(int argc, const char * argv[]) {
    
    setlocale(LC_ALL, ""); // 로케일 설정(X-code 오류) : 한글 출력, 특수문자 출력 깨짐 방지
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int cnt;
    int turn;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    srand((unsigned int)time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i",
                      name, &type, &credit, &energy) == 4) {

            // board를 DB(list)에 저장
            void* node = smmObj_genObject(
                name,
                OBJTYPE_BOARD,
                type,
                credit,
                energy,
                0
            );
            smmdb_addTail(LISTNO_NODE, node);
            board_nr++;
        }
        fclose(fp);

    printf("Total number of board nodes : %i\n", board_nr);
    
    
    //2. food card config

    if ((fp = fopen(FOODFILEPATH,"r")) == NULL) //오류나면 아래의 내용 print
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }

    printf("\n\nReading food card component......\n");
    food_nr = 0; //food 객체 갯수 저장용
    while (1)
    {
        
        if (fscanf(fp, "%s %i", name, &energy) != 2) break; // name, energy 2를 반환하지 않으면 break

        void* foodObj = smmObj_genObject(
            name,           // name ok
            OBJTYPE_FOOD,   // objType ok
            0,              // type x
            0,              // credit x
            energy,         // energy ok
            0               // grade x
        );

        smmdb_addTail(LISTNO_FOODCARD, foodObj); // 데이터베이스 리스트에 해당 객체를 추가
        food_nr++; //food 객체 갯수 저장용
    }

    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
    
    //3. festival card config
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }

    printf("\n\nReading festival card component......\n");

    festival_nr = 0;
    while (1)
    {
        // 축제 config파일에서 한줄씩 읽어서 name 변수에 저장
        if (fscanf(fp, "%s", name) != 1) break; //%s 로 한줄씩 읽음
        //name 변수를 사용해서 아래의 구조체를 통해 객체를 하나 만든다.
        void* festObj = smmObj_genObject(
            name,
            OBJTYPE_FEST,   // objType ok
            0,              // type x
            0,              // credit x
            0,              // energy x
            0               // grade x
        );
        // 데이터베이스 리스트에 해당 객체를 추가
        smmdb_addTail(LISTNO_FESTCARD, festObj);
        festival_nr++;
        //festival_nr: 갯수 저장
    }

    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
    

    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        printf("\n");
            printf("############################################################\n");
            printf("#                 WELCOME TO SMMARBLE >.<                  #\n");
            printf("#                 ARE YOU READY FOR SMMarble? 'o'          #\n");
            printf("#                                                          #\n");
            printf("#                  Input following info! ~('.')~           #\n");
            printf("#                                                          #\n");
            printf("############################################################\n");
            printf("\n");
        printf("Input player number:"); // 플레이어 수 입력

        if (scanf("%i", &player_nr) != 1) {
            printf("Invalid input! Please enter a number.\n");

            // 입력 버퍼 비우기 (fflush(stdin) 대체) - 맥에서 작동하지 않음
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        
        // 플레이어 수 범위 검사
        if (player_nr <= 0 || player_nr > MAX_PLAYER)
            printf("Invalid player number!\n");

    } while (player_nr <= 0 || player_nr > MAX_PLAYER);
    
    //시작 칸(0번 노드)의 에너지를 초기 에너지로 설정 (초기화)
    int initEnergy = 0;
    {
        void* startNode = smmdb_getData(LISTNO_NODE, 0);
        if (startNode != NULL) {
            initEnergy = smmObj_getNodeEnergy(startNode);
        }
    }
    
    // 플레이어 생성
    smm_player_t* players = generatePlayers(player_nr, initEnergy);

    // 실험 관련 상태 배열
    int experimenting[MAX_PLAYER] = {0};
    int expSuccess[MAX_PLAYER] = {0};
    int labPos = findLabPosition(); //실험실 위치 찾기
    
    cnt = 0;
    turn = 0;
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1)
    {
        int passedHome = 0;  // 이번 턴에서 HOME을 지나갔는지
        int winner = -1; // 졸업한 플레이어 인덱스
        
        // 현재 상태 출력
        printPlayerStatus(players, player_nr, experimenting);
        // 현재 플레이어의 위치 노드 정보
        void* curNode = smmdb_getData(LISTNO_NODE, players[turn].pos);
        int curType = smmObj_getNodeType(curNode);
        // 만약 실험중이고 실험실이면 이동 없이 바로 action
        if (experimenting[turn] && curType == SMMNODE_TYPE_LABORATORY)
        {
            actionNode(players, turn, experimenting, expSuccess, labPos);
            // 실험실에서 이동 없으니 passedHome는 0 유지
        }
        else
        {
            int die_result = rolldie(turn, players); // 주사위 굴림
            passedHome = goForward(players, turn, die_result);  // 이동하고 HOME 통과 여부 확인
            actionNode(players, turn, experimenting, expSuccess, labPos); // 이동 후 노드 액션 실행
        }
        
        // 졸업 조건 검사
        winner = isGraduated(players, turn, passedHome);
        if (winner >= 0) {
            printf("############################################################\n");
            printf("\nCongratulations! Finally, %s graduated!\n", players[winner].name);
            printf("############################################################\n");
            
            // 졸업자 성적표 출력
            printGrades(winner);
            
            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
            printf("\n                    SEE YOU LATER!                          \n");
            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
            
            break;
        }
        turn = (turn + 1) % player_nr;
    }
    free(players); // 동적 할당한 메모리 해제
    return 0;
}
