#ifndef PATH_NODE_H
#define PATH_NODE_H

#include "render/render.h"///@FIXME room.h wants OpenGL
#include "room.h"

class CPathNode
{
public:

    CPathNode();
    ~CPathNode();

    void SetG(uint32_t g);
    void SetH(uint32_t h);
    void SetParentNode(CPathNode* node);
    void SetSector(room_sector_s* sector);

    uint32_t GetFCost();
    uint32_t GetG();
    uint32_t GetH();
    CPathNode* GetParentNode();
    room_sector_s* GetSector();

private:

    uint32_t                m_g;
    uint32_t                m_h;
    CPathNode*              m_parentNode;
    struct room_sector_s*   m_sector;
};

#endif // PATH_NODE_H
