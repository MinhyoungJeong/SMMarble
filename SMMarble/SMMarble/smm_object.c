#include "smm_common.h"
#include "smm_object.h"
#include <string.h>
#include <stdlib.h>

#define MAX_NODENR        100 //노드 개수 최댓값
#define MAX_NODETYPE      7 // 노드 타입 종류 개수 (lecture~festival까지 7종)
#define MAX_GRADE         13 // 성적 종류 개수 (A+ ~ F까지 13종)

// 오브젝트 타입 구분용 상수
#define OBJTYPE_BOARD 0 // 보드 칸(노드)
#define OBJTYPE_GRADE 1 // 성적
#define OBJTYPE_FOOD 2 // 음식 카드
#define OBJTYPE_FEST 3 // 축제 카드

/* static char smmNodeName[MAX_NODETYPE][MAX_CHARNAME]
: 노드타입 종류개수 7개, 각 문자열은 최대 MAX_CHARNAME 길이까지 저장 가능
*/
 static char smmNodeName[MAX_NODETYPE][MAX_CHARNAME] = {
       "lecture",
       "restaurant",
       "laboratory",
       "home",
       "gotoLab",
       "foodChance",
       "festival"
};

//성적 이름 테이블 : GradeType enum 값(0~12)을 문자열(A+, A0)로 변환
static char smmObj_gradeName[MAX_GRADE][MAX_CHARNAME] = {
    "A+",
    "A0",
    "A-",
    "B+",
    "B0",
    "B-",
    "C+",
    "C0",
    "C-",
    "D+",
    "D0",
    "D-",
    "F",
};


// 노드,카드,성적을 한번에 관리하는 Object

typedef struct {
    char name[MAX_CHARNAME];
    int objType;
    int type;
    int credit;
    int energy;
    GradeType grade;
} smmObj_object_t;


// grade가 음수이거나, MAX_GRADE 이상이면 잘못된 값이므로 INVALID 반환
const char* smmObj_getGradeName(GradeType grade)
{
    if (grade < 0 || grade >= MAX_GRADE) return "INVALID";
    return smmObj_gradeName[grade];
}



// void* smmObj_genObject: 구조체 생성 후 초기화
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, GradeType grade)
{
    smmObj_object_t* ptr;
    ptr = (smmObj_object_t*)malloc(sizeof(smmObj_object_t));
    strcpy(ptr->name, name); //함수 인자로 들어온 name 문자열의 내용을 새 구조체의 name에 복사
    ptr->type = type;
    ptr->objType = objType;
    ptr->credit = credit;
    ptr->energy = energy;
    ptr->grade = grade;
    return ((void*)ptr);
}

/*
내부 캐스팅 함수
DB에서 꺼내온 obj는 void* 형태이기 때문에
실제로는 smmObj_object_t* 이므로, 내부에서 캐스팅해서 쓰기 위한 함수
캐스팅을 중복으로 계속 사용해야하는 것을 막기 위해 함수로 따로 분리
*/

//내부 캐스팅 함수: void* -> smmObj_object_t*
static smmObj_object_t* castObj(void* obj)
{
    return (smmObj_object_t*)obj;
}


// getter 함수들 (위의castObj 함수로 캐스팅하도록 설계)
const char* smmObj_getObjectName(void* obj)
{
    return castObj(obj)->name;
}

int smmObj_getNodeType(void* obj)
{
    return castObj(obj)->type;
}

int smmObj_getNodeCredit(void* obj)
{
    return castObj(obj)->credit;
}

int smmObj_getNodeEnergy(void* obj)
{
    return castObj(obj)->energy;
}

int smmObj_getObjectType(void* obj)
{
    return castObj(obj)->objType;
}

GradeType smmObj_getGrade(void* obj)
{
    return castObj(obj)->grade;
}
//여기까지 getter 함수들
