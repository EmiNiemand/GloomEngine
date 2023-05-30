//
// Created by masterktos on 06.04.23.
//

#include "Components/Scripts/SessionUI/SessionUI.h"
#include "Components/UI/Image.h"
#include "Components/UI/Text.h"
#include "GameObjectsAndPrefabs/GameObject.h"
#include "Components/Audio/AudioSource.h"
#include "Components/Animations/UIAnimator.h"

SessionUI::SessionUI(const std::shared_ptr<GameObject> &parent, int id) : Component(parent, id) {}

void SessionUI::Setup(int bpm, const std::vector<std::shared_ptr<Sample>> &samples, std::string metronomePath) {
    metronomeSoundEnabled = true;
    metronomeVisualEnabled = true;

    MetronomeSetup(metronomePath, bpm);
    AccuracyFeedbackSetup();

    // Set up sound samples
    // --------------------
    for (const auto& sample: samples)
    {
        sampleSources.push_back(GameObject::Instantiate("SampleSource", parent)->AddComponent<AudioSource>());
        sampleSources.back()->LoadAudioData(sample->clipPath.c_str(), AudioType::Direct);

        sampleImages.push_back(GameObject::Instantiate("SampleImage", parent)
                                    ->AddComponent<Image>());
        sampleImages.back()->pivot = {0.5, 0.5};
    }
}

void SessionUI::SetCheatSheet(const std::string& cheatSheetPath) {
    cheatSheet = GameObject::Instantiate("CheatSheet", parent)->AddComponent<Image>();
    cheatSheet->LoadTexture(451, -1100, cheatSheetPath, -1);
}

void SessionUI::SetInstrumentControl(const std::string &instrumentControlPath) {
    instrumentControl = GameObject::Instantiate("InstrumentControl", parent)->AddComponent<Image>();
    instrumentControl->LoadTexture(451, -1100, instrumentControlPath, -1);
}

void SessionUI::PlaySound(int index) {
    sampleSources[index]->ForcePlaySound();
    //spdlog::info("[SUI] Played sound at index "+std::to_string(index)+"!");
}

void SessionUI::ToggleCheatSheet() {
    if (GloomEngine::GetInstance()->FindGameObjectWithName("CheatSheetAnimator")) return;
    cheatSheetActive = !cheatSheetActive;
    if (cheatSheetActive) {
        GameObject::Instantiate("CheatSheetAnimator", parent->parent)
                ->AddComponent<UIAnimator>()->Setup(cheatSheet, {
                        {AnimatedProperty::Position, glm::vec3(451.0f, -50.0f, 0.0f), 0.5f}
                });
    } else {
        GameObject::Instantiate("CheatSheetAnimator", parent->parent)
                ->AddComponent<UIAnimator>()->Setup(cheatSheet, {
                        {AnimatedProperty::Position, glm::vec3(451.0f, -1100.0f, 0.0f), 0.5f}
                });
    }
}

void SessionUI::ToggleInstrumentControl() {
    if (GloomEngine::GetInstance()->FindGameObjectWithName("InstrumentControlAnimator")) return;
    instrumentControlActive = !instrumentControlActive;
    if (instrumentControlActive) {
        GameObject::Instantiate("InstrumentControlAnimator", parent->parent)
                ->AddComponent<UIAnimator>()->Setup(instrumentControl, {
                {AnimatedProperty::Position, glm::vec3(451.0f, -50.0f, 0.0f), 0.5f}
        });
    } else {
        GameObject::Instantiate("InstrumentControlAnimator", parent->parent)
                ->AddComponent<UIAnimator>()->Setup(instrumentControl, {
                {AnimatedProperty::Position, glm::vec3(451.0f, -1100.0f, 0.0f), 0.5f}
        });
    }
}

void SessionUI::UpdateAccuracy(float fraction) {
    int index = 0;
    for(auto threshold : accuracyThresholds)
    {
        if(fraction > threshold) index++;
        else break;
    }

    accuracyRatingAnimator[index]->Reset();
    //accuracyFeedback->text = accuracyTexts[index];
    //spdlog::info("[SUI] Accuracy rating:" + accuracyTexts[index]);
}

void SessionUI::Update() {
    if (metronomeImage->GetAlpha() == 1.0f) {
        tickSound->PlaySound();
    }
    Component::Update();
}

# pragma region Helper methods
void SessionUI::MetronomeSetup(const std::string& metronomePath, int bpm) {
    metronomeImage = GameObject::Instantiate("Metronome", parent)->AddComponent<Image>();
    metronomeImage->LoadTexture(0, 0, metronomePath, -0.5);
    GameObject::Instantiate("MetronomeAnimator", parent)->AddComponent<UIAnimator>()->Setup(metronomeImage, {
            {AnimatedProperty::Alpha, glm::vec3(0.2f), 30.0f / (float)bpm},
            {AnimatedProperty::Alpha, glm::vec3(1.0f), 30.0f / (float)bpm}
    }, AnimationBehaviour::Looping);

    tickSound = parent->AddComponent<AudioSource>();
    tickSound->LoadAudioData("res/sounds/direct/tick.wav", AudioType::Direct);
    tickSound->IsLooping(false);
}

void SessionUI::AccuracyFeedbackSetup() {
    accuracyRatingAnimator = {
            GameObject::Instantiate("AccuracyPoorAnimator", parent)->AddComponent<UIAnimator>(),
            GameObject::Instantiate("AccuracyNiceAnimator", parent)->AddComponent<UIAnimator>(),
            GameObject::Instantiate("AccuracyGreatAnimator", parent)->AddComponent<UIAnimator>(),
            GameObject::Instantiate("AccuracyPerfectAnimator", parent)->AddComponent<UIAnimator>()
    };
    std::string accuracyImagePaths[] = {
            "UI/Sesja/accuracyPoor.png", "UI/Sesja/accuracyNice.png",
            "UI/Sesja/accuracyAwesome.png", "UI/Sesja/AccuracyPerfect.png"
    };
    for (int i = 0; i < 4; ++i) {
        auto ratingImage = GameObject::Instantiate("AccuracyImage", parent)->AddComponent<Image>();
        ratingImage->LoadTexture(0, 0,accuracyImagePaths[i], 0.7f);
        ratingImage->SetPosition(960 - ratingImage->GetWidth()/2, 540 - ratingImage->GetHeight()/2);
        ratingImage->SetAlpha(0);
        accuracyRatingAnimator[i]->Setup(ratingImage, {
                {AnimatedProperty::Alpha, glm::vec3(1.0f), 0},
                {AnimatedProperty::Alpha, glm::vec3(0.0f), 1}
        }, AnimationBehaviour::Resetable);
    }
}

void SessionUI::OnDestroy() {
    accuracyRating.clear();
    accuracyRatingAnimator.clear();
    metronomeImage.reset();
    tickSound.reset();
    sampleSources.clear();
    sampleImages.clear();
    sampleAnimators.clear();
    cheatSheet.reset();
    instrumentControl.reset();
    Component::OnDestroy();
}

#pragma endregion