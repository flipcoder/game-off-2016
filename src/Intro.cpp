#include "Intro.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Intro :: Intro(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources())
{}

void Intro :: preload()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;
    
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pMusic = m_pQor->make<Sound>("menu.ogg");
    m_pRoot->add(m_pMusic);
    m_pRoot->add(m_pCamera);
    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    //m_pPlayer = m_pQor->make<Mesh>("guy.obj");
    //m_pRoot->add(m_pPlayer);

    auto _this = this;
    StateMachine::on_enter("screen", "title", [_this, sw, sh](){
        auto mat = make_shared<Material>("bg.png", _this->m_pResources);
        auto mx = mat->size().x;
        auto my = mat->size().y;
        if(_this->m_pBG)
            _this->m_pBG->detach();
        _this->m_pBG = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(vec2(sw/2.0f, sh/2.0f), vec2(-sw/2.0f, -sh/2.0f))),
            vector<shared_ptr<IMeshModifier>>{
                make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
            },
            make_shared<MeshMaterial>(mat)
        );
        _this->m_pBG->position(glm::vec3(sw/2.0f, sh/2.0f, 0.0f));
        _this->m_pRoot->add(_this->m_pBG);
    });
    StateMachine::on_enter("screen", "wanted", [_this, sw, sh](){
        auto mat = make_shared<Material>("wanted.png", _this->m_pResources);
        auto mx = mat->size().x;
        auto my = mat->size().y;
        if(_this->m_pBG)
            _this->m_pBG->detach();
        _this->m_pBG = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(vec2(sw/2.0f, sh/2.0f), vec2(-sw/2.0f, -sh/2.0f))),
            vector<shared_ptr<IMeshModifier>>{
                make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
            },
            make_shared<MeshMaterial>(mat)
        );
        _this->m_pBG->position(glm::vec3(sw/2.0f, sh/2.0f, 0.0f));
        _this->m_pRoot->add(_this->m_pBG);
    });

    
    StateMachine::on_enter("screen", "vcr", [_this, sw, sh](){
    
        auto mat = make_shared<Material>("vcrosd.png", _this->m_pResources);
        auto mx = mat->size().x;
        auto my = mat->size().y;
        if(_this->m_pBG)
            _this->m_pBG->detach();
        _this->m_pBG = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(vec2(sw/2.0f, sh/2.0f), vec2(-sw/2.0f, -sh/2.0f))),
            vector<shared_ptr<IMeshModifier>>{
                make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
            },
            make_shared<MeshMaterial>(mat)
        );
        //m_BGScale = 0.5f;
        _this->m_pBG->position(glm::vec3(sw/2.0f, sh/2.0f, 0.0f));
        _this->m_pRoot->add(_this->m_pBG);

        _this->m_pFont = std::make_shared<Font>(
            _this->m_pResources->transform(string("vcr.ttf:") +
                to_string(int(sw / 24.0f + 0.5f))),
            _this->m_pResources
        );
        _this->m_pText = std::make_shared<Text>(_this->m_pFont);
        _this->m_pText->align(Text::CENTER);
        _this->m_pText->position(glm::vec3(sw / 2.0f, sh / 2.0f, 1.0f));
        _this->m_pRoot->add(_this->m_pText);

        _this->m_Text = "Hack all the computers\nwhen no one is looking.\nThe clerks can spot a hacker\nfrom a mile away.";
    });
    state("screen", "title");
}

Intro :: ~Intro()
{
    m_pPipeline->partitioner()->clear();
}

void Intro :: enter()
{
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
    m_pMusic->play();
}

void Intro :: logic(Freq::Time t)
{
    State::logic(t);
    
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    if(state("screen")=="vcr")
    {
        if(m_TextVisibility >= 1.0f*m_Text.size() - K_EPSILON){
            if(m_pInput->key(SDLK_SPACE).pressed_now() || m_pInput->key(SDLK_RETURN).pressed_now())
                m_pQor->change_state("game");
        }

        m_TextVisibility = std::min(1.0f*m_Text.size(), m_TextVisibility + t.s() * 20.0f);
        m_pText->set(m_Text.substr(0,int(m_TextVisibility)));
    }
    else if(state("screen")=="title")
    {
        if(m_pInput->key(SDLK_SPACE).pressed_now() || m_pInput->key(SDLK_RETURN).pressed_now()){
            state("screen","wanted");
        }
    }
    else if(state("screen")=="wanted")
    {
        if(m_pInput->key(SDLK_SPACE).pressed_now() || m_pInput->key(SDLK_RETURN).pressed_now()){
            state("screen","vcr");
        }
    }

    
    m_pRoot->logic(t);
    //m_pBG->reset_orientation();
    //m_BGScale = std::min(1.0f, m_BGScale + 0.001f * t);
    //m_pBG->scale(glm::vec3(m_BGScale, m_BGScale, 1.0f));
}

void Intro :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

