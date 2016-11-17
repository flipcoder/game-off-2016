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
    "SUPREME COURT",
    "ANONYMOUS",
    "JUSTICE SYSTEM"
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

glm::vec3 Game :: calc_origin(Mesh* mesh)
{
    glm::vec3 pmin(mesh->geometry()->verts()[0]);
    glm::vec3 pmax(mesh->geometry()->verts()[0]);
    for(auto&& v: mesh->geometry()->verts())
    {
        if(v.x < pmin.x)
            pmin.x = v.x;
        if(v.x > pmax.x)
            pmax.x = v.x;
        if(v.y < pmin.y)
            pmin.y = v.y;
        if(v.y > pmax.y)
            pmax.y = v.y;
        if(v.z < pmin.z)
            pmin.z = v.z;
        if(v.z > pmax.z)
            pmax.z = v.z;
    }
    return (pmax + pmin) * 0.5f;
}

void Game :: preload()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pOrthoCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pMusic = m_pQor->make<Sound>("music.ogg");
    m_pRoot->add(m_pMusic);
    m_pScene = m_pQor->make<Mesh>("store2.obj");
    m_pRoot->add(m_pScene);
    
    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    
    m_pPlayer = make_shared<Player>(m_pResources);
    m_pRoot->add(m_pPlayer);
    //m_pPlayer->add(m_pCamera);
    //m_pCamera->move(Axis::Z);
    //m_pRoot->add(m_pCamera);

    auto meshes = m_pScene->find_type<Mesh>();
    for(auto&& mesh: meshes){
        string fn = mesh->material()->texture()->filename();
        auto pt = calc_origin(mesh);

        if(fn.find("nav") != string::npos)
            m_Nav.point.push_back(pt);
        else if(fn.find("orient") != string::npos)
            m_Nav.orient.push_back(pt);
        else if(fn.find("fork") != string::npos)
            m_Nav.fork.push_back(pt);
    }
    for(auto&& mesh: meshes){
        string fn = mesh->material()->texture()->filename();
        auto pt = calc_origin(mesh);

        if(fn.find("nav") != string::npos)
        {
            mesh->detach();
        }
        else if(fn.find("orient") != string::npos)
        {
            mesh->detach();
        }
        else if(fn.find("fork") != string::npos)
        {
            mesh->detach();
        }
        else if(fn.find("box") != string::npos ||
            fn.find("pack") != string::npos)
        {
            //mesh->bakeable(true);
            //m_BatchMeshes[fn].push_back(mesh);
            mesh->detach();
        }
        else if(fn.find("spawn-clerk") != string::npos)
        {
            //LOG(Vector::to_string(pt));
            auto clerk = make_shared<Enemy>(
                &m_Nav, pt + Axis::Y * 0.5f,
                m_pPlayer.get(), m_pPhysics.get(), m_pResources
            );
            //auto clerk = m_pQor->make<Mesh>("clerk.obj");
            m_Enemies.push_back(clerk);
            m_pRoot->add(clerk);
            mesh->detach();
        }
        else if(fn.find("computer-crt") != string::npos)
        {
            Comp* closest_comp = nullptr;
            glm::vec3 closest_pos;
            float closest_dist;
            for(auto&& cmp: m_Comps)
            {
                float dist = glm::length(cmp.pos - pt);
                if(!closest_comp || dist < closest_dist)
                {
                    closest_comp = &cmp;
                    closest_pos = pt;
                    closest_dist = dist;
                }
            }

            //LOGf("dist %s", closest_dist);
            if(closest_comp && closest_dist <= 0.5f)
            {
                //LOG("batch");
                closest_comp->meshes.push_back(mesh);
                m_CompMeshes[mesh] = closest_comp->index;
            }
            else
            {
                //LOG("new comp");
                Comp c;
                c.pos = pt;
                c.meshes.push_back(mesh);
                m_Comps.push_back(c);
                m_CompMeshes[mesh] = m_Comps.size()-1;
                m_Comps[m_Comps.size()-1].index = m_Comps.size()-1;
            }
            mesh->set_physics(Node::STATIC);
        }
        else
        {
            mesh->set_physics(Node::STATIC);
        }
    }

    // batch meshes of same materials together for speed
    //for(auto&& v: m_BatchMeshes)
    //{
    //    unsigned batch_index = 0;
    //    std::vector<glm::vec3> batch_verts;
    //    std::vector<glm::uvec3> batch_indices;
    //    for(auto&& bm: v)
    //    {
    //        auto geom = std::dynamic_pointer_cast<MeshIndexedGeometry>(bm.second->geometry());
    //        auto verts = geom->verts();
    //        auto indices = geom->indices();
    //        if(batch_index)
    //            for(auto& idx: indices)
    //                idx += base_index;
    //        batch_verts.insert(batch_verts.begin(), ENTIRE(verts));
    //        batch_indices.insert(batch_indices.begin(), ENTIRE(indices));
    //        batch_index += geom->size();
    //    }
    //    auto mesh = std::make_shared<Mesh>(std::make_shared<MeshGeometry>(
    //        batch_verts, batch_indices
    //    ));
    //    m_pScene->add(mesh);
    //    //assert(geom);
    //    //bm->detach();
    //}
    
    m_pPhysics->generate(m_pRoot.get(), Physics::GEN_RECURSIVE);
    m_pPhysics->world()->setGravity(btVector3(0.0, -9.8, 0.0));
    auto body = (btRigidBody*)m_pPlayer->body()->body();
    body->setActivationState(DISABLE_DEACTIVATION);
    
    float height = 1.0f;
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
    //m_pInstructions = std::make_shared<Text>(m_pFont);
    //m_pInstructions->align(Text::CENTER);
    //m_pInstructions->position(glm::vec3(sw * 2.0f, sh * (3.0f/4.0f), 0.0f));
    //m_pInstructions->set("Hack the computers, don't draw their attention...");
    m_pShadowText = std::make_shared<Text>(m_pFont);
    m_pShadowText->align(Text::CENTER);
    m_pShadowText->position(glm::vec3(sw / 2.0f + 2.0f, sh / 4.0f + 2.0f, 0.0f));
    m_pShadowText->color(Color::black());
    m_pOrthoRoot->add(m_pShadowText);
    m_pOrthoRoot->add(m_pText);
    //m_pOrthoRoot->add(m_pInstructions);
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
    //if(m_pController->button("fire")){
    bool hacked = false;
    auto hits = m_pPhysics->hits(m_pPlayer->position(), m_pPlayer->node()->orient_to_world(-Axis::Z) * 100.0f);
    for(auto&& hit: hits)
    {
        auto mesh = (Mesh*)std::get<0>(hit);
        auto fn = mesh->material()->texture()->filename();
        //LOGf("hit: %s", fn);
        float dist = glm::length(std::get<1>(hit) - m_pPlayer->position());
        
        if(m_pController->button("fire")){
            if(fn.find("computer") != string::npos && fn.find("hacked") == string::npos)
            {
                if(dist < 2.0f)
                {
                    hacked = true;
                    Comp* comp = nullptr;
                    for(auto&& c: m_Comps)
                    {
                        for(auto&& m: c.meshes)
                        {
                            if(m == mesh){
                                comp = &c;
                                
                                break;
                            }
                        }
                        if(comp)
                            break;
                    }
                    assert(comp);
                    for(auto&& m: comp->meshes){
                        m->material(make_shared<MeshMaterial>(
                            m_pResources->cache_as<Texture>("computer-hacked.png")
                        ));
                    }
                }
            }
        }
        if(fn.find("door") != string::npos && dist < 2.0f)
        {
            if(m_Hacked == m_Comps.size())
                m_pQor->change_state("intro");
        }
    }
    if(hacked)
    {
        m_pPlayer->hacking(true);
        
        ++m_Hacked;
        //LOGf("%s", (m_Comps.size() - m_Hacked));
        if(m_Hacked == m_Comps.size())
            m_Countdown = 10.0f;
        
        Sound::play(m_pCamera.get(),
            "hack" + to_string(std::rand()%MAX_HAX) + ".wav", m_pResources
        );
        Sound::play(m_pCamera.get(), "noise.wav", m_pResources);

        auto txt = string("HACKING ") +
            TARGETS[std::rand()%(sizeof TARGETS / sizeof TARGETS[0])];
        m_pText->set(txt);
        m_pShadowText->set(txt);
        m_TextTime = 2.0f;
    }else{
        m_pPlayer->hacking(false);
    }
    
    if(m_Countdown > -K_EPSILON){
        int countdown_before = int(std::ceil(m_Countdown));
        m_Countdown = std::max(0.0f, m_Countdown - t.s());
        int countdown_now = int(std::ceil(m_Countdown));
        if(countdown_before != countdown_now)
        {
            string msg = "EXIT IN ";
            m_pText->set(msg + to_string(countdown_now));
            m_pShadowText->set(msg + to_string(countdown_now));
            Sound::play(m_pCamera.get(), "alarm.wav", m_pResources);
        }
        if(m_Countdown < K_EPSILON)
        {
            m_pQor->change_state("intro");
        }
    }
    else
    {
        m_TextTime = std::max(0.0f, m_TextTime - t.s());
        if(m_TextTime <= K_EPSILON){
            m_pText->set("");
            m_pShadowText->set("");
        }
    }

    *m_pCamera->matrix() = *m_pPlayer->node()->matrix();
    m_pCamera->position(m_pPlayer->node()->position(Space::WORLD));
    auto zofs = m_pPlayer->node()->orient_from_world(2.0f * Axis::Z);
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
    m_pPlayer->node()->rotate(m_pInput->mouse_rel().x / 1000.0f, glm::vec3(0.0f, -1.0f, 0.0f));
    if(m_pController->button("up"))
        v += Matrix::heading(*m_pPlayer->node()->matrix(Space::WORLD)) *
            m_pController->button("up").pressure();
    if(m_pController->button("down"))
        v -= Matrix::heading(*m_pPlayer->node()->matrix(Space::WORLD)) *
            m_pController->button("down").pressure();
    if(m_pController->button("left"))
        v -= Matrix::right(*m_pPlayer->node()->matrix(Space::WORLD)) *
            m_pController->button("left").pressure();
    if(m_pController->button("right"))
        v += Matrix::right(*m_pPlayer->node()->matrix(Space::WORLD)) *
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
        m_pPlayer->mesh()->position(Axis::Y * m_BobAmp * sinf(K_TAU * m_BobPhase));
    }
    //else
    //    m_WasMoving = false;
    glm::vec3 now_v = m_pPlayer->velocity();
    v.y = now_v.y + more_y;

    m_pPlayer->velocity(v);
    
    if(m_pPlayer->spotted())
        m_pQor->change_state("intro");
    
    m_pRoot->logic(t);
    m_pOrthoRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->winding(false);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
    
    m_pPipeline->winding(true);
    m_pPipeline->render(m_pOrthoRoot.get(), m_pOrthoCamera.get(),
        nullptr, Pipeline::NO_CLEAR | Pipeline::NO_DEPTH
    );
}

