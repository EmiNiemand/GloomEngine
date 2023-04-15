//
// Created by Adrian on 07.04.2023.
//

#include <al.h>
#include <alc.h>
#include "Components/Audio/AudioListener.h"
#include "EngineManagers/AudioManager.h"
#include "GameObjectsAndPrefabs/GameObject.h"

AudioListener::AudioListener(const std::shared_ptr<GameObject> &parent, int id) : Component(parent, id) {}

AudioListener::~AudioListener() = default;

void AudioListener::Start() {
    position = parent->transform->GetLocalPosition();

    alListenerf(AL_GAIN, gain);
    alListener3f(AL_POSITION, position.x, position.y, position.z);
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

    // "up" and "at" vectors, i.e. at the head of the player
    ALfloat listenerOrientation[] = {0.0,0.0,-1.0, 0.0,1.0,0.0};
    alListenerfv(AL_ORIENTATION,listenerOrientation);

    Component::Start();
}

void AudioListener::Update() {
    position = parent->transform->GetLocalPosition();
    alListener3f(AL_POSITION, position.x, position.y, position.z);
    Component::Update();
}

void AudioListener::OnCreate() {
    AudioManager::GetInstance()->audioListener = std::dynamic_pointer_cast<AudioListener>(shared_from_this());
    Component::OnCreate();
}

// Sets player audio gain. Type: float, default 1.0f
void AudioListener::SetGain(float val) {
    gain = val;
    alListenerf(AL_GAIN, gain);
}

// Sets player audio velocity. Type: glm::vec3.
// Used in calculating doppler shift (moving objects emitting sound)
void AudioListener::SetVelocity(glm::vec3 vel) {
    velocity = vel;
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}
