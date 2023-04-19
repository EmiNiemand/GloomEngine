//
// Created by masterktos on 30.03.23.
//

#include "GloomEngine.h"
#include "Components/Scripts/PlayerManager.h"
#include "EngineManagers/HIDManager.h"
#include "GameObjectsAndPrefabs/GameObject.h"
#include "GameObjectsAndPrefabs/Prefab.h"
#include "Components/Scripts/PlayerInput.h"
#include "Components/Scripts/PlayerMovement.h"
#include "Components/Scripts/PlayerEquipment.h"
#include "Components/Scripts/PlayerUI.h"
#include "Components/UI/Button.h"

PlayerManager::PlayerManager(const std::shared_ptr<GameObject> &parent, int id)
                            : Component(parent, id) {}

void PlayerManager::Start() {
    movement = parent->AddComponent<PlayerMovement>();
    equipment = parent->AddComponent<PlayerEquipment>();
    equipment->Setup(5, 10);
    playerUI = GameObject::Instantiate("PlayerUI", parent)->AddComponent<PlayerUI>();
    playerUI->UpdateCash(equipment->GetCash());
    playerUI->UpdateRep(equipment->GetRep());
    sessionStarterUI = GameObject::Instantiate("SessionStarterUI", parent);
    GameObject::Instantiate("SessionUI", parent);

    moveInput = glm::vec2(0);
    inputEnabled = true;
	uiActive = false;
    pauseActive = false;

    pauseMenu = GloomEngine::GetInstance()->FindGameObjectWithName("Pause")->GetComponent<PauseMenu>();
    optionsMenu = GloomEngine::GetInstance()->FindGameObjectWithName("Options")->GetComponent<OptionsMenu>();
    shopMenu = GloomEngine::GetInstance()->FindGameObjectWithName("Shop")->GetComponent<ShopMenu>();

    BuyInstrument(0, Prefab::GetInstrument(InstrumentName::Clap));
    Component::Start();
}

void PlayerManager::Update() {
    Component::Update();
    PollInput();
}

#pragma region Equipment events
bool PlayerManager::BuyInstrument(int price, const std::shared_ptr<Instrument> &instrument) {
    if(!equipment->BuyInstrument(price, instrument)) return false;

    playerUI->UpdateCash(equipment->GetCash());
    return true;
}
#pragma endregion

#pragma region Movement Events
void PlayerManager::OnMove(glm::vec2 moveVector) {
    if(moveVector != glm::vec2(0))
        moveVector = glm::normalize(moveVector);
	movement->Move(moveVector);
}
#pragma endregion

#pragma region Interaction Events
void PlayerManager::OnInteract() {
    if (pauseActive) return;
    if (uiActive && !GloomEngine::GetInstance()->FindGameObjectWithName("Shop")->GetEnabled()) return;
    uiActive = !uiActive;

    if (uiActive) {
        GloomEngine::GetInstance()->timeScale = 0;
        shopMenu->ShowMenu();
    } else {
        GloomEngine::GetInstance()->timeScale = 1;
        shopMenu->HideMenu();
    }
}
#pragma endregion

#pragma region UI Events
void PlayerManager::OnMenuToggle() {
    if (!uiActive) {
        pauseActive = !pauseActive;

        if (pauseActive) {
            GloomEngine::GetInstance()->timeScale = 0;
            pauseMenu->ShowMenu();
        } else {
            GloomEngine::GetInstance()->timeScale = 1;
            pauseMenu->HideMenu();
        }
    }
}

void PlayerManager::OnApply() {
    if(!uiActive && !pauseActive) return;
    if (GloomEngine::GetInstance()->FindGameObjectWithName("Pause")->GetEnabled()) {
        pauseMenu->OnClick();
    } else if (GloomEngine::GetInstance()->FindGameObjectWithName("Options")->GetEnabled()) {
        optionsMenu->OnClick();
    } else if (GloomEngine::GetInstance()->FindGameObjectWithName("Shop")->GetEnabled()) {
        shopMenu->OnClick();
    } else if (sessionStarter) {
        sessionStarter->OnClick();
    }
}

void PlayerManager::OnUIMove(glm::vec2 moveVector) {
    if (GloomEngine::GetInstance()->FindGameObjectWithName("Pause")->GetEnabled()) {
        pauseMenu->ChangeActiveButton(moveVector);
    } else if (GloomEngine::GetInstance()->FindGameObjectWithName("Options")->GetEnabled()) {
        optionsMenu->ChangeActiveButton(moveVector);
    } else if (GloomEngine::GetInstance()->FindGameObjectWithName("Shop")->GetEnabled()) {
        shopMenu->ChangeActiveButton(moveVector);
    } else if (sessionStarter) {
        sessionStarter->ChangeActiveButton(moveVector);
    }
}
#pragma endregion

#pragma region Music Session Events
void PlayerManager::OnSessionToggle() {
    if (pauseActive) return;
    if (uiActive && !session && !sessionStarter) return;
    if (session) {
        uiActive = false;
        session->Stop();
        session.reset();
        return;
    }
    if (sessionStarter) {
        uiActive = false;
        sessionStarter->Stop();
        sessionStarter.reset();
        GloomEngine::GetInstance()->timeScale = 1;
        return;
    }

    uiActive = true;
    GloomEngine::GetInstance()->timeScale = 0;
    sessionStarter = GameObject::Instantiate("SessionStarter", sessionStarterUI)->AddComponent<SessionStarter>();
    sessionStarter->Setup(equipment->instruments);
}

void PlayerManager::OnSoundPlay(int index) {
    if(!session) return;

    session->PlaySample(index);
}
void PlayerManager::PlayedPattern(const std::shared_ptr<MusicPattern> &pat) {
    //TODO: uncomment when crowd manager gets implemented
//        crowdManager.PlayedPattern(pat);

    if (!pat) return;

    //TODO: uncomment when crowd manager gets implemented
    equipment->AddReward(1 /*crowdManager->GetCrowdSatisfaction()/100*/);

    playerUI->UpdateCash(equipment->cash);
    playerUI->UpdateRep(equipment->rep);
}

void PlayerManager::CreateMusicSession(std::shared_ptr<Instrument> instrument) {
    GloomEngine::GetInstance()->timeScale = 1;
    sessionStarter->Stop();
    sessionStarter.reset();
    session = parent->AddComponent<MusicSession>();
    session->Setup(std::move(instrument));
}

#pragma endregion

void PlayerManager::PollInput() {
	if(!inputEnabled) return;

	auto hid = HIDManager::GetInstance();
	glm::vec2 readMoveVector(0);

    // UI Input
    // --------
	for (auto key : PlayerInput::Menu)
		if(hid->IsKeyDown(key.first)) OnMenuToggle();

    for (auto key : PlayerInput::Interact)
        if(hid->IsKeyDown(key.first)) OnInteract();

    for (auto key: PlayerInput::StartSession)
        if (hid->IsKeyDown(key.first)) OnSessionToggle();

	if((uiActive || pauseActive) && !session) {
		for (auto key: PlayerInput::Move) {
			if (hid->IsKeyDown(key.first)) {
				readMoveVector.y = key.second == 0 ? 1 : key.second == 2 ? -1 : readMoveVector.y;
				readMoveVector.x = key.second == 1 ? 1 : key.second == 3 ? -1 : readMoveVector.x;
			}
		}

		for (auto key : PlayerInput::Apply)
			if(hid->IsKeyDown(key.first)) OnApply();
		if(readMoveVector != glm::vec2(0))
			OnUIMove(readMoveVector);

		return;
	}

	for (auto key: PlayerInput::Move) {
		if (hid->IsKeyPressed(key.first)) {
			readMoveVector.y = key.second == 0 ? 1 : key.second == 2 ? -1 : readMoveVector.y;
			readMoveVector.x = key.second == 1 ? 1 : key.second == 3 ? -1 : readMoveVector.x;
		}
	}

    for (auto key: PlayerInput::PlaySound)
        if (hid->IsKeyDown(key.first)) OnSoundPlay(key.second);

	if(readMoveVector != moveInput)
		OnMove(readMoveVector);
	moveInput = readMoveVector;
}