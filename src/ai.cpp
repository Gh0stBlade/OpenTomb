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

/*
 * Updates specific entity
 */

void AI_UpdateEntity(entity_p entity)
{
    uint32_t meshType = 0;
    entity_p targetEntity = World_GetPlayer();

    ///@TODO Only supporting TR1
    if(World_GetVersion() != TR_I)
    {
        return;
    }

    //We can only continue if entity and targetEntity are valid entities.
    if((entity != NULL) && targetEntity != NULL && (entity->state_flags & ENTITY_STATE_ACTIVE))
    {
        if(entity->self->room->id != targetEntity->self->room->id)
        {
            ///@TODO
            ///1. check near by rooms, if target room is in the list. Get list of rooms to go trough to reach Lara.
            return;
        }

        meshType = entity->bf->animations.model->id;

        switch(meshType)
        {
    case tr1Enemy::WOLF:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateWolf(entity);
            delete pathFinder;
        }
        break;
    case tr1Enemy::BEAR:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateBear(entity);
            delete pathFinder;
        }
        break;
    case tr1Enemy::BAT:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::FLYING);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::FLYING);
            AI_UpdateBat(entity);
            delete pathFinder;
        }
        break;
    case tr1Enemy::CROC:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateCroc(entity);
            delete pathFinder;
        }
        break;
    case tr1Enemy::CROC2:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::WATER);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::WATER);
            AI_UpdateCroc2(entity);
            delete pathFinder;
        }
        break;
    case tr1Enemy::LION_M:
    case tr1Enemy::LION_F:
        {
            CPathFinder* pathFinder = NULL;
            pathFinder = new CPathFinder();
            pathFinder->InitialiseSearch(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateLion(entity);
            delete pathFinder;
        }
        break;
    default:
        //Nothing
        break;
        }
    }
}

///@PLACHOLDER
void AI_MoveEntity(entity_p entity, entity_p target_entity, CPathFinder* path, uint32_t flags)
{
    btVector3 startPos, targetPos, resultPos;
    CPathNode* next_node = NULL;

    if(entity == NULL) return;
    if(path == NULL) return;
    if(path->m_resultPath.size() <= 0) return;

    if(path->m_resultPath.size() >= 1)
    {
         next_node = path->m_resultPath[path->m_resultPath.size()-1];
    }


    if(next_node != NULL)
    {
        ///This is DISGRACEFUL ;)
        startPos.setX(entity->transform[12]);
        startPos.setY(entity->transform[13]);
        startPos.setZ(entity->transform[14]);

        targetPos.setX(next_node->GetSector()->pos[0]);
        targetPos.setY(next_node->GetSector()->pos[1]);
        targetPos.setZ(next_node->GetSector()->floor);

        resultPos = lerp(startPos, targetPos, 0.87 * engine_frame_time);
        entity->transform[12] = resultPos.getX();
        entity->transform[13] = resultPos.getY();
        //entity->transform[14] = resultPos.getZ();

        if(flags & AIType::GROUND || flags & AIType::WATER)///Ground entities stay on floor
            entity->transform[14] = next_node->GetSector()->floor;

        if(flags & AIType::FLYING)
            entity->transform[14] = next_node->GetSector()->floor + 1024.0f;

        ///Get facing angle
       // float ang = atan2(target_entity->transform[13] - entity->transform[13], target_entity->transform[11] - entity->transform[11]);
       // entity->angles[0] = ang * (180/M_PI);

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
            entity->bf->animations.current_animation = 6;
            entity->bf->animations.last_state = 5;
         break;
        case 3:
            {
                ///ATTACK
                if(World_GetPlayer()->current_sector == entity->current_sector)
                {
                    entity->bf->animations.last_state = entity->bf->animations.next_state;
                    entity->bf->animations.current_animation = 10;
                }
            }
            break;
        default:
            Con_Printf("Unimplemented state: %i", entity->bf->animations.next_state);
            break;
        }


    }
}

///@TODO
void AI_UpdateBear(entity_p entity)
{
    if(entity != NULL)
    {

    }
}

///@TODO
void AI_UpdateBat(entity_p entity)
{
    if(entity != NULL)
    {

    }
}

///@TODO
void AI_UpdateCroc(entity_p entity)
{
    if(entity != NULL)
    {

    }
}

///@TODO
void AI_UpdateCroc2(entity_p entity)
{
    if(entity != NULL)
    {

    }
}

///@TODO
void AI_UpdateLion(entity_p entity)
{
    if(entity != NULL)
    {

    }
}
