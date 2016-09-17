#include "engine.h"
#include "entity.h"
#include "ai.h"
#include "world.h"
#include "skeletal_model.h"
#include "core/console.h"
#include "vt/tr_versions.h"
#include "mesh.h"
#include "room.h"
#include "path.h"
#include "physics.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletSoftBody/btsoftbodyinternals.h>

#include <cassert>
#include <cmath>

/*
 * Updates specific entity
 */

void AI_UpdateEntity(entity_p entity)
{
    entity_p target_entity = World_GetPlayer();

    ///@TODO Only supporting TR1
    if(World_GetVersion() != TR_I)
    {
        return;
    }

    //We can only continue if entity and targetEntity are valid entities.
    if((entity != NULL) && target_entity != NULL && (entity->state_flags & ENTITY_STATE_ACTIVE))
    {
        CPathFinder* pathFinder = new CPathFinder(entity->current_sector, target_entity->current_sector);
        if(!(Room_IsInNearRoomsList(entity->current_sector->owner_room, target_entity->current_sector->owner_room)))
        {
            return;///@TODO gen escape path!
        }

        switch(entity->bf->animations.model->id)
        {
    case tr1Enemy::WOLF:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateWolf(entity);
        }
        break;
    case tr1Enemy::BEAR:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateBear(entity);
        }
        break;
    case tr1Enemy::BAT:
        {
            pathFinder->FindPath(AIType::FLYING);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::FLYING);
            //AI_UpdateBat(entity);
        }
        break;
    case tr1Enemy::CROC:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateCroc(entity);
        }
        break;
    case tr1Enemy::CROC2:
        {
            if(target_entity->current_sector->owner_room->flags & TR_ROOM_FLAG_WATER)
            {
                pathFinder->FindPath(AIType::WATER);
                AI_MoveEntity(entity, target_entity, pathFinder, AIType::WATER);
                AI_UpdateCroc2(entity);
            }
            else
            {
                ///@TODO Gen escape path!
            }
        }
        break;
    case tr1Enemy::LION_M:
    case tr1Enemy::LION_F:
    case tr1Enemy::PANTHER:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateLion(entity);
        }
        break;
    case tr1Enemy::GORILLA:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateGorilla(entity);
        }
        break;
    case tr1Enemy::RAT:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND | AIType::WATER);
            AI_UpdateRat(entity);
        }
        break;
    case tr1Enemy::RAT2:
        {
            if(target_entity->current_sector->owner_room->flags & TR_ROOM_FLAG_WATER)
            {
                pathFinder->FindPath(AIType::WATER);
                AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND | AIType::WATER);
                AI_UpdateRat2(entity);
            }
            else
            {
                ///@TODO Gen escape path!
            }
        }
        break;
    case tr1Enemy::TREX:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateTrex(entity);
        }
        break;
    case tr1Enemy::RAPTOR:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateRaptor(entity);
        }
        break;
    case tr1Enemy::MUTANT_WINGED:
        {
            pathFinder->FindPath(AIType::FLYING);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::FLYING | AIType::GROUND);
            AI_UpdateMutantWinged(entity);
        }
        break;
    case tr1Enemy::MUTANT_CENTAUR:
        {
            pathFinder->FindPath(AIType::GROUND);
            AI_MoveEntity(entity, target_entity, pathFinder, AIType::GROUND);
            AI_UpdateMutantCentaur(entity);
        }
        break;
    default:
        //Nothing
        break;
        }
        delete pathFinder;
    }
}

void AI_MoveEntity(entity_p entity, entity_p target_entity, CPathFinder* path, unsigned char flags)
{
    assert(entity);
    assert(target_entity);
    assert(path);

    std::vector<CPathNode*> resultPath = path->GetResultPath();
    if(resultPath.size() <= 0)
    {
        return;
    }

    CPathNode* next_node = resultPath[resultPath.size()-1];
    btVector3 startPos, targetPos, resultPos;

    ///This is DISGRACEFUL ;)
    startPos.setX(entity->transform[12]);
    startPos.setY(entity->transform[13]);
    startPos.setZ(entity->transform[14]);

    targetPos.setX(next_node->GetSector()->pos[0]);
    targetPos.setY(next_node->GetSector()->pos[1]);
    targetPos.setZ(next_node->GetSector()->floor);

    if((flags & AIType::FLYING))///@FIXME No! Move state!
    {
        //targetPos.setZ(next_node->GetSector()->floor - next_node->GetSector()->ceiling + 512.0f);
        targetPos.setZ(next_node->GetSector()->floor + 512.0f);
    }

    resultPos = lerp(startPos, targetPos, 2.2f * engine_frame_time);
    entity->transform[12] = resultPos.getX();
    entity->transform[13] = resultPos.getY();

    if((flags & AIType::FLYING))///@FIXME No! Move state!
    {
        entity->transform[14] = resultPos.getZ();
    }

    if((flags & AIType::GROUND) || (flags & AIType::WATER))///Ground entities stay on floor
        entity->transform[14] = next_node->GetSector()->floor;

    CPathNode* parent_node = next_node->GetParentNode();
    if(parent_node != NULL)
    {
        float dx = (next_node->GetSector()->index_x - parent_node->GetSector()->index_x);
        float dy = (next_node->GetSector()->index_y - parent_node->GetSector()->index_y);
        float target_angle = atan2f(dx, dy) * (180.0f/M_PI);
        entity->angles[0] = -target_angle;//Lerp(entity->angles[0], -target_angle, 2.3 * engine_frame_time);
        Entity_UpdateTransform(entity);
    }
}

///@PLACHOLDER
void AI_UpdateWolf(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 8:
            Entity_SetAnimation(entity, 0, 6, 0);///@FIXME illegal state change
         break;
        case 6:
            Entity_SetAnimation(entity, 0, 8, 0);
            break;
        case 3:
            {
                ///ATTACK
                if(World_GetPlayer()->current_sector == entity->current_sector)
                {
                    Entity_SetAnimation(entity, 0, 10, 0);
                }
            }
            break;
        default:
            //Con_Printf("Unimplemented state: %i", entity->bf->animations.next_state);
            break;
        }
    }
}

///@TODO
void AI_UpdateBear(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 3, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateBat(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 2:
            break;
        default:
            Entity_SetAnimation(entity, 0, 2, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateCroc(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 2:
            break;
        default:
            Entity_SetAnimation(entity, 0, 3, 0);
            break;
        }

    }
}

///@TODO
void AI_UpdateCroc2(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 1:
            break;
        default:
            Entity_SetAnimation(entity, 0, 1, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateLion(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 5, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateGorilla(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 7, 0);
            break;

        }
    }
}

///@TODO
void AI_UpdateRat(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 1:
            break;
        default:
            Entity_SetAnimation(entity, 0, 1, 0);
            break;
        }
    }
}

void AI_UpdateRat2(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 2, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateTrex(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 2, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateRaptor(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 2, 0);
            break;
        }
    }
}

///@TODO
void AI_UpdateMutantWinged(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 13:
            break;
        default:
            Entity_SetAnimation(entity, 0, 25, 0);
            break;
        }
    }
}
///@TODO
void AI_UpdateMutantCentaur(entity_p entity)
{
    if(entity != NULL)
    {
        switch(entity->bf->animations.next_state)
        {
        case 3:
            break;
        default:
            Entity_SetAnimation(entity, 0, 5, 0);
            break;
        }
    }
}




