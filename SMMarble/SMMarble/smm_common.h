//
//  smm_common.h
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
#ifndef smm_common_h
#define smm_common_h
#include <stdio.h>
#include <stdlib.h>
#define MAX_CHARNAME                200
#define GRADUATE_CREDIT             30
#define MAX_DIE                     6
#define MAX_PLAYER                  10

typedef struct {
    int pos;
    int credit;
    int energy;
    char name[MAX_CHARNAME];
} smm_player_t;

extern smm_player_t* smm_players;


#endif /* smm_common_h */
