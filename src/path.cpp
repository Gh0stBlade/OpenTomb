#include "core/system.h"

#include "ai.h"
#include "path.h"
#include "world.h"

#include <cassert>

///TEMPORARY
#define SINGLE_ROOM             (0)
#define PATH_DISABLE_DIAGONAL   (0) ///@FIXME Don't enable, illegal for portal sectors.
#define PATH_STABILITY_DEBUG    (0)
#define PATH_LOG_DETAILED_INFO  (0)

///@DEADLOCK - Dead lock now occurs due to portals being traversed through.
///Occurs because the search iterates through every single room/sector so if entities cannot reach Lara, search scale is every room in the level!
///@TODO
///Terminate path generation depending on how many rooms have currently been searched.

/*
 * Default constructor, initialise CPathFinder here.
 */

CPathFinder::CPathFinder()
{
    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_flags = 0;
}

/*
 * Default destructor, uninitialise CPathFinder here.
 */

CPathFinder::~CPathFinder()
{
    if(this->m_nodes.size() > 0)
    {
        for(size_t i = 0; i < this->m_nodes.size(); i++)
        {
            delete this->m_nodes[i];
        }
    }

    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_flags = 0;
}

/*
 * This method starts a search from one room sector to the other
 */
void CPathFinder::FindPath(room_sector_s* start, room_sector_s* target, unsigned char flags)
{
    CPathNode* current_node = NULL;
    CPathNode* neighbour_node = NULL;

    //Impossible to continue if either start/target sector pointers are NULL
    assert(start);
    assert(target);

#if SINGLE_ROOM
    if(start->owner_room != target->owner_room)
    {
        return;
    }
#endif // SINGLE_ROOM

    //Set flags so that we can customise the algorithm based on ai types
    this->m_flags |= flags;

    CPathNode* start_node = this->AddNode();
    assert(start_node);
    start_node->SetSector(start);
    this->AddToOpenList(start_node);

    CPathNode* target_node = this->AddNode();
    assert(target_node);
    target_node->SetSector(target);

    while((this->m_openList.size() > 0))
    {
        //Get the next node with lowest cost in the open list
        current_node = this->GetNextOpenNode();
        assert(current_node);

#if PATH_LOG_DETAILED_INFO
        Sys_DebugLog(SYS_LOG_PATHFINDER, "PathFinder is at X: %i, Y: %i\n", current_node->GetSector()->index_x, current_node->GetSector()->index_y);
        Sys_DebugLog(SYS_LOG_PATHFINDER, "Target is at X: %i, Y: %i\n", target->index_x, target->index_y);
#endif // PATH_LOG_DETAILED_INFO

        //If current_node's sector is the target sector we stop!
        if(current_node->GetSector() == target)
        {
            //Entity is in current sector as player
            if(current_node->GetParentNode() == NULL)
            {
                return;
            }

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
                //This is the current node, we'll skip it as it's useless!
                if(x == 0 && y == 0)
                {
                    continue;
                }

                //Grab the neighbour node
                neighbour_node = this->GetNeighbourNode(x, y, current_node);
                if(neighbour_node == NULL)
                {
#if PATH_STABILITY_DEBUG
                    Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder] - Neighbour is NULL!\n");
#endif
                    continue;
                }

                //We need to check if this is a valid sector the entity is able to walk upon
                if(!this->IsValidNeighbour(current_node, neighbour_node))
                {
                    continue;
                }

                ///We want to set different costs for vertical&horziontal, diagonal moves.
                if(neighbour_node->GetSector()->index_x != current_node->GetSector()->index_x && neighbour_node->GetSector()->index_y != current_node->GetSector()->index_y)
                {
#if PATH_DISABLE_DIAGONAL
                        continue;///@FIXME illegal for portal sectors!
#endif
                    neighbour_node->SetG(14);
                }
                else
                {
                    neighbour_node->SetG(10);
                }

                if(neighbour_node != NULL)
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

        for(size_t i = 0; i < this->m_openList.size(); i++)
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
        return NULL;
    }
#else
    if(this->m_openList.size() > 0)
    {
        return this->m_openList.at(0);
    }
    else
    {
        return NULL;
    }
#endif
}

/*
 * Adds new node to open list.
 */

void CPathFinder::AddToOpenList(CPathNode* node)
{
    assert(node);
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
    assert(node);
    this->m_closedList.push_back(node);
}

/*
 * Removes specific node pointer from open list.
 */

void CPathFinder::RemoveFromOpenList(CPathNode* node)
{
    assert(node);
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
 * Removes specific node pointer from closed list.
 */

void CPathFinder::RemoveFromClosedList(CPathNode* node)
{
    assert(node);
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
 * Returns 1 if input node is in the open list
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
 * Returns 1 if input node is in the closed list
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

CPathNode* CPathFinder::GetNeighbourNode(short x, short y, CPathNode* current_node)
{
    assert(current_node);

    room_sector_s* current_sector = current_node->GetSector();
    assert(current_sector);

    room_s* current_room = current_sector->owner_room;
    assert(current_room);

    short neighbour_x = (x + current_sector->index_x);
    short neighbour_y = (y + current_sector->index_y);

    //We don't want to process any out of bound sectors (this should never happen)
    if((neighbour_x >= 0) &&
        (neighbour_y >= 0) &&
        (neighbour_x < current_room->sectors_x) &&
        (neighbour_y < current_room->sectors_y))
    {

        //Generate a neighbour node with the newly found sector.
        room_sector_s* neighbour_sector = World_GetRoomSector(current_room->id, neighbour_x, neighbour_y);
        CPathNode* node = this->AddNode();
        node->SetSector(neighbour_sector);
        return node;
    }
    else
    {
#if !SINGLE_ROOM
        ///An out of bound room sector means we've possibly reached a portal sector (hence why the current neighbour is not in bounds of the current room)
        ///Here we check this and add the sector through the portal so the search is continued.
        room_s* neighbour_room = current_node->GetSector()->portal_to_room;
        if(neighbour_room != NULL)
        {
            ///Here we iterate through all sectors in the neighbour room to find the sector that joins with the current portal
            ///@OPTIMISE We can use World_GetSector... if we know the x,y indexs, should be calculated!
            for(unsigned int i = 0; i < neighbour_room->sectors_count; i++)
            {
                room_sector_s* sector = &neighbour_room->sectors[i];
                if(sector->portal_to_room == current_room)
                {
                    CPathNode* node = this->AddNode();
                    node->SetSector(sector);
                    return node;
                }
            }
        }
#endif
    }

    return NULL;
}

/*
 * Calculates heuristic distance between two nodes.
 */

int CPathFinder::CalculateHeuristic(CPathNode* start, CPathNode* target)
{
    int sx, sy, tx, ty, dx, dy;

    if(start != NULL && target != NULL)
    {
        if(start->GetSector() != NULL && target->GetSector() != NULL)
        {
            sx = start->GetSector()->index_x;
            sy = start->GetSector()->index_y;
            tx = target->GetSector()->index_x;
            ty = target->GetSector()->index_y;
            dx = tx - sx;
            dy = ty - sy;
            return int(sqrt((dx*dx)+(dy*dy)));
        }
        else
        {
            Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder::CalculateHeuristic] - (Warning) - Start or Target sector is NULL!\n");
        }
    }
    else
    {
        Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder::CalculateHeuristic] - (Warning) - Start or Target NODE is NULL!\n");
    }

    return 0;
}

/*
 * Prints final path
 */

void CPathFinder::GeneratePath(CPathNode* end_node)
{
    this->m_resultPath.clear();

    while(end_node->GetParentNode() != NULL)
    {
        this->m_resultPath.push_back(end_node);
        end_node = end_node->GetParentNode();
    }

    for(size_t i = this->m_resultPath.size(); i-- > 0;)
    {
        if(this->m_resultPath[i]->GetSector() == NULL)
        {
            Sys_DebugLog(SYS_LOG_PATHFINDER, "Error during path gen, GetSector() returned NULL!\n");
            continue;
        }

        //Target will be marked red
        if(i == 0)
        {
            renderer.debugDrawer->SetColor(1.0, 0.0, 0.0);
        }
        else if(i == this->m_resultPath.size()-1)
        {
            renderer.debugDrawer->SetColor(0.0, 0.0, 1.0);
        }
        else
        {
            renderer.debugDrawer->SetColor(0.0, 1.0, 0.0);
        }

        renderer.debugDrawer->DrawSectorDebugLines(this->m_resultPath[i]->GetSector());
    }
}

/*
 * Returns 1 if entity can move to this sector
 */
///@TODO - This should check sector heights and anything that would block a specific entity type from moving to a sector/node i.e (water, walls, next step too high..)
///@TODO - Ceiling checks, water checks.
bool CPathFinder::IsValidNeighbour(CPathNode* current_node, CPathNode* neighbour_node)
{
    int diff;
    room_sector_p current_sector, neighbour_sector, current_sector_below, neighbour_sector_below;

    if(current_node != NULL && neighbour_node != NULL)
    {
        current_sector = current_node->GetSector();
        neighbour_sector = neighbour_node->GetSector();

        if(current_sector  != NULL && neighbour_sector != NULL && !(this->m_flags & AIType::WATER))
        {
            ///@FIXME find better way of doing this
            current_sector_below = current_sector->sector_below;
            neighbour_sector_below = neighbour_sector->sector_below;

            if(current_sector_below != NULL)
            {
                if(current_sector_below->owner_room->flags & TR_ROOM_FLAG_WATER) return false;
            }

            if(neighbour_sector_below != NULL)
            {
                if(neighbour_sector_below->owner_room->flags & TR_ROOM_FLAG_WATER) return false;
            }

            if(current_sector->floor != neighbour_sector->floor)
            {
            //Height difference
            diff = current_sector->floor - neighbour_sector->floor;

                //If the current node's floor+1step is higher
                if(diff > 256 || diff < -256)
                {
                    return false;
                }
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

/*
 * Returns movement cost from node A to node B.
 */

int CPathFinder::GetMovementCost(CPathNode* from_node, CPathNode* to_node)
{
    int movement_cost = 0;

#if 0
    CPathNode* parent_node = from_node->GetParentNode();
    if(parent_node != NULL)
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

CPathNode* CPathFinder::AddNode()
{
    this->m_nodes.emplace_back(new CPathNode());
    CPathNode* node = this->m_nodes[this->m_nodes.size()-1];
}
