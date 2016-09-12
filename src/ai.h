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
void AI_UpdateGorilla(entity_p entity);
void AI_UpdateRat(entity_p entity);
void AI_UpdateRat2(entity_p entity);
void AI_UpdateTrex(entity_p entity);
void AI_UpdateRaptor(entity_p entity);
void AI_UpdateMutantWinged(entity_p entity);
void AI_UpdateMutantCentaur(entity_p entity);
void AI_MoveEntity(entity_p entity, entity_p target_entity, CPathFinder* path, unsigned char flags);

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
    LION_F,
    PANTHER,
    GORILLA,
    RAT,//LAND
    RAT2,//SWIM
    TREX,
    RAPTOR,
    MUTANT_WINGED,
    UNSUSED_21,
    UNUSED_22,
    MUTANT_CENTAUR,
    UNUSED_24,
    UNUSED_25,
    UNUSED_26,
    LARSON,///
    PIERRE,///
};

//Note: Multiple flags can be set for entities that fly/move on ground (TR1 Flying Mutants) and swim/move on ground (TR1 Rats)
enum AIType
{
    GROUND  = (1 << 0),
    WATER   = (1 << 1),
    FLYING  = (1 << 2),
    NUM_AI_TYPES
};

#endif // AI_H
