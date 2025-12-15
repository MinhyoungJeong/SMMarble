#include "smm_common.h"
#include "smm_object.h"
#include <string.h>
#include <stdlib.h>

#define MAX_NODENR        100
#define MAX_NODETYPE      7
#define MAX_GRADE         13

#define OBJTYPE_BOARD 0
#define OBJTYPE_GRADE 1
#define OBJTYPE_FOOD 2
#define OBJTYPE_FEST 3

static char smmNodeName[MAX_NODETYPE][MAX_CHARNAME] = {
       "lecture",
       "restaurant",
       "laboratory",
       "home",
       "gotoLab",
       "foodChance",
       "festival"
};

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

static int smmObj_gradePoint[MAX_GRADE] = {
    0, // A+
    1, // A0
    2, // A-
    3, // B+
    4, // B0
    5, // B-
    6, // C+
    7, // C0
    8, // C-
    9, // D+
    10, // D0
    11, // D-
    12  // F
};

static int smm_nodeNr=0;

typedef struct {
    char name[MAX_CHARNAME];
    int objType;
    int type;
    int credit;
    int energy;
    GradeType grade;
} smmObj_object_t;


const char* smmObj_getGradeName(GradeType grade)
{
    if (grade < 0 || grade >= MAX_GRADE) return "INVALID";
    return smmObj_gradeName[grade];
}


//배열 형태가 아니라 linkedlist - database에 넣기
//object generation
//node->확장시킬예정, 동적메모리형태로 할당하기
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, GradeType grade) // 구조체 생성 후 초기화
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

// (1) 내부 캐스팅 함수: void* -> smmObj_object_t*
static smmObj_object_t* castObj(void* obj)
{
    return (smmObj_object_t*)obj;
}

// (2) 새 getter들: 입력은 무조건 void*
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
