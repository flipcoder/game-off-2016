#include "Enemy.h"
#include "Game.h"
using namespace std;
using namespace glm;

Enemy :: Enemy(Nav* nav, glm::vec3 pos, Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache),
    m_pNav(nav)
{
    position(pos);
    
    // is closest point in nav a nav point or orient?
    bool is_orient = false; // nav point or orient?
    glm::vec3 nearest(m_pNav->point[0]);
    float nearest_dist = glm::length(position() - nearest);
    for(vec3 n: m_pNav->point)
    {
        float dist = glm::length(position() - n);
        if(dist <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
            is_orient = false;
        }
    }
    for(vec3 n: m_pNav->orient)
    {
        float dist = glm::length(position() - n);
        if(glm::length(position() - n) <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
            is_orient = true;
        }
    }
    LOGf("dist %s", nearest_dist);
    LOGf("orient %s", is_orient);

    if(not is_orient)
    {
        // if nav, then set path to first nav and recalculate
        nearest.y = position().y;
        velocity(glm::normalize(nearest - position()));
    }
    else
    {
        // if orient, get the nearest orients and put them in vec
    }
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);

    
}

