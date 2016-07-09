#include "core/system.h"

#include "ai.h"
#include "path.h"
#include "world.h"


///TEMPORARY
#define PATH_DISABLE_DIAGONAL   (1)
#define PATH_STABILITY_DEBUG    (0)
#define PATH_LOG_DETAILED_INFO  (0)

/*
 * Default constructor, initialise CPathFinder here.
 */

CPathFinder::CPathFinder()
{
    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_startRoom = NULL;
    this->m_targetRoom = NULL;
    this->m_flags = 0;
}

/*
 * Default destructor, uninitialise CPathNode here.
 */

CPathFinder::~CPathFinder()
{
    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();
    this->m_resultPath.clear();
    this->m_startRoom = NULL;
    this->m_targetRoom = NULL;
    this->m_flags = 0;
}

/*
 * This method starts a search from one room sector to the other
 */
///@TODO Search multiple rooms.

void CPathFinder::InitialiseSearch(room_sector_s* start, room_sector_s* target, uint32_t flags)
{
    CPathNode* start_node = NULL;
    CPathNode* target_node = NULL;
    CPathNode* current_node = NULL;
    CPathNode* neighbour_node = NULL;

    //The sector's start and target sector's room must not be null
    if(start->owner_room == NULL || target->owner_room == NULL)
    {
        return;
    }

    if(start->owner_room != target->owner_room)
    {
        ///@TODO
        ///1. check near by rooms, if target room is in the list. Get list of rooms to go trough to reach Lara.
        return;
    }

    //Set our start/target room
    this->m_startRoom = start->owner_room;
    this->m_targetRoom = target->owner_room;

    //Clear nodes list, open and closed list
    this->m_nodes.clear();
    this->m_openList.clear();
    this->m_closedList.clear();

    //Initialise all the nodes
    for(uint16_t y = 0; y < start->owner_room->sectors_y; y++)
    {
        for(uint16_t x = 0; x < start->owner_room->sectors_x; x++)
        {
            this->m_nodes.emplace_back();
            CPathNode* initial_node = &this->m_nodes[x + (start->owner_room->sectors_x * y)];///@HACK?
            initial_node->SetSector(World_GetRoomSector(start->owner_room->id, x, y));
        }
    }

    //Set flags so that we can customise the algorithm based on ai types
    this->m_flags ^= flags;

    //Get our start node and target node
    start_node = &this->m_nodes[start->index_x + (start->owner_room->sectors_x * start->index_y)];
    target_node = &this->m_nodes[target->index_x + (target->owner_room->sectors_x * target->index_y)];

    //Add our start node to the open list so it is searched
    this->m_openList.push_back(start_node);

    //Set start node initial cost/heuristic properties
    start_node->SetH(0);
    start_node->SetG(0);

    //Start node's parent must be NULL so we can saftely traverse back later
    start_node->SetParentNode(NULL);

    while((this->m_openList.size() > 0))
    {
        //Get the next node with lowest cost
        current_node = this->GetNextOpenNode();

#if PATH_LOG_DETAILED_INFO
        Sys_DebugLog(SYS_LOG_PATHFINDER, "PathFinder is at X: %i, Y: %i\n", current_node->GetSector()->index_x, current_node->GetSector()->index_y);
        Sys_DebugLog(SYS_LOG_PATHFINDER, "Target is at X: %i, Y: %i\n", target_node->GetSector()->index_x, target_node->GetSector()->index_y);
#endif // PATH_LOG_DETAILED_INFO

        if(current_node != NULL)
        {
            //If current_node is the target node we stop!
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

            //Remove the current Node from the Open List
            this->RemoveFromOpenList(current_node);

            //Add the current Node to the Closed List (we don't need it anymore)
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
///Diagonal sectors (0, 2, 5, 7) must cost us more?

            //Iterate through neighbours of the current node, get the one with the lowest F cost
            for(int32_t x = -1; x < 2; x++)
            {
                for(int32_t y = -1; y < 2; y++)
                {
                    //This is the current node, we'll skip it as it's useless!
                    if(x == 0 && y == 0)
                    {
                        continue;
                    }

                    //Grab the neighbour node
                    neighbour_node = this->GetNodeFromXY(x + current_node->GetSector()->index_x, y + current_node->GetSector()->index_y);

                    if(neighbour_node == NULL)
                    {
#if PATH_STABILITY_DEBUG
                        Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder] - Neighbour is NULL!\n");
#endif
                        continue;
                    }

                    if(neighbour_node->GetSector() == NULL)///@TODO assertion
                    {
#if PATH_STABILITY_DEBUG
                        Sys_DebugLog(SYS_LOG_PATHFINDER, "[CPathFinder] - Neighbour's sector is NULL!\n");
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
                        continue;
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
}

/*
 * Returns node in Open List with lowest F Cost.
 */

 ///@OPTIMISE - Order by F-Cost.
CPathNode* CPathFinder::GetNextOpenNode()
{
    int32_t current_cost, current_index;

    current_cost = 1000;///@FIXME
    current_index = -1;

    for(size_t i = 0; i < this->m_openList.size(); i++)
    {
        uint32_t cost = this->m_openList.at(i)->GetFCost();
        if(cost < current_cost)
        {
            current_cost = cost;
            current_index = i;
        }
    }

    if(this->m_openList.size() > 0 && current_index != -1)
    {
        return this->m_openList.at(current_index);
    }
    else
    {
        return NULL;
    }
}

/*
 * Adds new node to open list.
 */


void CPathFinder::AddToOpenList(CPathNode* node)
{
    if(node != NULL)
    {
        this->m_openList.push_back(node);
    }
}


/*
 * Adds new node to closed list.
 */


void CPathFinder::AddToClosedList(CPathNode* node)
{
    if(node != NULL)
    {
        this->m_closedList.push_back(node);
    }
}

/*
 * Removes specific node pointer from open list.
 */

void CPathFinder::RemoveFromOpenList(CPathNode* node)
{
    if(node != NULL)
    {
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
}

/*
 * Removes specific node pointer from closed list.
 */

void CPathFinder::RemoveFromClosedList(CPathNode* node)
{
    if(node != NULL)
    {
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
}

/*
 * Returns 1 if input node is in the open list
 */

int CPathFinder::IsInOpenList(CPathNode* node)
{
    if(this->m_openList.size() > 0)
    {
        for(uint32_t i = 0; i < this->m_openList.size(); i++)
        {
            if(this->m_openList[i] == node) return 1;
        }
    }

    return 0;
}

/*
 * Returns 1 if input node is in the closed list
 */

int CPathFinder::IsInClosedList(CPathNode* node)
{
    if(this->m_closedList.size() > 0)
    {
        for(uint32_t i = 0; i < this->m_closedList.size(); i++)
        {
            if(this->m_closedList[i] == node) return 1;
        }
    }

    return 0;
}

/*
 * Returns CPathNode* at x, y in node list.
 */

CPathNode* CPathFinder::GetNodeFromXY(uint16_t x, uint16_t y)
{
    if(this->m_startRoom != NULL)
    {
            if((x >= 0) && (x < this->m_startRoom->sectors_x) && (y >= 0) && (y < this->m_startRoom->sectors_y))
            {
                return &this->m_nodes[x + (this->m_startRoom->sectors_x*y)];
            }
    }
    else
    {
        Sys_DebugLog(SYS_LOG_FILENAME, "[CPathFinder::GetNodeFromXY] - (Warning) - Start room is NULL!\n");
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
int CPathFinder::IsValidNeighbour(CPathNode* current_node, CPathNode* neighbour_node)
{
    int32_t diff;
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
                if(current_sector_below->owner_room->flags & TR_ROOM_FLAG_WATER) return 0;
            }

            if(neighbour_sector_below != NULL)
            {
                if(neighbour_sector_below->owner_room->flags & TR_ROOM_FLAG_WATER) return 0;
            }

            if(current_sector->floor != neighbour_sector->floor)
            {
            //Height difference
            diff = current_sector->floor - neighbour_sector->floor;

                //If the current node's floor+1step is higher
                if(diff > 256 || diff < -256)
                {
                    return 0;
                }
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

/*
 * Returns movement cost from node A to node B.
 */

int CPathFinder::GetMovementCost(CPathNode* from_node, CPathNode* to_node)
{
    int movement_cost = 0;

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
