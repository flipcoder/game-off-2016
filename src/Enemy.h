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
        
        Cache<Resource, std::string>* m_pResources;
        Node* m_pRoot;
        Nav* m_pNav;

        bool m_Orient = false;
        //std::shared_ptr<Mesh> m_pMesh;

        // if Orient: orient points close to self
        // else: nav points
        std::vector<glm::vec3> m_Points;

        // nav history
        glm::vec3 m_Target;
        boost::circular_buffer<glm::vec3> m_History;

};

#endif

