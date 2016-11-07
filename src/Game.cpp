#include "Game.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include "Qor/Material.h"
#include "Qor/Text.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

// These keywords are going to get me in trouble
const char* const Game :: TARGETS[] = {
    "WHITE HOUSE",
    "NSA",
    "PENTAGON",
    "AREA 51",
    "STATUE OF LIBERTY",
    "CAPITOL BUILDING",
    "FEDERAL RESERVE",
    "IMF",
    "DNC",
    "ANONYMOUS"
};

Game :: Game(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pController(engine->session()->active_profile(0)->controller().get()),
    m_pRoot(make_shared<Node>()),
    m_pOrthoRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources())
{}

void Game :: preload()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pOrthoCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pMusic = m_pQor->make<Sound>("music.ogg");
    m_pRoot->add(m_pMusic);
    m_pScene = m_pQor->make<Mesh>("store1.obj");
    m_pRoot->add(m_pScene);
    
    m_pPlayer = make_shared<Mesh>();
    m_pPlayerNode = make_shared<Node>();
    m_pPlayerMesh = m_pQor->make<Mesh>("hacker.obj");
    m_pPlayerMesh->rotate(0.5f, Axis::Y);
    m_pPlayer->self_visible(false);
    auto sz = 0.25f;
    auto height = 1.0f;
    m_pPlayer->set_box(m_pPlayerMesh->box());
    m_pPlayerNode->add(m_pPlayerMesh);
    m_pPlayer->add(m_pPlayerNode);
    //m_pPlayer->set_box(Box(
    //    glm::vec3(-sz, -height, -sz),
    //    glm::vec3(sz, height, sz)
    //));
    m_pPlayer->set_physics(Node::DYNAMIC);
    m_pPlayer->set_physics_shape(Node::CAPSULE);
    m_pPlayer->mass(1.0f);
    m_pPlayer->inertia(false);
    
    m_pRoot->add(m_pPlayer);
    //m_pPlayer->add(m_pCamera);
    //m_pCamera->move(Axis::Z);
    //m_pRoot->add(m_pCamera);

    auto meshes = m_pScene->find_type<Mesh>();
    for(auto&& mesh: meshes){
        mesh->set_physics(Node::STATIC);
    }
    
    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    m_pPhysics->generate(m_pRoot.get(), Physics::GEN_RECURSIVE);
    m_pPhysics->world()->setGravity(btVector3(0.0, -9.8, 0.0));
    auto body = (btRigidBody*)m_pPlayer->body()->body();
    body->setActivationState(DISABLE_DEACTIVATION);
    
    m_pPlayer->position(Axis::Y * height);

    auto mat = make_shared<Material>("videostripes.png", m_pResources);
    auto mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw, sh), vec2(0.0f, 0.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
        },
        make_shared<MeshMaterial>(mat)
    );
    //m_BGScale = 0.5f;
    m_pOrthoRoot->add(mesh);

    m_pFont = std::make_shared<Font>(
        m_pResources->transform(string("vcr.ttf:") +
            to_string(int(sw / 24.0f + 0.5f))),
        m_pResources
    );
    m_pText = std::make_shared<Text>(m_pFont);
    m_pText->align(Text::CENTER);
    m_pText->position(glm::vec3(sw / 2.0f, sh / 4.0f, 0.0f));
    m_pOrthoRoot->add(m_pText);
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: enter()
{
    m_pCamera->perspective();
    m_pOrthoCamera->ortho();
    //m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(true);
    m_pMusic->play();
}

void Game :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();
    
    m_pPhysics->logic(t);

    // camera logic
    if(m_pController->button("fire")){
        bool hacked = false;
        auto hits = m_pPhysics->hits(m_pPlayer->position(), m_pPlayerNode->orient_to_world(-Axis::Z) * 100.0f);
        for(auto&& hit: hits)
        {
            auto mesh = (Mesh*)std::get<0>(hit);
            auto fn = mesh->material()->texture()->filename();
            //LOGf("hit: %s", fn);
            if(fn.find("computer") != string::npos && fn.find("hacked") == string::npos)
            {
                float dist = glm::length(std::get<1>(hit) - m_pPlayer->position());
                if(dist < 2.0f)
                {
                    hacked = true;
                    mesh->material(make_shared<MeshMaterial>(
                        m_pResources->cache_as<Texture>("computer-hacked.png")
                    ));
                }
            }
        }
        if(hacked)
        {
            Sound::play(m_pCamera.get(),
                "hack" + to_string(std::rand()%MAX_HAX) + ".wav", m_pResources
            );
            m_pText->set(
                string("HACKING ") +
                TARGETS[std::rand()%(sizeof TARGETS / sizeof TARGETS[0])]
            );
            m_TextTime = 2.0f;
        }
    }

    m_TextTime = std::max(0.0f, m_TextTime - t.s());
    if(m_TextTime <= K_EPSILON)
        m_pText->set("");

    *m_pCamera->matrix() = *m_pPlayerNode->matrix();
    m_pCamera->position(m_pPlayerNode->position(Space::WORLD));
    auto zofs = m_pPlayerNode->orient_from_world(2.0f * Axis::Z);
    zofs.x = -zofs.x;
    m_pCamera->move(zofs);
    m_pCamera->pend();
 
    // TEMP: position and tilt camera
    //if(m_pInput->key(SDLK_SPACE))
    //    m_CamHeight += t.s();
    //if(m_pInput->key(SDLK_a))
    //    m_CamHeight -= t.s();
    //if(m_pInput->key(SDLK_r))
    //    m_CamTilt += t.s();
    //if(m_pInput->key(SDLK_w))
    //    m_CamTilt -= t.s();
    
    m_pCamera->move(Axis::Y * m_CamHeight);
    m_pCamera->rotate(m_CamTilt, Axis::X);

    // player logic
    float sens = 12.0f;
    float max_speed = 5.0f;

    glm::vec3 v;
    m_pPlayerNode->rotate(m_pInput->mouse_rel().x / 1000.0f, glm::vec3(0.0f, -1.0f, 0.0f));
    if(m_pController->button("up"))
        v += Matrix::heading(*m_pPlayerNode->matrix_c(Space::WORLD)) *
            m_pController->button("up").pressure();
    if(m_pController->button("down"))
        v -= Matrix::heading(*m_pPlayerNode->matrix_c(Space::WORLD)) *
            m_pController->button("down").pressure();
    if(m_pController->button("left"))
        v -= Matrix::right(*m_pPlayerNode->matrix_c(Space::WORLD)) *
            m_pController->button("left").pressure();
    if(m_pController->button("right"))
        v += Matrix::right(*m_pPlayerNode->matrix_c(Space::WORLD)) *
            m_pController->button("right").pressure();
    
    //    auto decel = glm::normalize(v) * sens * 0.8f * t.s();
    //    if(glm::length(decel) > K_EPSILON)
    //        v -= decel;
    //    else
    //        v = glm::vec3(0.0f);
    //}

    glm::vec3 xz = glm::vec3(v.x, 0.0f, v.z);
    auto y = v.y;
    float more_y = 0.0f;
    if(glm::length(xz) > K_EPSILON){
    //if(glm::length(xz) >= max_speed){
        v = glm::normalize(xz) * max_speed;
        //if(not m_WasMoving){
        //    more_y = 0.1f;
        //}
        //m_WasMoving = true;
        m_BobPhase = fmod(m_BobPhase + t.s() * m_BobSpeed, 1.0f);
        m_pPlayerMesh->position(Axis::Y * m_BobAmp * sinf(K_TAU * m_BobPhase));
    }
    //else
    //    m_WasMoving = false;
    glm::vec3 now_v = m_pPlayer->velocity();
    v.y = now_v.y + more_y;

    m_pPlayer->velocity(v);
    
    m_pRoot->logic(t);
    m_pOrthoRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->blend(false);
    m_pPipeline->winding(false);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
    
    m_pPipeline->blend(true);
    m_pPipeline->winding(true);
    m_pPipeline->render(m_pOrthoRoot.get(), m_pOrthoCamera.get(),
        nullptr, Pipeline::NO_CLEAR | Pipeline::NO_DEPTH
    );
    
}

