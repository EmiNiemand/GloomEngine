#include "EngineManagers/ColliderManager.h"
#include "GloomEngine.h"
#include "EngineManagers/RendererManager.h"
#include "LowLevelClasses/Shader.h"
#include "GameObjectsAndPrefabs/GameObject.h"
#include "Components/Renderers/Camera.h"
#include "Components/PhysicsAndColliders/BoxCollider.h"
#include "Components/PhysicsAndColliders/Rigidbody.h"

ColliderManager::ColliderManager() {
    colliderDebugShader = std::make_shared<Shader>("colliderDebug.vert", "colliderDebug.frag");

    // create buffers/arrays
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
}

ColliderManager::~ColliderManager() = default;

ColliderManager* ColliderManager::GetInstance() {
    if (colliderManager == nullptr) {
        colliderManager = new ColliderManager();
    }
    return colliderManager;
}


void ColliderManager::ManageCollision() {
    if (boxColliders.empty()) return;

    if (boxColliders.size() > 1) {
        // Handle collision
        for (auto&& box : boxColliders) {
            if (box.second->GetParent()->GetComponent<Rigidbody>() != nullptr) {
                for(auto&& box2 : boxColliders) {
                    if (box2.second == box.second) continue;
                    box.second->CheckCollision(box2.second);
                }
            }
        }
    }
}

void ColliderManager::DrawColliders() {
    colliderDebugShader->Activate();
    colliderDebugShader->SetVec3("color", debugColor);
    colliderDebugShader->SetMat4("projection", RendererManager::GetInstance()->projection);
    colliderDebugShader->SetMat4("view", Camera::activeCamera->GetComponent<Camera>()->GetViewMatrix());
    for (auto&& box : boxColliders) {
        colliderDebugShader->SetMat4("model", box.second->GetModelMatrix());
        glBindVertexArray(vao);
        glDrawElements(GL_LINES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void ColliderManager::Free() {
    colliderDebugShader->Delete();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void ColliderManager::RemoveBoxCollider(int componentId) {
    if (boxColliders.contains(componentId)) boxColliders.erase(componentId);
}

void ColliderManager::OnBoxCollidersChange() {
    vertices.clear();
    indices.clear();
    int i = 0;
    for (auto&& col : boxColliders) {
        for (auto&& point : col.second->GetBoxPoints()) {
            vertices.push_back(point);
        }
        // UP LEFT
        indices.push_back(i * 8 + 0);
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 2);

        // UP RIGHT
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 3);
        indices.push_back(i * 8 + 2);

        // FRONT LEFT
        indices.push_back(i * 8 + 0);
        indices.push_back(i * 8 + 6);
        indices.push_back(i * 8 + 4);

        // FRONT RIGHT
        indices.push_back(i * 8 + 0);
        indices.push_back(i * 8 + 2);
        indices.push_back(i * 8 + 6);

        // LEFT BACK
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 4);
        indices.push_back(i * 8 + 5);

        // LEFT FRONT
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 0);
        indices.push_back(i * 8 + 4);

        // RIGHT BACK
        indices.push_back(i * 8 + 3);
        indices.push_back(i * 8 + 7);
        indices.push_back(i * 8 + 6);

        // RIGHT FRONT
        indices.push_back(i * 8 + 3);
        indices.push_back(i * 8 + 6);
        indices.push_back(i * 8 + 2);

        // DOWN BACK
        indices.push_back(i * 8 + 4);
        indices.push_back(i * 8 + 5);
        indices.push_back(i * 8 + 7);

        // DOWN FRONT
        indices.push_back(i * 8 + 4);
        indices.push_back(i * 8 + 7);
        indices.push_back(i * 8 + 6);

        // BACK DOWN
        indices.push_back(i * 8 + 5);
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 7);


        // BACK UP
        indices.push_back(i * 8 + 1);
        indices.push_back(i * 8 + 3);
        indices.push_back(i * 8 + 7);

        i++;
    }

    if (vertices.empty() || indices.empty()) return;
    glBindVertexArray(vao);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}
