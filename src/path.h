#ifndef PATH_H
#define PATH_H

#include "render/render.h"///@FIXME room.h wants OpenGL

#include "room.h"
#include "path_node.h"

#include <cstdlib>
#include <vector>

class CPathFinder
{

public:

     CPathFinder(room_sector_s* start_sector, room_sector_s* target_sector);
    ~CPathFinder();

    void                        FindPath(unsigned char flags);
    std::vector<CPathNode*>     GetResultPath();         ///Returns the result path

private:
    std::vector<CPathNode*>     m_resultPath;            ///Final result path is stored here if found.
    std::vector<CPathNode*>     m_nodes;                 ///List of all nodes (open and closed)
    std::vector<CPathNode*>     m_openList;              ///Nodes which have to be searched
    std::vector<CPathNode*>     m_closedList;            ///Nodes which have been searched and are now closed
    unsigned char               m_flags;                 ///Flags telling us what type of AI we need to create the path for.
    room_sector_s*              m_startSector;
    room_sector_s*              m_targetSector;

    CPathNode*                  GetNextOpenNode();
    void                        AddToOpenList(CPathNode* node);
    void                        AddToClosedList(CPathNode* node);
    void                        RemoveFromOpenList(CPathNode* node);
    void                        RemoveFromClosedList(CPathNode* node);
    bool                        IsInOpenList(CPathNode* node);
    bool                        IsInClosedList(CPathNode* node);
    CPathNode*                  GetNeighbourNode(short x, short y, short z, CPathNode* current_node);
    int                         CalculateHeuristic(CPathNode* start, CPathNode* target);
    void                        GeneratePath(CPathNode* end_node);
    bool                        IsValidNeighbour(CPathNode* current_node, CPathNode* neighbour_node);
    int                         GetMovementCost(CPathNode* from_node, CPathNode* to_node);
    CPathNode*                  AddNode();
};

#endif // PATH_H
