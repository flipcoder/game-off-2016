#include "Enemy.h"
#include "Game.h"
using namespace std;
using namespace glm;

Enemy :: Enemy(Nav* nav, glm::vec3 pos, Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache),
    m_pNav(nav)
{
    m_History.set_capacity(4);
    
    position(pos);
    
    // is nearest point in nav a nav point or orient?
    m_Orient = false; // nav point or orient?
    glm::vec3 nearest(m_pNav->point[0]);
    float nearest_dist = glm::length(position() - nearest);
    for(vec3 n: m_pNav->point)
    {
        float dist = glm::length(position() - n);
        if(dist <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
        }
    }
    for(vec3 n: m_pNav->orient)
    {
        float dist = glm::length(position() - n);
        if(glm::length(position() - n) <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
            m_Orient = true;
        }
    }
    LOGf("dist %s", nearest_dist);
    LOGf("orient %s", m_Orient);

    if(not m_Orient)
    {
        // if nav, then set path to first nav and recalculate
        nearest.y = 0.0f;
        m_Target = nearest;
        //velocity(glm::normalize(nearest - position()));
    }
    else
    {
        // if orient, get the nearest orients and put them in vec
        //auto orients = m_pNav->orient;
        //std::transform(ENTIRE());
        //std::remove_if(orients, [](glm::vec3 n){
        //    // out of range?
        //});
    }
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);
    auto pos = position();
    pos.y = 0.0f;

    if(m_Orient)
    {
    }
    else
    {
        auto posdelta = m_Target - position();
        if(glm::length(posdelta) < 1.0f)
        {
            // close to nav, find new nav
            if(not in_history(m_Target))
                m_History.push_back(m_Target);
            auto nav = m_pNav->point;
            auto _this = this;
            std::sort(ENTIRE(nav), [_this,pos](vec3 a, vec3 b){
                a.y = b.y = 0.0f;
                return glm::length(a - pos) <
                    glm::length(b - pos);
            });
            m_Target = nearest_not_in_history(nav);
        }
        velocity(3.0f * glm::normalize(posdelta));
    }
}

std::tuple<glm::vec3, float> Enemy :: nearest_nav() const
{
    glm::vec3 nearest(m_pNav->point[0]);
    float nearest_dist = glm::length(position() - nearest);
    for(vec3 n: m_pNav->point)
    {
        float dist = glm::length(position() - n);
        if(dist <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
        }
    }
    return { nearest, nearest_dist };
}

std::tuple<glm::vec3, float> Enemy :: nearest_orient() const
{
    glm::vec3 nearest(m_pNav->orient[0]);
    float nearest_dist = glm::length(position() - nearest);
    for(vec3 n: m_pNav->orient)
    {
        float dist = glm::length(position() - n);
        if(dist <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
        }
    }
    return { nearest, nearest_dist };
}

bool Enemy :: in_history(glm::vec3 target) const
{
    for(vec3 v: m_History)
    {
        if(v == target)
            return true;
    }
    return false;
}

glm::vec3 Enemy :: nearest_not_in_history(std::vector<glm::vec3>& nav) const
{
    for(auto&& v: nav)
    {
        if(not in_history(v))
            return v;
    }
    assert(false);
}

