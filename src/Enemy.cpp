#include "Enemy.h"
#include "Game.h"

Enemy :: Enemy(Nav* nav, Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache),
    m_pNav(nav)
{
    
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);

    
}

