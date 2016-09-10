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

     CPathFinder();
    ~CPathFinder();

    void                        FindPath(room_sector_s* start, room_sector_s* target, unsigned char flags);
    CPathNode*                  GetNextOpenNode();

    ///@TODO private
    std::vector<CPathNode*>     m_resultPath;            ///Final result path

private:

    std::vector<CPathNode*>     m_nodes;                 ///List of all nodes
    std::vector<CPathNode*>     m_openList;              ///Nodes which have to be searched
    std::vector<CPathNode*>     m_closedList;            ///Nodes which have been searched and are now closed
    unsigned char               m_flags;

    void                        AddToOpenList(CPathNode* node);
    void                        AddToClosedList(CPathNode* node);
    void                        RemoveFromOpenList(CPathNode* node);
    void                        RemoveFromClosedList(CPathNode* node);
    bool                        IsInOpenList(CPathNode* node);
    bool                        IsInClosedList(CPathNode* node);
    CPathNode*                  GetNeighbourNode(short x, short y, CPathNode* current_node);
    int                         CalculateHeuristic(CPathNode* start, CPathNode* target);
    void                        GeneratePath(CPathNode* end_node);
    bool                        IsValidNeighbour(CPathNode* current_node, CPathNode* neighbour_node);
    int                         GetMovementCost(CPathNode* from_node, CPathNode* to_node);
    CPathNode*                  AddNode();
};

#endif // PATH_H
