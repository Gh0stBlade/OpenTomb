#ifndef AI_H
#define AI_H

#include "entity.h"
#include "path.h"

void AI_UpdateEntity(entity_p entity);
void AI_UpdateWolf(entity_p entity);
void AI_UpdateBear(entity_p entity);
void AI_UpdateBat(entity_p entity);
void AI_UpdateCroc(entity_p entity);
void AI_UpdateCroc2(entity_p entity);
void AI_UpdateLion(entity_p entity);
void AI_MoveEntity(entity_p entity, entity_p target_entity, CPathFinder* path, uint32_t flags);

enum tr1Enemy
{
    UNUSED_00,
    UNUSED_01,
    UNUSED_02,
    UNUSED_03,
    UNUSED_04,
    UNUSED_05,
    UNUSED_06,
    WOLF,
    BEAR,
    BAT,
    CROC,//LAND
    CROC2,//SWIM
    LION_M,
    LION_F
};

enum AIType
{
    GROUND = (1 << 1),
    WATER  = (1 << 2),
    FLYING = (1 << 3)
};

#endif // AI_H
