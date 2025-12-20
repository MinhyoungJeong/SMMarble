//
//  smm_common.h
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_common_h
#define smm_common_h

#include <stdio.h>
#include <stdlib.h>
#define MAX_CHARNAME                200 // 문자열 최대 길이
#define GRADUATE_CREDIT             30 // 졸업 조건 기준 학점
#define MAX_DIE                     6 // 주사위 최대 눈의 수
#define MAX_PLAYER                  10 //최대 플레이어 수

typedef struct {
    int pos;
    int credit;
    int energy;
    char name[MAX_CHARNAME];
} smm_player_t; // 이 구조체는 플레이어 한 명에 대한 정보를 저장


#endif /* smm_common_h */

