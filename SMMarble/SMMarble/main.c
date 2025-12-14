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

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"
//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;
static int player_nr;

void actionNode(
    smm_player_t* players, int player,
    int* experimenting, int* expSuccess,
    int labPos
);

// 변경 : players를 매개변수로 받도록 변경
smm_player_t* generatePlayers(int n, int initEnergy);
void printPlayerStatus(smm_player_t* players, int playerCount);
void goForward(smm_player_t* players, int player, int step);

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, const char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif


// 실험실 위치찾기
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

// (FIX) 보드 칸 이름 얻는 함수 (DB에서 꺼내서 getObjectName 호출)
// 강의 실습 14주차 “get 호출 직전에 smmdb_getData()로 구조체를 받아오기”
static const char* getNodeNameByPos(int pos)
{
    void* node = smmdb_getData(LISTNO_NODE, pos);
    return node ? smmObj_getObjectName(node) : "(null)";
}

//findGrade: findGrade는 strcmp로 비교 + const char* 사용 + 못 찾으면 NULL 반환
void* findGrade(int player, char *lectureName)
{
    int size = smmdb_len(LISTNO_OFFSET_GRADE+player);
    int i;
    for (i=0; i<size; i++) {
        void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);
        if (strcmp(smmObj_getObjectName(ptr), lectureName) == 0) {
            return ptr; // (FIX) 모든 경로에서 반환
        }
    }
    return NULL;
}

// 주사위 개수만큼 보드 위를 한칸씩 이동시키는 함수
void goForward(smm_player_t* players, int player, int step) // 플레이어정보배열, player의 인덱스번호, 주사위 결과
{
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

    //  만약 type = HOME에 놓이면 에너지 회복
        if (type == SMMNODE_TYPE_HOME) {
            players[player].energy += smmObj_getNodeEnergy(node);
            // HOME 노드의 energy 값을 가져와 플레이어 에너지에 더함
            //HOME을 지나가면 에너지 회복
        }
    printf("  => moved to %i(%s)\n",
        players[player].pos, // 이동 후의 위치(인덱스), 칸 이름 출력
        getNodeNameByPos(players[player].pos));
    }
}



// 변경 -> players를 인자로 받아 사용
void printPlayerStatus(smm_player_t* players, int playerCount)
{
    for (int i = 0; i < playerCount; i++) {
        printf("%s - position:%i(%s), credit:%i, energy:%i\n",
               players[i].name,                    // [변경]
               players[i].pos,
               getNodeNameByPos(players[i].pos),
               players[i].credit,
               players[i].energy);
    }
}

smm_player_t* generatePlayers(int n, int initEnergy)
{
    smm_player_t* players =
        (smm_player_t*)malloc(sizeof(smm_player_t) * n); // [변경]

    for (int i =0; i < n; i++) {
        players[i].pos = 0;
        players[i].credit = 0;
        players[i].energy = initEnergy;

        printf("Input %i-th player name: ", i);
        scanf("%s", players[i].name);
    }

    return players; // [추가]
}

int rolldie(int player)
{
    int c;

    printf(" Press Enter to roll a die (press g then Enter to see grade): ");

    c = getchar();

    //엔터만 눌렀다면 OK
    //g를 눌렀다면 나머지 줄(\n)까지 버퍼 비우기
    if (c == 'g') {
#if 0
        printGrades(player);
#endif
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

//action code when a player stays at a node
void actionNode(smm_player_t* players, int player, int* experimenting, int* expSuccess,int labPos){
    
    void* node = smmdb_getData(LISTNO_NODE, players[player].pos);
    
    if (node == NULL) return;

    int type   = smmObj_getNodeType(node);
    int credit = smmObj_getNodeCredit(node);
    int energy = smmObj_getNodeEnergy(node);
    const char* nodeName = smmObj_getObjectName(node);

    switch (type)
    {
        case SMMNODE_TYPE_LECTURE:
        {
            // 수강 가능 조건: 에너지 충분 + 이전에 듣지 않은 강의인지 체크
            if (players[player].energy < energy) {
                printf("[LECTURE] not enough energy to take %s.\n", nodeName);
                break;
            }
            if (findGrade(player, (char*)nodeName) != NULL) {
                printf("[LECTURE] already took %s.\n", nodeName);
                break;
            }

            // 수강 or 드랍중 선택
            char choice;
            printf("[LECTURE] %s Are you going to takie this course? (answer y or n) : ", nodeName);
            scanf(" %c", &choice);

            if (choice != 'y' && choice != 'Y') {
                printf("[LECTURE] dropped.\n");
                break;
            }

            // 수강 처리
            players[player].credit += credit;
            players[player].energy -= energy;
            // 성적: A+ ~ C- 랜덤부여
            GradeType grade = (GradeType)(rand() % (GRADE_C_MINUS + 1));
            void* gradeObj = smmObj_genObject(
                (char*)nodeName,
                OBJTYPE_GRADE,
                0,
                credit,   // 학점 기록용
                0,
                grade
            );
            smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradeObj);
            printf("[LECTURE] took %s! credit +%d, energy -%d\n", nodeName, credit, energy);
            break;
        }

        case SMMNODE_TYPE_RESTAURANT:
        {
            // 식당: 보충 에너지만큼 더함
            players[player].energy += energy;
            printf("[RESTAURANT] energy +%d\n", energy);
            break;
        }

        case SMMNODE_TYPE_GOTOLAB:
        {
            // 실험 노드: 실험중 전환 + 성공기준값 설정 + 실험실이동
            experimenting[player] = 1;
            expSuccess[player] = (rand() % MAX_DIE) + 1;

            printf("[EXPERIMENT] start! goal=%d, move to lab.\n", expSuccess[player]);
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

            int die = rolldie(player);
            printf("[LAB] roll=%d (goal=%d), energy -%d\n", die, expSuccess[player], energy);

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
                printf("[FOOD] no food cards loaded.\n");
                break;
            }

            int idx = rand() % len;
            void* food = smmdb_getData(LISTNO_FOODCARD, idx);
            int addE = smmObj_getNodeEnergy(food);

            players[player].energy += addE;
            printf("[FOOD] got '%s' energy +%d\n", smmObj_getObjectName(food), addE);
            break;
        }

        case SMMNODE_TYPE_FESTIVAL:
        {
            // 축제: 축제카드 랜덤 1장 뽑기
            int len = smmdb_len(LISTNO_FESTCARD);
            if (len <= 0) {
                printf("no festival cards\n");
                break;
            }

            int idx = rand() % len;
            void* fest = smmdb_getData(LISTNO_FESTCARD, idx);

            printf("Please mission complete! : %s\n", smmObj_getObjectName(fest));
            break;
        }

        default:
            
            break;
    }
}


int main(int argc, const char * argv[]) {
    
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
        printf("Input player number:");

        if (scanf("%i", &player_nr) != 1) {
            printf("Invalid input! Please enter a number.\n");

            // 입력 버퍼 비우기 (fflush(stdin) 대체) - 맥에서 작동하지 않음
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }

        if (player_nr <= 0 || player_nr > MAX_PLAYER)
            printf("Invalid player number!\n");

    } while (player_nr <= 0 || player_nr > MAX_PLAYER);
    
    int initEnergy = 0;
    {
        void* startNode = smmdb_getData(LISTNO_NODE, 0);
        if (startNode != NULL) {
            initEnergy = smmObj_getNodeEnergy(startNode);
        }
    }
    
    smm_player_t* players = generatePlayers(player_nr, initEnergy);
    
    
    int experimenting[MAX_PLAYER] = {0};
    int expSuccess[MAX_PLAYER] = {0};
    int labPos = findLabPosition();
    
    cnt = 0;
    turn = 0;
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (cnt < 5)   // 졸업 조건은 나중에 변경
    {
        printPlayerStatus(players, player_nr);

        // 현재 위치의 노드 타입담기
        void* curNode = smmdb_getData(LISTNO_NODE, players[turn].pos);
        int curType = smmObj_getNodeType(curNode);

        // 실험중 + 실험실이면 이동 x, 실험만 o
        if (experimenting[turn] && curType == SMMNODE_TYPE_LABORATORY)
        {
            actionNode(players, turn, experimenting, expSuccess, labPos);
        }
        else // 실험중이 아니면 주사위 굴리기
        {
            int die_result = rolldie(turn);
            goForward(players, turn, die_result);
            actionNode(players, turn, experimenting, expSuccess, labPos);
        }

        cnt++;
        turn = (turn + 1) % player_nr;
    }
    free(players);
    return 0;
}
