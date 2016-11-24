#ifndef PLAYER_H_TKMY2G9V
#define PLAYER_H_TKMY2G9V

#include "Qor/Mesh.h"
class Enemy;

class Player:
    public Mesh
{
    public:

        Player(Cache<Resource, std::string>* cache);
        virtual ~Player() {}

        Mesh* mesh() { return m_pMesh.get(); }
        Node* node() { return m_pNode.get(); }

        void hacking(bool b) { m_bHacking = b; }
        bool hacking() const { return m_bHacking; }
        void spotted(Enemy* e) {
            if(e){
                m_bSpotted = true;
                on_spotted(e);
            }else
                m_bSpotted = false;
        }
        bool spotted() const { return m_bSpotted; }

        boost::signals2::signal<void(Enemy*)> on_spotted;
        
    private:

        Cache<Resource, std::string>* m_pResources;
        std::shared_ptr<Node> m_pNode;
        std::shared_ptr<Mesh> m_pMesh;

        bool m_bHacking = false;
        bool m_bSpotted = false;
};

#endif

