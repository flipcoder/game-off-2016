#ifndef ENEMY_H_WFXIECAP
#define ENEMY_H_WFXIECAP

#include "Qor/Mesh.h"
#include <boost/circular_buffer.hpp>

class Nav;

class Enemy:
    public Mesh
{
    public:

        Enemy(Nav* nav, glm::vec3 pos, Cache<Resource, std::string>* cache);
        virtual ~Enemy() {}

        virtual void logic_self(Freq::Time t) override;

        std::tuple<glm::vec3, float> nearest_nav() const;
        std::tuple<glm::vec3, float> nearest_orient() const;

    private:

        bool in_history(glm::vec3 target) const;
        glm::vec3 nearest_not_in_history(std::vector<glm::vec3>& nav) const;
        void orient_towards(glm::vec3 p);
        
        Cache<Resource, std::string>* m_pResources;
        Node* m_pRoot;
        Nav* m_pNav;

        bool m_Orient = false;
        //std::shared_ptr<Mesh> m_pMesh;

        // if Orient: orient points close to self
        // else: nav points
        std::vector<glm::vec3> m_Points;
        int m_PointsIndex = 0;

        // nav history
        glm::vec3 m_Target;
        glm::mat4 m_OrientSrc;
        glm::mat4 m_OrientTarget;
        float m_OrientT = 0.0f;
        boost::circular_buffer<glm::vec3> m_History;

        float m_Y = 0.0f;
        float m_BobPhase = 0.0f;
        float m_BobAmp = 0.02f;
        float m_BobSpeed = 4.0f;

};

#endif

