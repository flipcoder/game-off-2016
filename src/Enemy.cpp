#include "Enemy.h"
#include "Game.h"
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

Enemy :: Enemy(Nav* nav, glm::vec3 pos, Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache),
    m_pNav(nav)
{
    m_History.set_capacity(4);
    
    position(pos);
    m_Y = pos.y;
    pos.y = 0.0f;
    
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
        if(dist <= nearest_dist){
            nearest = n;
            nearest_dist = dist;
            m_Orient = true;
        }
        LOGf("dist %s", dist);
        if(dist <= 4.0f){
            m_Points.push_back(n);
            LOG("orient point")
        }
    }
    LOGf("nearest dist %s", nearest_dist);
    LOGf("orient %s", m_Orient);

    if(not m_Orient)
    {
        // if nav, then set path to first nav and recalculate
        nearest.y = 0.0f;
        m_Target = nearest;
        m_Points.clear();
        //velocity(glm::normalize(nearest - position()));
    }
    else
    {
        auto _this = this;
        std::sort(ENTIRE(m_Points), [_this,pos](vec3 a, vec3 b){
            a.y = b.y = pos.y;
            return glm::length(a - pos) <
                glm::length(b - pos);
        });
        orient_towards(m_Points[m_PointsIndex]);
    }
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);
    auto pos = position();
    auto py = pos.y;
    pos.y = 0.0f;

    if(m_Orient)
    {
        *matrix() = glm::interpolate(m_OrientSrc, m_OrientTarget, m_OrientT);
        pend();
        m_OrientT = std::min(1.0f, m_OrientT + t.s() / 3.0f);
        if(m_OrientT >= 1.0f - K_EPSILON)
        {
            m_PointsIndex = (m_PointsIndex+1) % m_Points.size();
            orient_towards(m_Points[m_PointsIndex]);
        }
    }
    else
    {
        auto pos = position();
        m_BobPhase = fmod(m_BobPhase + t.s() * m_BobSpeed, 1.0f);
        pos.y = m_Y + m_BobAmp * sinf(K_TAU * m_BobPhase);
        position(glm::vec3(pos.x,pos.y,pos.z));
        
        auto targ = m_Target;
        targ.y = py;
        auto posdelta = targ - position();
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

void Enemy :: orient_towards(glm::vec3 p)
{
    LOGf("orient towards %s", Vector::to_string(p));
    auto pos = position();
    p.y = pos.y;
    auto nz = glm::normalize(p - pos);
    auto ang = glm::angle(nz, Axis::nZ);
    mat4 mat = glm::axisAngleMatrix(Axis::Y, ang);
    //LOG(Matrix::to_string(mat));
    //auto yvec = Axis::Y;
    ////LOGf("nz %s", Vector::to_string(nz));
    //auto mat = glm::mat4(glm::mat3(
    //    glm::cross(-nz, Axis::Y),
    //    Axis::Y,
    //    -nz
    //));
    Matrix::translation(mat, pos);
    m_OrientTarget = mat;
    m_OrientSrc = *matrix();
    m_OrientT = 0.0f;
}

