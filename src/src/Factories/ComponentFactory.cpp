#include "include/Factories/ComponentFactory.h"
#include "include/GloomEngine.h"
#include "include/EngineManagers/RendererManager.h"
#include "include/EngineManagers/ColliderManager.h"
#include "include/GameObjectsAndPrefabs/GameObject.h"
#include "include/Components/Renderers/Renderer.h"
#include "include/Components/Renderers/Camera.h"
#include "include/Components/Renderers/Lights/PointLight.h"
#include "include/Components/Renderers/Lights/DirectionalLight.h"
#include "include/Components/Renderers/Lights/SpotLight.h"
#include "include/Components/PhysicsAndColliders/Rigidbody.h"
#include "include/Components/PhysicsAndColliders/BoxCollider.h"
#include "include/Components/Scripts/PlayerMovement.h"

ComponentFactory::ComponentFactory(const std::shared_ptr<GloomEngine> &gloomEngine) : gloomEngine(gloomEngine) {}

std::shared_ptr<Component> ComponentFactory::CreateComponent(std::string type, const std::shared_ptr<GameObject> &parent){
    if (type == typeid(Renderer).name()) {
        std::shared_ptr<Component> component = CreateRenderer(parent);
        return component;
    }
    else if (type == typeid(Camera).name()) {
        std::shared_ptr<Component> component = CreateCamera(parent);
        return component;
    }
    else if (type == typeid(PointLight).name()) {
        std::shared_ptr<Component> component = CreatePointLight(parent);
        return component;
    }
    else if (type == typeid(DirectionalLight).name()) {
        std::shared_ptr<Component> component = CreateDirectionalLight(parent);
        return component;
    }
    else if (type == typeid(SpotLight).name()) {
        std::shared_ptr<Component> component = CreateSpotLight(parent);
        return component;
    }
    else if (type == typeid(BoxCollider).name()) {
        std::shared_ptr<Component> component = CreateBoxCollider(parent);
        return component;
    }
    else if (type == typeid(Rigidbody).name()) {
        std::shared_ptr<Component> component = CreateRigidbody(parent);
        return component;
    }
    else if (type == typeid(PlayerMovement).name()) {
        std::shared_ptr<Component> component = CreatePlayerMovement(parent);
        return component;
    }
    return nullptr;
};

std::shared_ptr<Renderer> ComponentFactory::CreateRenderer(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<Renderer> component = parent->GetComponent<Renderer>();
    if (component != nullptr) return component;
    id++;
    component = std::make_shared<Renderer>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    return component;
}

std::shared_ptr<Camera> ComponentFactory::CreateCamera(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<Camera> component = parent->GetComponent<Camera>();
    if (component != nullptr) return component;
    id++;
    component = std::make_shared<Camera>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    return component;
}

std::shared_ptr<PointLight> ComponentFactory::CreatePointLight(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<PointLight> component = parent->GetComponent<PointLight>();
    if (component != nullptr) return component;
    id++;

    int number = 0;

    for (auto&& pointLight : gloomEngine->engineRenderer->pointLights) {
        if (pointLight.second == nullptr) {
            component = std::make_shared<PointLight>(gloomEngine, parent, id);
            std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
            gloomEngine->AddComponent(component);
            gloomEngine->engineRenderer->pointLights.find(number)->second = component;
            gloomEngine->engineRenderer->UpdateLight(id);
            id++;
            return component;
        }
        number++;
    }

    component = std::make_shared<PointLight>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    gloomEngine->engineRenderer->pointLights.insert({number, component});
    gloomEngine->engineRenderer->UpdateLight(id);
    return component;

}

std::shared_ptr<DirectionalLight> ComponentFactory::CreateDirectionalLight(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<DirectionalLight> component = parent->GetComponent<DirectionalLight>();
    if (component != nullptr) return component;
    id++;
    int number = 0;

    for (auto&& directionalLight : gloomEngine->engineRenderer->directionalLights){
        if (directionalLight.second == nullptr) {
            component = std::make_shared<DirectionalLight>(gloomEngine, parent, id);
            std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
            gloomEngine->AddComponent(component);
            gloomEngine->engineRenderer->directionalLights.find(number)->second = component;
            gloomEngine->engineRenderer->UpdateLight(id);
            id++;
            return component;
        }
        number++;
    }

    component = std::make_shared<DirectionalLight>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    gloomEngine->engineRenderer->directionalLights.insert({number, component});
    gloomEngine->engineRenderer->UpdateLight(id);
    return component;
}

std::shared_ptr<SpotLight> ComponentFactory::CreateSpotLight(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<SpotLight> component = parent->GetComponent<SpotLight>();
    if (component != nullptr) return component;
    id++;
    int number = 0;

    for (auto&& spotLight : gloomEngine->engineRenderer->spotLights){
        if (spotLight.second == nullptr) {
            component = std::make_shared<SpotLight>(gloomEngine, parent, id);
            std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
            gloomEngine->AddComponent(component);
            gloomEngine->engineRenderer->spotLights.find(number)->second = component;
            gloomEngine->engineRenderer->UpdateLight(id);
            return component;
        }
        number++;
    }
    component = std::make_shared<SpotLight>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    gloomEngine->engineRenderer->spotLights.insert({number, component});
    gloomEngine->engineRenderer->UpdateLight(id);
    return component;
}

std::shared_ptr<BoxCollider> ComponentFactory::CreateBoxCollider(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<BoxCollider> component = parent->GetComponent<BoxCollider>();
    if (component != nullptr) return component;
    id++;
    component = std::make_shared<BoxCollider>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->engineColliders->boxColliders.insert({id, component});
    gloomEngine->engineColliders->OnBoxColliderAdd();
    gloomEngine->AddComponent(component);
    return component;
}

std::shared_ptr<Rigidbody> ComponentFactory::CreateRigidbody(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<Rigidbody> component = parent->GetComponent<Rigidbody>();
    if (component != nullptr) return component;
    id++;
    component = std::make_shared<Rigidbody>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    return component;
}

std::shared_ptr<PlayerMovement> ComponentFactory::CreatePlayerMovement(const std::shared_ptr<GameObject> &parent) {
    std::shared_ptr<PlayerMovement> component = parent->GetComponent<PlayerMovement>();
    if (component != nullptr) return component;
    id++;
    component = std::make_shared<PlayerMovement>(gloomEngine, parent, id);
    std::shared_ptr<Component> parentComponent = std::dynamic_pointer_cast<Component>(component);
    gloomEngine->AddComponent(component);
    return component;
}
