//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

#define OBJTYPE_BOARD 0
#define OBJTYPE_GRADE 1
#define OBJTYPE_FOOD  2
#define OBJTYPE_FEST  3
//object.c에 있던 enum을 헤더파일로 옮겨옴
typedef enum {
    GRADE_A_PLUS,
    GRADE_A_ZERO,
    GRADE_A_MINUS,
    GRADE_B_PLUS,
    GRADE_B_ZERO,
    GRADE_B_MINUS,
    GRADE_C_PLUS,
    GRADE_C_ZERO,
    GRADE_C_MINUS,
    GRADE_D_PLUS,
    GRADE_D_ZERO,
    GRADE_D_MINUS,
    GRADE_F
} GradeType;

//object generation
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, GradeType grade);

const char* smmObj_getObjectName(void* obj);
int smmObj_getObjectType(void* obj);   // objType
int smmObj_getNodeType(void* obj);     // type
int smmObj_getNodeCredit(void* obj);
int smmObj_getNodeEnergy(void* obj);
// 잠시 주석처리 int smmObj_getGradeValue(void* obj);


//구현에 있는 함수 원형 헤더에도 선언
GradeType smmObj_getGrade(void* obj);

//member retrieving
//element to string

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/


#endif /* smm_object_h */
