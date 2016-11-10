#ifndef ENEMY_H_WFXIECAP
#define ENEMY_H_WFXIECAP

#include "Qor/Mesh.h"

class Nav;

class Enemy:
    public Mesh
{
    public:

        Enemy(Nav* nav, Cache<Resource, std::string>* cache);
        virtual ~Enemy() {}

        virtual void logic_self(Freq::Time t) override;

    private:
        
        Cache<Resource, std::string>* m_pResources;
        Node* m_pRoot;
        Nav* m_pNav;
        //std::shared_ptr<Mesh> m_pMesh;

};

#endif

