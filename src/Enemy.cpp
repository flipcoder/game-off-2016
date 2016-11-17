#include "Enemy.h"
#include "Player.h"
#include "Game.h"
#include "Qor/Physics.h"
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

Enemy :: Enemy(Nav* nav, glm::vec3 pos, Player* player, ::Physics* physics, Cache<Resource, std::string>* cache):
    Mesh(cache->transform("clerk.obj"), cache),
    m_pResources(cache),
    m_pNav(nav),
    m_pPlayer(player),
    m_pPhysics(physics)
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
        //LOGf("dist %s", dist);
        if(dist <= 4.5f)
            m_Points.push_back(n);
    }
    //LOGf("nearest dist %s", nearest_dist);
    //LOGf("orient %s", m_Orient);

    if(not m_Orient)
    {
        // if nav, then set path to first nav and recalculate
        nearest.y = 0.0f;
        m_Target = nearest;
        m_Points.clear();
        m_OrientSpeed = 3.0f;
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

void Enemy :: gradually_orient(Freq::Time t)
{
    *matrix() = glm::interpolate(m_OrientSrc, m_OrientTarget, std::min(1.0f,m_OrientT));
    pend();
}

void Enemy :: logic_self(Freq::Time t)
{
    Mesh::logic_self(t);
    auto pos = position();
    auto py = pos.y;
    pos.y = 0.0f;

    if(m_Orient)
    {
        gradually_orient(t);
        m_OrientT = std::min(3.0f, m_OrientT + t.s() * m_OrientSpeed);
        if(m_OrientT >= 2.0f - K_EPSILON)
        {
            m_PointsIndex = (m_PointsIndex+1) % m_Points.size();
            orient_towards(m_Points[m_PointsIndex]);
            //LOGf("points size %s", m_Points.size());
            //LOGf("change to %s", m_PointsIndex);
        }
    }
    else
    {
        auto pos = position();
        m_BobPhase = fmod(m_BobPhase + t.s() * m_BobSpeed, 1.0f);
        pos.y = m_Y + m_BobAmp * sinf(K_TAU * m_BobPhase);
        
        gradually_orient(t);
        m_OrientT = std::min(1.0f, m_OrientT + t.s() * m_OrientSpeed);
        position(pos);

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
            orient_towards(m_Target);
        }
        velocity(2.0f * glm::normalize(posdelta));
    }

    // orient_to_world(-Axis::Z) * 100.0f
    auto hits = m_pPhysics->hits(position(), m_pPlayer->position());
    auto turns = glm::angle(
        orient_to_world(-Axis::Z),
        glm::normalize(m_pPlayer->position() - position())
    ) / K_TAU;
    if(turns < 1.0f/4.0f)
    {
        for(auto&& hit: hits)
        {
            auto mesh = (Mesh*)std::get<0>(hit);
            //if(mesh->material()){
                //auto fn = mesh->material()->texture()->filename();
                //LOG(fn);
            if(mesh == m_pPlayer){
                if(m_pPlayer->hacking())
                    m_pPlayer->spotted(true);
            }
            else if(mesh == this)
            {}
            else
                break;
            float dist = glm::length(std::get<1>(hit) - position());
            
        }
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
    //LOGf("orient towards %s", Vector::to_string(p));
    auto pos = position();
    p.y = pos.y;
    
    auto nz = glm::normalize(p - pos);
    //LOG(Vector::to_string(nz));
    auto turns = glm::angle(nz, Axis::nZ) / K_TAU;
    if(p.x > pos.x)
        turns = -turns;
    //LOGf("turns %s", turns);

    m_OrientAngle = turns;

    auto mat = glm::rotate(float(turns * K_TAU), Axis::Y);
    
    //mat4 mat(1.0f);
    //mat4 mat = glm::axisAngleMatrix(Axis::Y, ang);

    //LOG(Matrix::to_string(mat));
    //auto yvec = Axis::Y;
    ////LOGf("nz %s", Vector::to_string(nz));
    //auto mat = glm::mat4(glm::mat3(
    //    glm::cross(-nz, Axis::Y),
    //    Axis::Y,
    //    -nz
    //));
    
    Matrix::translation(mat, pos);
    //*matrix() = mat;
    //pend();
    
    m_OrientTarget = mat;
    m_OrientSrc = *matrix();
    m_OrientT = 0.0f;
}

