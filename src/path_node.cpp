#include "path_node.h"

/*
 * Default constructor, initialise CPathNode here.
 */

CPathNode::CPathNode()
{
    this->m_g = 0;
    this->m_h = 0;
    this->m_sector = NULL;
    this->m_parentNode = NULL;
}

/*
 * Default destructor, uninitialise CPathNode here.
 */

CPathNode::~CPathNode()
{
    this->m_g = 0;
    this->m_h = 0;
    this->m_sector = NULL;
    this->m_parentNode = NULL;
}

/*
 * Sets path node G cost
 */

void CPathNode::SetG(uint32_t g)
{
    this->m_g = g;
}

/*
 * Sets path node heuristic
 */

void CPathNode::SetH(uint32_t h)
{
    this->m_h = h;
}

/*
 * Sets path node's parent node
 */

void CPathNode::SetParentNode(CPathNode* node)
{
    this->m_parentNode = node;
}

/*
 * Sets path node's sector
 */

void CPathNode::SetSector(room_sector_s* sector)
{
    if(sector != NULL)
    {
        this->m_sector = sector;
    }
}

/*
 * Returns the F cost (g+h)
 */

uint32_t CPathNode::GetFCost()
{
    return this->m_g + this->m_h;
}

/*
 * Returns the G cost
 */

uint32_t CPathNode::GetG()
{
    return this->m_g;
}

/*
 * Returns the heuristic
 */

uint32_t CPathNode::GetH()
{
    return this->m_h;
}

/*
 * Returns the parent node
 */

CPathNode* CPathNode::GetParentNode()
{
    return this->m_parentNode;
}


/*
 * Returns the sector
 */

room_sector_s* CPathNode::GetSector()
{
    return this->m_sector;
}
