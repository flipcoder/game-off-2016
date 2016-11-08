#ifndef _GAMESTATE_H
#define _GAMESTATE_H

#include "Qor/Node.h"
#include "Qor/State.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"
#include "Qor/Pipeline.h"
#include "Qor/Mesh.h"
#include "Qor/Sound.h"
#include "Qor/Text.h"

class Qor;

class Game:
    public State
{
    public:
        
        Game(Qor* engine);
        virtual ~Game();

        virtual void preload() override;
        virtual void enter() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

        struct Comp
        {
            glm::vec3 pos;
            unsigned index;
            std::vector<Mesh*> meshes;
        };

    private:
        
        static const char* const TARGETS[];
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Controller* m_pController = nullptr;
        Pipeline* m_pPipeline = nullptr;
        Cache<Resource, std::string>* m_pResources = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Node> m_pOrthoRoot;
        std::shared_ptr<Camera> m_pOrthoCamera;
        
        std::shared_ptr<Sound> m_pMusic;
        std::shared_ptr<Physics> m_pPhysics;

        std::shared_ptr<Mesh> m_pPlayer;
        std::shared_ptr<Node> m_pPlayerNode;
        std::shared_ptr<Mesh> m_pPlayerMesh;
        std::shared_ptr<Mesh> m_pScene;

        float m_CamHeight = 1.05f;
        float m_CamTilt = -0.031f;
        //bool m_WasMoving = false;
        float m_BobPhase = 0.0f;
        float m_BobAmp = 0.02f;
        float m_BobSpeed = 4.0f;

        const int MAX_HAX = 5;

        std::shared_ptr<Font> m_pFont;
        std::shared_ptr<Text> m_pText;
        std::shared_ptr<Text> m_pShadowText;

        std::vector<std::shared_ptr<Mesh>> m_Enemies;
        std::vector<Comp> m_Comps;
        std::map<Mesh*, unsigned> m_CompMeshes;

        float m_TextTime = 0.0f;
};

#endif


