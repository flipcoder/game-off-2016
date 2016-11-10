#include "Enemy.h"

Enemy :: Enemy(Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache)
{
    
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);

    
}

