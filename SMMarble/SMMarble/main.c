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

// player 배열방식 모두 삭제예정
/*
static int player_pos[MAX_PLAYER];
static int player_credit[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
static int player_energy[MAX_PLAYER];
*/

void generatePlayers(int n, int initEnergy); //generate a new player
void printPlayerStatus(void); //print all player status at the beginning of each turn
void goForward(int player, int step);

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, const char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif



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

void goForward(int player, int step)
{
    printf("start from %i(%s) (%i)\n",
           smm_players[player].pos,
           getNodeNameByPos(smm_players[player].pos),
           step);

    for (int i = 0; i < step; i++) {
        smm_players[player].pos =
            (smm_players[player].pos + 1) % board_nr;

        printf("  => moved to %i(%s)\n",
               smm_players[player].pos,
               getNodeNameByPos(smm_players[player].pos));
    }
}

void printPlayerStatus(void)
{
    for (int i = 0; i < player_nr; i++) {
        printf("%s - position:%i(%s), credit:%i, energy:%i\n",
               smm_players[i].name,
               smm_players[i].pos,
               getNodeNameByPos(smm_players[i].pos),
               smm_players[i].credit,
               smm_players[i].energy);
    }
}

void generatePlayers(int n, int initEnergy)
{
    // [MOD] 플레이어 상태를 하나의 구조체 배열로 관리
    smm_players = (smm_player_t*)malloc(sizeof(smm_player_t) * n);

    for (int i = 0; i < n; i++) {
        smm_players[i].pos = 0;
        smm_players[i].credit = 0;
        smm_players[i].energy = initEnergy;

        printf("Input %i-th player name: ", i);
        scanf("%s", smm_players[i].name);
    }
}

int rolldie(int player)
{
    int c;

    printf(" Press Enter to roll a die (press g then Enter to see grade): ");

    c = getchar();

    // [MOD] 엔터만 눌렀다면 OK
    // [MOD] g를 눌렀다면 나머지 줄(\n)까지 버퍼 비우기
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
#if 0
//action code when a player stays at a node
void actionNode(int player)
{
    void *ptr = smmdb_getData(LISTNO_NODE,sm_players[player].pos);
    int type = smmObj_getNodeType(sm_player)
    int credit = smmObj_getNodeCredit
    int energy = smmObj_getNodeEnergy
    
    printf();
    
    switch(type)
    {
            //case lecture:
        case SMMNODE_TYPE_LECTURE;
            if(findGrade(,)!=NULL)
                smm_players[player].credit += credit;
            smm_players[player].energy -= energy;
            
            grade = rand()%SMMNODE_MAX_GRADE;
            gradePtr = smmObj_genObject(smmObj_getObjectName(ptr),SMNODE_OBJTYPE_GRADE, type, credit, energy,grade)
            smmdb_addTail(LISTNO_OFFSET)GRADE+player,gradePtr);
            break;
    }
}
#endif




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

            // [MOD] board를 배열이 아닌 DB(list)에 저장
            void* node = smmObj_genObject(
                name,
                OBJTYPE_BOARD,   // objType
                type,
                credit,
                energy,
                0                // grade unused
            );
            smmdb_addTail(LISTNO_NODE, node);
            board_nr++;
        }
        fclose(fp);

    printf("Total number of board nodes : %i\n", board_nr);
    
    
#if 0
    //2. food card config
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    //여기 업데이트 안된부분 업데이트 해놓기.
    printf("\n\nReading food card component......\n");
    while () //read a food parameter set
{
    //store the parameter set
    ptr = smmObj_ //여기는 11월 마지막주 수업 내용 참고
    smm_board_nr = smmdb_addTail(LISTNO_NODE, ptr); //linkedlist에 데이터를 넣는 코드
    
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
}
    
    
    //3. festival card config
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while () //read a festival card string
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
    
#endif
    
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

    }
    while (player_nr <= 0 || player_nr > MAX_PLAYER);
    
    int initEnergy = 0;
    {
        void* startNode = smmdb_getData(LISTNO_NODE, 0);
        if (startNode != NULL) {
            initEnergy = smmObj_getNodeEnergy(startNode);
        }
    }
    
    generatePlayers(player_nr, initEnergy);
    
 //   generatePlayers(sum_player_nr, smmObj_getNodeEnergy(0));// 이미 계산한 initEnergy 사용예정
    // FIX: 의미 없고 문법 깨지는 줄 삭제
    // smmdb_getData(SMMNODE_OBJTYPE,0)
    cnt = 0;
    turn = 0;
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (cnt < 5) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
        //pos = pos + 2;
        
        //4-4. take action at the destination node of the board
        //actionNode();
        
        //4-5. next turn
        cnt++;
        turn = (turn + 1)%player_nr;
    }
    free(smm_players);
    system("PAUSE");
    return 0;
}
