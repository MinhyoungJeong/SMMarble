//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

#define OBJTYPE_BOARD 0 // 보드 칸(노드) 오브젝트
#define OBJTYPE_GRADE 1 // 성적 오브젝트
#define OBJTYPE_FOOD  2 // 음식 카드 오브젝트
#define OBJTYPE_FEST  3 // 축제 카드 오브젝트

#define SMMNODE_TYPE_LECTURE                0 // 강의 노드
#define SMMNODE_TYPE_RESTAURANT             1 // 식당 노드
#define SMMNODE_TYPE_LABORATORY             2 // 실험실 노드
#define SMMNODE_TYPE_HOME                   3 // 집 노드
#define SMMNODE_TYPE_GOTOLAB                4 // 실험 노드
#define SMMNODE_TYPE_FOODCHANGE             5 // 음식 찬스 노드
#define SMMNODE_TYPE_FESTIVAL               6 // 축제 노드

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
    GRADE_F,
    GRADE_INVALID //수강실패(에너지부족 or 중복수강일 경우)
} GradeType;

//object generation
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, GradeType grade);
const char* smmObj_getObjectName(void* obj); // 오브젝트 이름(문자열) 반환
int smmObj_getObjectType(void* obj);   // OBJTYPE_BOARD/FOOD/FEST/GRADE 중 하나를 가져오는 get 함수
int smmObj_getNodeType(void* obj);     // 노드 타입 반환: SMMNODE_TYPE_HOME/LECTURE/ 중 하나
int smmObj_getNodeCredit(void* obj); // credit반환:
int smmObj_getNodeEnergy(void* obj); // energy 반환
const char* smmObj_getGradeName(GradeType grade); // GradeType(enum) 을 문자열로 반환 : ex.GRADE_A_PLUS -> "A+"
GradeType smmObj_getGrade(void* obj); // 오브젝트가 저장하고 있는 GradeType(enum) 값 반환

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
