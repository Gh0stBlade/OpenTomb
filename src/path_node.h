#ifndef PATH_NODE_H
#define PATH_NODE_H

#include "render/render.h"///@FIXME room.h wants OpenGL
#include "room.h"

class CPathNode
{

public:

    CPathNode(room_sector_s* sector);
    ~CPathNode();

    void                    SetG(unsigned int g);
    void                    SetH(unsigned int h);
    void                    SetParentNode(CPathNode* node);
    void                    SetSector(room_sector_s* sector);
    void                    SetOpen(bool is_open);
    void                    SetClosed(bool is_closed);

    unsigned int            GetFCost();
    unsigned int            GetG();
    unsigned int            GetH();
    CPathNode*              GetParentNode();
    room_sector_s*          GetSector();
    bool                    IsOpen();
    bool                    IsClosed();

private:

    unsigned int            m_g;
    unsigned int            m_h;
    CPathNode*              m_parentNode;
    struct room_sector_s*   m_sector;
    bool                    m_open;
    bool                    m_closed;
};

#endif // PATH_NODE_H
