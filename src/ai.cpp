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

#include <cassert>

/*
 * Updates specific entity
 */

void AI_UpdateEntity(entity_p entity)
{
    entity_p targetEntity = World_GetPlayer();

    ///@TODO Only supporting TR1
    if(World_GetVersion() != TR_I)
    {
        return;
    }

    //We can only continue if entity and targetEntity are valid entities.
    if((entity != NULL) && targetEntity != NULL && (entity->state_flags & ENTITY_STATE_ACTIVE))
    {
        CPathFinder* pathFinder = new CPathFinder();

        switch(entity->bf->animations.model->id)
        {
    case tr1Enemy::WOLF:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateWolf(entity);
        }
        break;
    case tr1Enemy::BEAR:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateBear(entity);
        }
        break;
    case tr1Enemy::BAT:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::FLYING);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::FLYING);
            AI_UpdateBat(entity);
        }
        break;
    case tr1Enemy::CROC:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateCroc(entity);
        }
        break;
    case tr1Enemy::CROC2:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::WATER);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::WATER);
            AI_UpdateCroc2(entity);
        }
        break;
    case tr1Enemy::LION_M:
    case tr1Enemy::LION_F:
        {
            pathFinder->FindPath(entity->current_sector, targetEntity->current_sector, AIType::GROUND);
            AI_MoveEntity(entity, targetEntity, pathFinder, AIType::GROUND);
            AI_UpdateLion(entity);
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
        targetPos.setZ(next_node->GetSector()->floor + 512.0f);
    }

    resultPos = lerp(startPos, targetPos, 1.30 * engine_frame_time);
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
        float dx = static_cast<float>((next_node->GetSector()->index_x - parent_node->GetSector()->index_x) * 90.0f);
        float dy = static_cast<float>((next_node->GetSector()->index_y - parent_node->GetSector()->index_y) * 90.0f);
        dx = dx *(M_PI / 180.0);
        dy = dy *(M_PI / 180.0);
        float ang = atan2(entity->angles[0] - dx, target_entity->angles[0] - dx);
        entity->angles[0] = ang * (180/M_PI);
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
