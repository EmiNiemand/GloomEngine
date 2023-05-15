#include "EngineManagers/SceneManager.h"
#include "GloomEngine.h"
#include "Components/Renderers/Camera.h"
#include "GameObjectsAndPrefabs/GameObject.h"
#include "GameObjectsAndPrefabs/Prefab.h"
#include "GameObjectsAndPrefabs/Prefabs/MainMenuPrefab.h"
#include "Game.h"
#include "Components/Renderers/Animator.h"
#include "Components/Renderers/Renderer.h"
#include "Components/Scripts/NewGameMenu.h"
#include "Components/Scripts/LoadGameMenu.h"

#ifdef DEBUG
#include <tracy/Tracy.hpp>
#endif

SceneManager::SceneManager() = default;

SceneManager::~SceneManager() = default;

SceneManager* SceneManager::GetInstance() {
    if (sceneManager == nullptr) {
        sceneManager = new SceneManager();
    }
    return sceneManager;
}

void SceneManager::InitializeScene() {
#ifdef DEBUG
    ZoneScopedNC("Scene Init", 0xDC143C);
#endif
    activeScene = GameObject::Instantiate("Scene", nullptr, Tags::SCENE);
    Camera::activeCamera = GameObject::Instantiate("Camera", activeScene, Tags::CAMERA);
}

void SceneManager::LoadScene(const std::string& scene) {
    if (scene == "Scene") {
        if (!GloomEngine::GetInstance()->FindGameObjectWithName("NewGameMenu")->GetComponent<NewGameMenu>()->file.empty())
            file = GloomEngine::GetInstance()->FindGameObjectWithName("NewGameMenu")->GetComponent<NewGameMenu>()->file;
        else
            file = GloomEngine::GetInstance()->FindGameObjectWithName("LoadGameMenu")->GetComponent<LoadGameMenu>()->file;
        ClearScene();
        InitializeScene();
        GloomEngine::GetInstance()->game->activeCamera = Camera::activeCamera;
        GloomEngine::GetInstance()->game->activeScene = activeScene;
        GloomEngine::GetInstance()->game->InitializeGame();
    } else if (scene == "MainMenu") {
        ClearScene();
        activeScene = GameObject::Instantiate("MainMenuScene", nullptr, Tags::SCENE);
        Prefab::Instantiate<MainMenuPrefab>();
    }
}

void SceneManager::ClearScene() const {
    activeScene->RemoveAllChildren();
    Animator::animationModels.clear();
    Animator::animations.clear();
    Renderer::models.clear();
    GameObject::Destroy(activeScene);
}

void SceneManager::Free() {
    ClearScene();
    Camera::activeCamera = nullptr;
    activeScene = nullptr;
}

