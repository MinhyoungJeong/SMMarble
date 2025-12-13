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

// 변경: smm_players → players 매개변수
void goForward(smm_player_t* players, int player, int step)
{
    printf("start from %i(%s) (%i)\n",
           players[player].pos,                 // [변경]
           getNodeNameByPos(players[player].pos),
           step);

    for (int i = 0; i < step; i++) {
        players[player].pos =
            (players[player].pos + 1) % board_nr;

        printf("  => moved to %i(%s)\n",
               players[player].pos,
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

            // board를 배열이 아닌 DB(list)에 저장
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

    }
    
    while (player_nr <= 0 || player_nr > MAX_PLAYER);
    
    int initEnergy = 0;
    {
        void* startNode = smmdb_getData(LISTNO_NODE, 0);
        if (startNode != NULL) {
            initEnergy = smmObj_getNodeEnergy(startNode);
        }
    }
    
    //extern 제거
    smm_player_t* players = generatePlayers(player_nr, initEnergy);
    cnt = 0;
    turn = 0;
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (cnt < 5) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus(players, player_nr);
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(players, turn, die_result);

        //pos = pos + 2;
        
        //4-4. take action at the destination node of the board
        //actionNode();
        
        //4-5. next turn
        cnt++;
        turn = (turn + 1)%player_nr;
    }
    free(players);
    return 0;
}
