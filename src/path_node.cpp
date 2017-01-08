#include "path_node.h"

/*
 * Default constructor, initialise CPathNode here.
 */

CPathNode::CPathNode(room_sector_s* sector)
{
    this->m_g = 0;
    this->m_h = 0;
    this->m_parentNode = nullptr;
    this->m_sector = sector;
}

/*
 * Default destructor, uninitialise CPathNode here.
 */

CPathNode::~CPathNode()
{
    this->m_g = 0;
    this->m_h = 0;
    this->m_parentNode = nullptr;
    this->m_sector = nullptr;
}

/*
 * Sets path node movement cost
 */

void CPathNode::SetG(unsigned int g)
{
    this->m_g = g;
}

/*
 * Sets path node heuristic
 */

void CPathNode::SetH(unsigned int h)
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
    this->m_sector = sector;
}

/*
 * Returns the F cost (g+h)
 */

unsigned int CPathNode::GetFCost()
{
    return (this->m_g + this->m_h);
}

/*
 * Returns the G cost
 */

unsigned int CPathNode::GetG()
{
    return this->m_g;
}

/*
 * Returns the heuristic
 */

unsigned int CPathNode::GetH()
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
