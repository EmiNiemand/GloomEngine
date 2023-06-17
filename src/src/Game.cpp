#include "Game.h"
#include "GloomEngine.h"
#include "EngineManagers/SceneManager.h"
#include "GameObjectsAndPrefabs/GameObject.h"
#include "Components/Renderers/Renderer.h"
#include "Components/Renderers/CubeMap.h"
#include "Components/Renderers/Camera.h"
#include "Components/Renderers/Lights/PointLight.h"
#include "Components/Renderers/Lights/DirectionalLight.h"
#include "Components/Renderers/Lights/SpotLight.h"
#include "Components/PhysicsAndColliders/Rigidbody.h"
#include "Components/PhysicsAndColliders/BoxCollider.h"
#include "Components/Scripts/Player/PlayerManager.h"
#include "Components/Scripts/Player/PlayerMovement.h"
#include "Components/Scripts/Menus/PauseMenu.h"
#include "Components/Scripts/Menus/OptionsMenu.h"
#include "Components/Renderers/Animator.h"
#include "Components/UI/Image.h"
#include "Components/UI/Text.h"
#include "Components/UI/Button.h"
#include "GameObjectsAndPrefabs/Prefab.h"
#include "Components/Audio/AudioListener.h"
#include "Components/Audio/AudioSource.h"
#include "Components/Scripts/Menus/SavePointMenu.h"
#include "Components/Animations/GameObjectAnimator.h"
#include "Components/Scripts/Menus/MainMenu.h"
#include "GameObjectsAndPrefabs/Prefabs/Player.h"
#include "GameObjectsAndPrefabs/Prefabs/Die.h"
#include "GameObjectsAndPrefabs/Prefabs/Shop.h"
#include "GameObjectsAndPrefabs/Prefabs/ConeIndicator.h"
#include "GameObjectsAndPrefabs/Prefabs/SavePoint.h"
#include "Components/Scripts/Menus/Dialogue.h"
#include "Components/Scripts/Menus/Shopkeeper.h"
#include "EngineManagers/AIManager.h"
#include "Components/Scripts/Opponent.h"
#include "Components/Scripts/Instrument.h"

#ifdef DEBUG
#include <tracy/Tracy.hpp>
#endif

Game::Game() {
    activeCamera = Camera::activeCamera;
    activeScene = SceneManager::GetInstance()->activeScene;
}

Game::~Game() = default;

void Game::InitializeGame() const {
#ifdef DEBUG
    ZoneScopedNC("Game Init", 0xDC143C);
#endif
    // Set up camera
    // -------------
    std::shared_ptr<Camera> camera = activeCamera->AddComponent<Camera>();
    camera->cameraOffset = glm::vec3(0, 25, 25);

    // Set up cubemap
    // --------------
    auto sky = GameObject::Instantiate("CubeMap", activeScene);
    auto skyCubeMap = sky->AddComponent<CubeMap>();
    skyCubeMap->LoadTextures("skybox/");

    // Load all animations
    // -------------
    Animator::LoadAnimation("CrowdAnimations/Walk.dae");
    Animator::LoadAnimation("CrowdAnimations/Happy.dae");
    Animator::LoadAnimation("CrowdAnimations/Angry.dae");
    Animator::LoadAnimation("CrowdAnimations/Idle1.dae");
    Animator::LoadAnimation("CrowdAnimations/Idle3.dae");
    Animator::LoadAnimation("MainHero/MainHeroIdle.dae");
    Animator::LoadAnimation("MainHero/MainHeroRun.dae");
    Animator::LoadAnimation("MainHero/MainHeroClap.dae");
    Animator::LoadAnimation("MainHero/MainHeroTrumpet.dae");
    Animator::LoadAnimation("MainHero/MainHeroDrums.dae");

    // Set up player
    // -------------
    std::shared_ptr<GameObject> player = Prefab::Instantiate<Player>();

    // Set up ground
    // -------------
    std::shared_ptr<GameObject> ground = Prefab::Instantiate<Die>("Ground");
    ground->transform->SetLocalPosition({0, -4, 0});
    ground->transform->SetLocalScale({150, 2, 150});
    ground->GetComponent<Renderer>()->textScale = glm::vec2(150, 150);

    // Set up lights
    // -------------
    std::shared_ptr<GameObject> sun = GameObject::Instantiate("Sun", activeScene);
    sun->AddComponent<DirectionalLight>();
    sun->transform->SetLocalPosition({20, 40, 20});
    sun->transform->SetLocalRotation({-50, 70, 0});

    auto opponent = GameObject::Instantiate("JazzOpponent", activeScene);
    opponent->AddComponent<Renderer>()->LoadModel("Opponent/opponent.obj");
    opponent->AddComponent<BoxCollider>()->SetSize({3, 1, 3});
    opponent->transform->SetLocalPosition(glm::vec3(-87, 0, -42));
    opponent->transform->SetLocalRotation(glm::vec3(0, 45, 0));
    // 2      *   *
    // 1    *   *
    // 0  *
    auto opponentComponent = opponent->AddComponent<Opponent>();
    opponentComponent->Setup(Instrument::GetInstrument(InstrumentName::Drums),
                                              {{0, 0.5}, {1, 0.5}, {2, 0.5}, {1, 0.5}, {2, 0.5}}, 80.0f, 50);
    opponentComponent->dialogue->texts.push_back({{""},
                                                      {"Zaplac jezeli chcesz ze mna walczyc."},
                                                      {""}});
    opponentComponent->dialogue->texts.push_back({{""},
                                                  {"Walcz!."},
                                                  {""}});
    opponentComponent->winDialogue->texts.push_back({{"Pokonales mnie."},
                             {"Masz tu moja odznake Jazz Badge."},
                             {""}});
    opponentComponent->winDialogue->texts.push_back({{"Odblokowales dostep do nastepnej dzielnicy."},
                             {"Pokonaj kolejnego lidera w Electro Gymie."},
                             {""}});
    opponentComponent->lossDialogue->texts.push_back({{""},
                                                     {"Przegrales."},
                                                     {""}});

    auto dialogue = GameObject::Instantiate("GateDialogue", activeScene);
    dialogue->transform->SetLocalPosition(glm::vec3(-1, 0, -27));
    dialogue->transform->SetLocalScale(glm::vec3(0.5f));
    dialogue->AddComponent<BoxCollider>()->SetSize({1, 1, 1});
    auto dialogueAnimator = dialogue->AddComponent<Animator>();
    dialogueAnimator->LoadAnimationModel("Crowd/BasicMan002/BasicMan002.dae");
    dialogueAnimator->SetAnimation("CrowdAnimations/Idle3.dae");
    auto dialogueComponent = GameObject::Instantiate("Dialogue", dialogue)->AddComponent<Dialogue>();
    dialogueComponent->texts.push_back({{"Sorry buddy, cannot let you in - this place is under construction."},
                                                       {"Come back when a music competition begins."},
                                                       {"In a meantime - why don't you explore the rest of the city?"}});
    auto dialogueIndicator = Prefab::Instantiate<ConeIndicator>("Indicator");
    dialogueIndicator->SetParent(dialogue);
    dialogueIndicator->transform->SetLocalPosition(glm::vec3(0, 5, 0));
    dialogueIndicator->transform->SetLocalScale(glm::vec3(0.5f, 0.5f, 0.5f));

    dialogue = GameObject::Instantiate("PatternDialogue1", activeScene);
    dialogue->transform->SetLocalPosition(glm::vec3(-23.5, 0, -8));
    dialogue->transform->SetLocalRotation(glm::vec3(0, 90, 0));
    dialogue->transform->SetLocalScale(glm::vec3(0.5f));
    dialogue->AddComponent<BoxCollider>()->SetSize({1, 1, 1});
    dialogueAnimator = dialogue->AddComponent<Animator>();
    dialogueAnimator->LoadAnimationModel("Crowd/BasicMan003/BasicMan003.dae");
    dialogueAnimator->SetAnimation("CrowdAnimations/Idle3.dae");
    dialogueComponent = GameObject::Instantiate("Dialogue", dialogue)->AddComponent<Dialogue>();
    dialogueComponent->texts.push_back({{"Sprobuj zagrac pattern tego lata."},
                               {"ram pam bum pam"},
                               {"ram pam bum pam"}});
    dialogueIndicator = Prefab::Instantiate<ConeIndicator>("Indicator");
    dialogueIndicator->SetParent(dialogue);
    dialogueIndicator->transform->SetLocalPosition(glm::vec3(0, 5, 0));
    dialogueIndicator->transform->SetLocalScale(glm::vec3(0.5f, 0.5f, 0.5f));

    dialogue = GameObject::Instantiate("CrowdDialogue", activeScene);
    dialogue->transform->SetLocalPosition(glm::vec3(-5, 0, -3));
    dialogue->transform->SetLocalScale(glm::vec3(0.5f));
    dialogue->AddComponent<BoxCollider>()->SetSize({1, 1, 1});
    dialogueAnimator = dialogue->AddComponent<Animator>();
    dialogueAnimator->LoadAnimationModel("Crowd/BasicMan001/BasicMan001.dae");
    dialogueAnimator->SetAnimation("CrowdAnimations/Idle3.dae");
    dialogueComponent = GameObject::Instantiate("Dialogue", dialogue)->AddComponent<Dialogue>();
    dialogueComponent->texts.push_back({{"Pamietaj, zeby nie grac dlugo czas tego samego patternu."},
                                        {"Ludzie zaczna sie nudzic."},
                                        {""}});
    dialogueIndicator = Prefab::Instantiate<ConeIndicator>("Indicator");
    dialogueIndicator->SetParent(dialogue);
    dialogueIndicator->transform->SetLocalPosition(glm::vec3(0, 5, 0));
    dialogueIndicator->transform->SetLocalScale(glm::vec3(0.5f, 0.5f, 0.5f));

    auto shopkeeper = GameObject::Instantiate("Shopkeeper", activeScene);
    shopkeeper->transform->SetLocalPosition(glm::vec3(1.5f, 0, -2));
    shopkeeper->AddComponent<Shopkeeper>();

#ifdef DEBUG
    AIManager::GetInstance()->InitializeSpawner(50);
#else
    AIManager::GetInstance()->InitializeSpawner(50);
#endif

    camera->SetTarget(nullptr);
}

bool Game::GameLoop() {
    auto pauseMenu = GloomEngine::GetInstance()->FindGameObjectWithName("Pause");
    if (pauseMenu)
        if (pauseMenu->GetComponent<PauseMenu>())
            shouldQuit = pauseMenu->GetComponent<PauseMenu>()->gameShouldExit;

    auto mainMenu = GloomEngine::GetInstance()->FindGameObjectWithName("MainMenu");
    if (mainMenu)
        if (mainMenu->GetComponent<MainMenu>())
            shouldQuit = mainMenu->GetComponent<MainMenu>()->gameShouldExit;


    return shouldQuit;
}
