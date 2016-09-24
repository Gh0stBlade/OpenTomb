#include "core/system.h"

#include "ai.h"
#include "path.h"
#include "world.h"

#include <cmath>
#include <cassert>
#include <algorithm>

///TEMPORARY
#define PATH_STABILITY_DEBUG     (0)
#define PATH_LOG_DETAILED_INFO   (0)
#define PATH_DEBUG_DRAW          (1)
#define PATH_DEBUG_CLOSED_NODES  (0)
#define PATH_DISABLE_VALID_NODES (0)

//Maximum height an AI entity can step upon
const unsigned short MAX_STEP_HEIGHT = 256;

/*
 * Default constructor, initialise CPathFinder here.
 */

CPathFinder::CPathFinder(room_sector_s* start_sector, room_sector_s* target_sector)
{
    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_flags = 0;
    this->m_startSector = start_sector;
    this->m_targetSector = target_sector;
}

/*
 * Default destructor, uninitialise CPathFinder here.
 */

CPathFinder::~CPathFinder()
{
    for(size_t i = 0; i < this->m_nodes.size(); i++)
    {
        delete this->m_nodes[i];
    }

    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_flags = 0;
    this->m_startSector = nullptr;
    this->m_targetSector = nullptr;
}

/*
 * This method starts a search from one room sector to the other
 */
void CPathFinder::FindPath(unsigned char flags)
{
    //Impossible to continue if either start/target sector pointers are nullptr
    assert(this->m_startSector != nullptr && this->m_targetSector != nullptr);

    //Set flags so that we can customise the algorithm based on ai types
    this->m_flags |= flags;

    CPathNode* start_node = this->AddNode(this->m_startSector);
    CPathNode* target_node = this->AddNode(this->m_targetSector);
    assert(start_node != nullptr && target_node != nullptr);
    this->AddToOpenList(start_node);

    ///Early exit ground/flying entities will never find Lara.
    if(!(this->m_flags & AIType::WATER) && (this->m_targetSector->owner_room->flags & TR_ROOM_FLAG_WATER))
    {
        return;///@TODO generate escape path.
    }

    while((this->m_openList.size() > 0))
    {
        if(this->m_nodes.size() > (32*32))///@HACK
        {
             break;
        }

        //Get the next node with lowest cost in the open list
        CPathNode* current_node = this->GetNextOpenNode();
        assert(current_node != nullptr);

#if PATH_LOG_DETAILED_INFO
        Sys_DebugLog(SYS_LOG_PATHFINDER, "PathFinder is at X: %i, Y: %i\n", current_node->GetSector()->index_x, current_node->GetSector()->index_y);
        Sys_DebugLog(SYS_LOG_PATHFINDER, "Target is at X: %i, Y: %i\n", target->index_x, target->index_y);
#endif // PATH_LOG_DETAILED_INFO

        //If current_node's sector is the target sector we stop!
        if(current_node->GetSector() == this->m_targetSector)
        {
            //We're ready to generate the final path
            this->GeneratePath(current_node);
            break;
        }

        //Remove the current node from the openList and add it to the closed list.
        this->RemoveFromOpenList(current_node);
        this->AddToClosedList(current_node);
///1.
///SEARCHING OVERLAPS
/// |-----------------------| (Y)
/// |   0   |   1   |   2   |  |
/// |-----------------------|  V
/// |   3   | START |   4   |
/// |-----------------------|
/// |   5   |   6   |   7   |
/// |-----------------------|
/// (X) ->
///2.
///Diagonal sectors (0, 2, 5, 7) must cost us more.

        //Iterate through neighbours of the current node, get the one with the lowest F cost
        for(short x = -1; x < 2; x++)
        {
            for(short y = -1; y < 2; y++)
            {
                for(short z = 0; z < 3; z++)
                {
                    //This is the current node, we'll skip it as it's useless!
                    if(x == 0 && y == 0 && z == 0)
                    {
                        continue;
                    }

                    //Grab the neighbour node
                    CPathNode* neighbour_node = this->GetNeighbourNode(x, y, z, current_node);
                    if(neighbour_node == nullptr)
                    {
#if PATH_STABILITY_DEBUG
                        Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder] - Neighbour is NULL!\n");
#endif
                        continue;
                    }

                    //We need to check if this is a valid sector the entity is able to walk upon
                    if(!this->IsValidNeighbour(current_node, neighbour_node))
                    {
                        ///Early out, impossible to reach target
                        if(neighbour_node->GetSector() == target_node->GetSector())
                        {
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if(neighbour_node != nullptr)
                    {
                        //Get distance between our current_node and the neighbour_node
                        int step_cost = current_node->GetG() + this->GetMovementCost(current_node, neighbour_node);
                        if (step_cost < neighbour_node->GetG())
                        {
                            if (this->IsInOpenList(neighbour_node))
                            {
                                this->RemoveFromOpenList(neighbour_node);
                            }

                            if (this->IsInClosedList(neighbour_node))
                            {
                                this->RemoveFromClosedList(neighbour_node);
                            }
                        }

                        //This is a better route, add it to the open list so we may search it next!
                        if (!this->IsInOpenList(neighbour_node) && !this->IsInClosedList(neighbour_node))
                        {
                            neighbour_node->SetG(step_cost);
                            neighbour_node->SetH(this->CalculateHeuristic(neighbour_node, target_node));
                            neighbour_node->SetParentNode(current_node);
                            this->AddToOpenList(neighbour_node);
                        }
                    }
                }
            }
        }
    }
}

/*
 * Returns node in Open List with lowest F Cost.
 */

CPathNode* CPathFinder::GetNextOpenNode()
{
#if 1
    if(this->m_openList.size() > 0)
    {
        int current_cost = this->m_openList[0]->GetFCost();
        int current_index = 0;

        for(size_t i = 1; i < this->m_openList.size(); i++)
        {
            unsigned int next_cost = this->m_openList.at(i)->GetFCost();
            if(next_cost < current_cost)
            {
                current_cost = next_cost;
                current_index = i;
            }
        }

        return this->m_openList.at(current_index);
    }
    else
    {
        return nullptr;
    }
#else
    if(this->m_openList.size() > 0)
    {
        return this->m_openList.at(0);
    }
    else
    {
        return nullptr;
    }
#endif
}

/*
 * Adds new node to open list.
 */

void CPathFinder::AddToOpenList(CPathNode* node)
{
    assert(node != nullptr);
#if 1
    this->m_openList.push_back(node);
#else
    if(this->m_openList.size() > 0)
    {
        unsigned int current_cost = this->m_openList[0]->GetFCost();
        unsigned int current_index = 0;

        if(node->GetFCost() <= current_cost)
        {
            this->m_openList.push_back(node);
            return;
        }

        for(size_t i = 1; i < this->m_openList.size(); i++)
        {
            unsigned int next_cost = this->m_openList.at(i)->GetFCost();
            if(next_cost >= current_cost)
            {
                this->m_openList.insert(this->m_openList.begin() + i, node);
                return;
            }
        }
    }
    else
    {
        this->m_openList.push_back(node);
    }
#endif // 1
}

/*
 * Adds new node to closed list.
 */

void CPathFinder::AddToClosedList(CPathNode* node)
{
    assert(node != nullptr);
    this->m_closedList.push_back(node);
}

/*
 * Removes input node pointer from open list.
 */

void CPathFinder::RemoveFromOpenList(CPathNode* node)
{
    assert(node != nullptr);
    for(size_t i = 0; i < this->m_openList.size(); i++)
    {
        //If we located a pointer match it's the same node
        if(this->m_openList[i] == node)
        {
            this->m_openList.erase(this->m_openList.begin() + i);
            return;
        }
    }
}

/*
 * Removes input node pointer from closed list.
 */

void CPathFinder::RemoveFromClosedList(CPathNode* node)
{
    assert(node != nullptr);
    for(size_t i = 0; i < this->m_closedList.size(); i++)
    {
        //If we located a pointer match it's the same node
        if(this->m_closedList[i] == node)
        {
            this->m_closedList.erase(this->m_closedList.begin() + i);
            return;
        }
    }
}

/*
 * Returns true if input node is in the open list
 */

bool CPathFinder::IsInOpenList(CPathNode* node)
{
    if(this->m_openList.size() > 0)
    {
        for(size_t i = 0; i < this->m_openList.size(); i++)
        {
            if(this->m_openList[i]->GetSector() == node->GetSector()) return true;
        }
    }

    return false;
}

/*
 * Returns true if input node is in the closed list
 */

bool CPathFinder::IsInClosedList(CPathNode* node)
{
    if(this->m_closedList.size() > 0)
    {
        for(size_t i = 0; i < this->m_closedList.size(); i++)
        {
            if(this->m_closedList[i]->GetSector() == node->GetSector()) return true;
        }
    }

    return false;
}

/*
 * Returns CPathNode* at x, y in node list.
 */

CPathNode* CPathFinder::GetNeighbourNode(short x, short y, short z, CPathNode* current_node)///@OPTIMISE if current node is same as target, we don't want to search portals.
{
    assert(current_node != nullptr);

    room_sector_s* current_sector = current_node->GetSector();
    assert(current_sector != nullptr);

    room_s* current_room = current_sector->owner_room;
    assert(current_room != nullptr);

    short neighbour_x = (x + current_sector->index_x);
    short neighbour_y = (y + current_sector->index_y);

    if(z == 1)
    {
        room_sector_s* sector_below = current_sector->sector_below;
        if(sector_below != nullptr)
        {
            current_room = sector_below->owner_room;
            neighbour_x = (x + sector_below->index_x);
            neighbour_y = (y + sector_below->index_y);
        }
        else
        {
            return nullptr;
        }
    }
    else if(z == 2)
    {
        room_sector_s* sector_above = current_sector->sector_above;
        if(sector_above != nullptr)
        {
            current_room = sector_above->owner_room;
            neighbour_x = (x + sector_above->index_x);
            neighbour_y = (y + sector_above->index_y);
        }
        else
        {
            return nullptr;
        }
    }

    //We don't want to process any out of bound sectors (this should never happen)
    if((neighbour_x >= 0) &&
        (neighbour_y >= 0) &&
        (neighbour_x < current_room->sectors_x) &&
        (neighbour_y < current_room->sectors_y))
    {
        //Generate a neighbour node with the newly found sector.
        room_sector_s* neighbour_sector = World_GetRoomSector(current_room->id, neighbour_x, neighbour_y);

        //Disable diagonal
        if(neighbour_sector->index_x != current_sector->index_x && neighbour_sector->index_y != current_sector->index_y && z == 0)
        {
            return nullptr;
        }

        CPathNode* neighbour_node = this->AddNode(neighbour_sector);
        return neighbour_node;
    }
    else
    {
        ///We're only required to search portals if target is NOT in the current room
        if(this->m_targetSector->owner_room == current_node->GetSector()->owner_room)
        {
            return nullptr;
        }

        ///An out of bound room sector means we've possibly reached a portal sector (hence why the current neighbour is not in bounds of the current room)
        ///Here we check this and add the sector through the portal so the search is continued.
        room_s* neighbour_room = current_node->GetSector()->portal_to_room;

        if(neighbour_room != nullptr)
        {
            ///Here we iterate through all sectors in the neighbour room to find the sector that joins with the current portal
            ///@OPTIMISE We can use World_GetSector... if we know the x,y indexs, should be calculated!
            for(unsigned int i = 0; i < neighbour_room->sectors_count; i++)
            {
                room_sector_s* sector = &neighbour_room->sectors[i];
                if(sector->portal_to_room != current_room)
                {
                    continue;
                }
                else
                {
                    if(sector->pos[0] == current_node->GetSector()->pos[0] || sector->pos[1] == current_node->GetSector()->pos[1])
                    {
                        ///@FIXME should check to see which is the best portal sector that our entity can go to!
                        CPathNode* neighbour_node = this->AddNode(sector);
                        if(IsValidNeighbour(current_node, neighbour_node))
                        {
                            return neighbour_node;
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

/*
 * Calculates heuristic distance between two nodes.
 */

int CPathFinder::CalculateHeuristic(CPathNode* start, CPathNode* target)
{
    assert(start != nullptr && target != nullptr);

    room_sector_s* start_sector = start->GetSector();
    room_sector_s* target_sector = target->GetSector();

    assert(start_sector != nullptr && target_sector != nullptr);

    int dx = std::abs((start_sector->pos[0] - target_sector->pos[0]));
    int dy = std::abs(start_sector->pos[1] - target_sector->pos[1]);

    if(dx > dy) return 14 * dy + 10 * (dx - dy);
    return (14 * dx) + (10 * (dy - dx));
}

/*
 * Generates final path, stores in result_path
 */

void CPathFinder::GeneratePath(CPathNode* end_node)
{
#if PATH_DEBUG_DRAW
    renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
#endif
    while(end_node->GetParentNode() != nullptr)///@FIXME should be end_node?
    {
#if PATH_DEBUG_DRAW
        if(end_node->GetSector() != nullptr)
        {
            renderer.debugDrawer->DrawSectorDebugLines(end_node->GetSector());
        }
#endif
        this->m_resultPath.push_back(end_node);
        end_node = end_node->GetParentNode();
    }

#if PATH_DEBUG_CLOSED_NODES
    renderer.debugDrawer->SetColor(1.0, 0.0, 0.0);
    for(size_t i = 0; i < this->m_closedList.size(); i++)
    {
        room_sector_s* sector = this->m_closedList.at(i)->GetSector();
        if(sector != nullptr)
        {
            renderer.debugDrawer->DrawSectorDebugLines(sector);
        }
    }

#endif // PATH_DEBUG_CLOSED_NODES
}

/*
 * Returns true if entity can move to this sector
 */
bool CPathFinder::IsValidNeighbour(CPathNode* current_node, CPathNode* neighbour_node)
{
    assert(current_node != nullptr);
    assert(neighbour_node != nullptr);

    room_sector_s* current_sector = current_node->GetSector();
    room_sector_s* neighbour_sector = neighbour_node->GetSector();

    assert(current_sector != nullptr);
    assert(neighbour_sector != nullptr);

    ///Invalid entity cannot move here
    if(neighbour_sector->floor == neighbour_sector->ceiling)///@FIXME &&! neighbour_sector->sector_below?
    {
        return false;
    }

    ///Water entities can only move through rooms that have water flag set
    if(this->m_flags & AIType::WATER && !(neighbour_sector->owner_room->flags & TR_ROOM_FLAG_WATER))
    {
        return false;
    } ///Ground and flying entities should never enter water.
    else if(!this->m_flags & AIType::WATER && (neighbour_sector->owner_room->flags & TR_ROOM_FLAG_WATER))
    {
        return false;
    }

    ///Ground entities can only move on floor, so here we check if the neighbour has a sector below it!
    if((this->m_flags & AIType::GROUND))
    {
#if 0
        room_sector_s* neighbour_sector_below = neighbour_sector->sector_below;
        if(neighbour_sector_below != NULL)
        {
            ret = false;
        }
#else
        if(current_sector->floor != neighbour_sector->floor)
        {
            //Height difference
            int diff = current_sector->floor - neighbour_sector->floor;
            //If the current node's floor+1step is higher
            if(diff > MAX_STEP_HEIGHT || diff < -MAX_STEP_HEIGHT)
            {
                return false;
            }
        }

        room_sector_s* neighbour_sector_below = neighbour_sector->sector_below;
        int next_floor = 0;
        int current_floor = current_sector->floor;
        while(neighbour_sector_below)
        {
            next_floor += neighbour_sector_below->floor;
            if(current_floor - next_floor > MAX_STEP_HEIGHT || (neighbour_sector_below->owner_room->flags & TR_ROOM_FLAG_WATER))
            {
                return false;
            }
            else
            {
                if(neighbour_sector_below->sector_below != nullptr)
                {
                    return false;
                }
            }

            neighbour_sector_below = neighbour_sector_below->sector_below;
        }
#endif
    }

    return true;
}

/*
 * Returns movement cost from node A to node B.
 */

int CPathFinder::GetMovementCost(CPathNode* from_node, CPathNode* to_node)
{
    int movement_cost = 0;

#if 1
    CPathNode* parent_node = from_node->GetParentNode();
    if(parent_node != nullptr)
    {
        movement_cost += parent_node->GetG();
    }
#endif

    if(from_node->GetSector()->index_x != to_node->GetSector()->index_x && from_node->GetSector()->index_y != to_node->GetSector()->index_y)
    {
        movement_cost += 14;
    }
    else
    {
        movement_cost += 10;
    }

    return movement_cost;
}

/*
 * Adds then returns pointer to new node
 */

CPathNode* CPathFinder::AddNode(room_sector_s* sector)
{
    this->m_nodes.emplace_back(new CPathNode(sector));
    return this->m_nodes.back();
}

/*
 * Returns result path found.
 */

std::vector<CPathNode*>* CPathFinder::GetResultPath()
{
    return &this->m_resultPath;
}
