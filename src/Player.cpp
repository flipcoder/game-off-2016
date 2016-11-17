#include "Player.h"
using namespace std;

Player :: Player(Cache<Resource, std::string>* cache):
    m_pResources(cache)
{
    m_pNode = make_shared<Node>();
    m_pMesh = make_shared<Mesh>(m_pResources->transform("hacker.obj"), m_pResources);
    m_pNode->add(m_pMesh);
    add(m_pNode);
    self_visible(false);
    set_box(m_pMesh->box());
    m_pNode->add(m_pMesh);
    add(m_pNode);
    set_physics(Node::DYNAMIC);
    set_physics_shape(Node::CAPSULE);
    mass(1.0f);
    inertia(false);
}

