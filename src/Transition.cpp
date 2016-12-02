#include "Transition.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Transition :: Transition(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources())
{}

void Transition :: preload()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);

    m_pMusic = m_pQor->make<Sound>("status.ogg");
    m_pRoot->add(m_pMusic);
    
    auto mat = make_shared<Material>("vcrosd.png", m_pResources);
    auto bg = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(sw, sh), vec2(0.0f, 0.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)))
        },
        make_shared<MeshMaterial>(mat)
    );
    m_pRoot->add(bg);
    
    m_pFont = std::make_shared<Font>(
        m_pResources->transform(string("vcr.ttf:") +
            to_string(int(sw / 24.0f + 0.5f))),
        m_pResources
    );
    m_pText = std::make_shared<Text>(m_pFont);
    m_pText->align(Text::CENTER);
    m_pText->position(glm::vec3(sw / 2.0f, sh / 2.0f, 1.0f));
    m_pRoot->add(m_pText);

    m_Text = "There's no way this will work.\nYou can't just go around hacking\neverything in sight...";
}

Transition :: ~Transition()
{
    m_pPipeline->partitioner()->clear();
}

void Transition :: enter()
{
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
    m_pMusic->play();

    string mapno = m_pQor->args().value_or("map","1");
    int map_number = boost::lexical_cast<int>(mapno) + 1;
    m_pQor->args().set("map",to_string(map_number));
    if(not m_pQor->exists("store" + to_string(map_number) + ".obj"))
    {
        m_Text = "Congrats Bernard, you pretty much\nhacked the whole world by now.";
        m_Done = true;
    }
}

void Transition :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);

    if(m_TextVisibility >= 1.0f*m_Text.size() - K_EPSILON){
        if(m_pInput->key(SDLK_SPACE).pressed_now() || m_pInput->key(SDLK_RETURN).pressed_now())
        {
            if(m_Done)
                m_pQor->change_state("intro");
            else
                m_pQor->change_state("game");
        }
    }

    m_TextVisibility = std::min(1.0f*m_Text.size(), m_TextVisibility + t.s() * 20.0f);
    m_pText->set(m_Text.substr(0,int(m_TextVisibility)));

}

void Transition :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

