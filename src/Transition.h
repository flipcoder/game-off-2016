#ifndef _PREGAMESTATE_H
#define _PREGAMESTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Console.h"
#include "Qor/Sound.h"
#include "Qor/Text.h"
#include "Qor/Material.h"

class Qor;

class Transition:
    public State
{
    public:
        
        Transition(Qor* engine);
        virtual ~Transition();

        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

    private:
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;
        Cache<Resource, std::string>* m_pResources = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Sound> m_pMusic;
        std::shared_ptr<Font> m_pFont;
        std::shared_ptr<Text> m_pText;
        std::string m_Text;
        float m_TextVisibility = 0.0f;
        bool m_Done = false;// done with game?

};

#endif


