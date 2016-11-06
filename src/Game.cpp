#include "Game.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Game :: Game(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pController(engine->session()->active_profile(0)->controller().get()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline())
{}

void Game :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pMusic = m_pQor->make<Sound>("music.ogg");
    m_pRoot->add(m_pMusic);
    m_pScene = m_pQor->make<Mesh>("store1.obj");
    m_pRoot->add(m_pScene);
    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    
    //m_pPlayer = m_pQor->make<Mesh>("hacker.obj");
    m_pPlayer = make_shared<Node>();
    m_pRoot->add(m_pPlayer);
    m_pPlayer->position(glm::vec3(0.0f, 1.0f, 0.0f));
    m_pPlayer->add(m_pCamera);
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: enter()
{
    m_pCamera->perspective();
    //m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(true);
    m_pMusic->play();
}

void Game :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    float sens = 12.0f;
    float max_speed = 3.0f;

    //glm::vec3 v = m_pPlayer->velocity();
    glm::vec3 v;
    m_pCamera->rotate(m_pInput->mouse_rel().x / 1000.0f, glm::vec3(0.0f, -1.0f, 0.0f));
    if(m_pController->button("up"))
        v += t.s() * Matrix::heading(*m_pCamera->matrix_c()) *
            m_pController->button("up").pressure();
    if(m_pController->button("down"))
        v -= t.s() * Matrix::heading(*m_pCamera->matrix_c()) *
            m_pController->button("down").pressure();
    if(m_pController->button("left"))
        v -= t.s() * Matrix::right(*m_pCamera->matrix_c()) *
            m_pController->button("left").pressure();
    if(m_pController->button("right"))
        v += t.s() * Matrix::right(*m_pCamera->matrix_c()) *
            m_pController->button("right").pressure();
    
    //    auto decel = glm::normalize(v) * sens * 0.8f * t.s();
    //    if(glm::length(decel) > K_EPSILON)
    //        v -= decel;
    //    else
    //        v = glm::vec3(0.0f);
    //}

    if(m_pInput->key(SDLK_SPACE))
        m_pPlayer->move(Axis::Y * t.s());
    if(m_pInput->key(SDLK_a))
        m_pPlayer->move(-Axis::Y * t.s());

    glm::vec3 xz = glm::vec3(v.x, 0.0f, v.z);
    auto y = v.y;
    if(glm::length(xz) > K_EPSILON){
    //if(glm::length(xz) >= max_speed){
        v = glm::normalize(xz) * max_speed;
        v.y = y;
    }
    
    m_pPlayer->velocity(v);

    m_pRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

