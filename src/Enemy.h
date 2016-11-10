#ifndef ENEMY_H_WFXIECAP
#define ENEMY_H_WFXIECAP

#include "Qor/Mesh.h"

class Enemy:
    public Mesh
{
    public:

        Enemy(Cache<Resource, std::string>* cache);
        virtual ~Enemy() {}

        virtual void logic_self(Freq::Time t) override;

    private:
        
        Cache<Resource, std::string>* m_pResources;
        //std::shared_ptr<Mesh> m_pMesh;

};

#endif

