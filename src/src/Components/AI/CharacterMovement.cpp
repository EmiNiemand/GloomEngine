//
// Created by Adrian on 01.05.2023.
//

#include "GameObjectsAndPrefabs/GameObject.h"
#include "EngineManagers/AIManager.h"
#include "EngineManagers/RandomnessManager.h"
#include "EngineManagers/CollisionManager.h"
#include "Components/AI/CharacterMovement.h"
#include "Components/PhysicsAndColliders/Rigidbody.h"
#include "Components/PhysicsAndColliders/BoxCollider.h"

#ifdef DEBUG
#include <tracy/Tracy.hpp>
#endif

CharacterMovement::CharacterMovement(const std::shared_ptr<GameObject> &parent, int id) : Component(parent, id) { }

CharacterMovement::~CharacterMovement() = default;

void CharacterMovement::Start() {
    playerTransform = GloomEngine::GetInstance()->FindGameObjectWithName("Player")->transform;
    playerRigidbody = GloomEngine::GetInstance()->FindGameObjectWithName("Player")->GetComponent<Rigidbody>();
    rigidbody = parent->GetComponent<Rigidbody>();
    collisionGrid = CollisionManager::GetInstance()->grid;
    collisionGridSize = CollisionManager::GetInstance()->gridSize;
    aiGrid = AIManager::GetInstance()->aiGrid;
    pathfinding = AIManager::GetInstance()->pathfinding;

    glm::vec3 coords;
    coords = AIManager::GetInstance()->minSpawnCoords;
    minSpawnCoords = {coords.x, coords.z};
    coords = AIManager::GetInstance()->maxSpawnCoords;
    maxSpawnCoords = {coords.x, coords.z};

    if (AIManager::GetInstance()->isInitializing)
        SetRandomSpawnPointNearPlayer();
    else
        SetRandomSpawnPoint();

    AIManager::GetInstance()->tempCharacters.erase(id);
    Component::Start();
}

void CharacterMovement::FixedUpdate() {
#ifdef DEBUG
    ZoneScopedNC("CharacterMovement", 0xfc0f03);
#endif

    currentPosition = parent->transform->GetLocalPosition();
    currentPositionForward = currentPosition + parent->transform->GetForward() * 2.0f;
    playerPosition = playerTransform->GetLocalPosition();
    playerVelocity = glm::length(glm::vec2(playerRigidbody->velocity.x, playerRigidbody->velocity.z));

    if (glm::distance(playerPosition, currentPositionForward) < DISTANCE_TO_PLAYER && playerVelocity > 0.01f) {
        if (movementState != Waiting) {
            previousMovementState = movementState;
            movementState = Waiting;
        }
    } else {
        if (previousMovementState != Waiting) {
            movementState = previousMovementState;
            previousMovementState = Waiting;
        }

        if (pathIterator >= 0) {
            cellPos = glm::ivec2((int) (currentPosition.x / collisionGridSize) + GRID_SIZE / 2,
                                 (int) (currentPosition.z / collisionGridSize) + GRID_SIZE / 2);

            cellPtr = &collisionGrid[cellPos.x + cellPos.y * GRID_SIZE];

            speed = std::lerp(speed, MOVEMENT_MAX_SPEED, MOVEMENT_SMOOTHING_PARAM);

            steeringForce = glm::normalize((*path)[pathIterator] - currentPosition);

            if (!cellPtr->empty()) {
                for (const auto &box: *cellPtr) {
                    if (box.first == parent->GetComponent<BoxCollider>()->GetId())
                        continue;

                    if (box.second->isDynamic) {
                        steeringPosition = box.second->GetParent()->transform->GetGlobalPosition();
                        distance = glm::distance(currentPosition, steeringPosition);

                        if (distance < maxDistanceToCharacter) {
                            maxDistanceToCharacter = distance;
                            steeringDirection = glm::normalize(currentPosition - steeringPosition);
                        }
                    }
                }

                if (maxDistanceToCharacter < DISTANCE_TO_COLLISION) {
                    rotationAngle = std::acos(glm::dot(steeringForce, steeringDirection)) / AVOIDANCE_ROTATION_FACTOR;

                    if (rotationAngle > 0.45f) {
                        steeringMatrix = glm::rotate(glm::mat4(1), rotationAngle, glm::vec3(0, 1, 0));
                        steeringForce = steeringMatrix * glm::vec4(steeringDirection, 1) * AVOIDANCE_FORCE_MODIFIER;
                    }
                }

                maxDistanceToCharacter = FLT_MAX;
            }

            ApplyForces(steeringForce);

            distance = glm::distance(currentPosition, (*path)[pathIterator]);

            if (distance < DISTANCE_TO_POINT) {
                --pathIterator;
                timeSinceLastPoint = 0.0f;
            }
        }

        if (movementState == NearPlayerPosition || movementState == NearDuelPosition) {
            steeringForce = glm::normalize(playerPosition - currentPosition);

            ApplyRotation(steeringForce);
        } else {
            timeSinceLastPoint += GloomEngine::GetInstance()->fixedDeltaTime;

            if (timeSinceLastPoint > MOVEMENT_TIMEOUT) {
                timeSinceLastPoint = 0.0f;
                pathIterator = -1;

                if (movementState == OnPathToPlayer)
                    movementState = NearPlayerPosition;
                else if (movementState == OnPathToDuel)
                    movementState = NearDuelPosition;
                else
                    movementState = NearTargetPosition;
            }
        }
    }

    Component::FixedUpdate();
}

void CharacterMovement::AIUpdate() {
    switch (movementState) {
        case NearTargetPosition:
            SetRandomEndPoint();
            break;
        case OnPathToTarget:
            if (pathIterator < 0)
                movementState = NearTargetPosition;
            break;
        case ReturningToPreviousTarget:
            isStatic = false;
            parent->GetComponent<BoxCollider>()->ChangeAIGridPoints(isStatic);
            ReturnToPreviousPath();
            break;
        case OnPathToDuel:
            if (pathIterator < 0)
                movementState = NearDuelPosition;
            break;
        case SettingPathToDuel:
            SetNewPathToDuel();
            break;
        case NearDuelPosition:
            if (!isStatic) {
                isStatic = true;
                parent->GetComponent<BoxCollider>()->ChangeAIGridPoints(isStatic);
            }
            break;
        case SettingPathToPlayer:
            SetNewPathToPlayer();
            break;
        case OnPathToPlayer:
            if (pathIterator < 0)
                movementState = NearPlayerPosition;
            break;
        case NearPlayerPosition:
            if (!isStatic) {
                isStatic = true;
                parent->GetComponent<BoxCollider>()->ChangeAIGridPoints(isStatic);
            }
            break;
        default:
            break;
    }

    Component::AIUpdate();
}

void CharacterMovement::OnCreate() {
    AIManager::GetInstance()->charactersMovements.insert({id, std::dynamic_pointer_cast<CharacterMovement>(shared_from_this())});
    AIManager::GetInstance()->tempCharacters.insert({id, std::dynamic_pointer_cast<CharacterMovement>(shared_from_this())});
    Component::OnCreate();
}

void CharacterMovement::OnDestroy() {
    AIManager::GetInstance()->charactersMovements.erase(id);
    if (path != nullptr) {
        path->clear();
        delete path;
    }
    rigidbody = nullptr;
    pathfinding = nullptr;
    collisionGrid = nullptr;
    playerTransform = nullptr;
    playerRigidbody = nullptr;
    Component::OnDestroy();
}

/**
 * @annotation
 * Applies all forces based on force value.
 * @param velocity - force
 */
inline void CharacterMovement::ApplyForces(const glm::vec3& force) {
    rigidbody->AddForce(force * speed * speedMultiplier, ForceMode::Force);

    ApplyRotation(force);
}

/**
 * @annotation
 * Applies rotation based on force value.
 * @param velocity - force
 */
inline void CharacterMovement::ApplyRotation(const glm::vec3& force) {
    rotationAngle = std::atan2f(-force.x, -force.z) * 180.0f / std::numbers::pi_v<float>;

    rigidbody->AddTorque(rotationAngle, ForceMode::Force);
}

/**
 * @annotation
 * Sets random spawn point on AI grid near player position.
 */
void CharacterMovement::SetRandomSpawnPointNearPlayer() {
    int minX, maxX, minY, maxY;
    playerPosition = playerTransform->GetLocalPosition();
    glm::ivec2 newEndPoint = {playerPosition.x, playerPosition.z};

    minX = std::clamp(newEndPoint.x - AI_SPAWN_PLAYER_DISTANCE, minSpawnCoords.x, maxSpawnCoords.x);
    maxX = std::clamp(newEndPoint.x + AI_SPAWN_PLAYER_DISTANCE, minSpawnCoords.x, maxSpawnCoords.x);
    minY = std::clamp(newEndPoint.y - AI_SPAWN_PLAYER_DISTANCE, minSpawnCoords.y, maxSpawnCoords.y);
    maxY = std::clamp(newEndPoint.y + AI_SPAWN_PLAYER_DISTANCE, minSpawnCoords.y, maxSpawnCoords.y);

    while (true) {
        newEndPoint.x = RandomnessManager::GetInstance()->GetInt(minX, maxX);
        newEndPoint.y = RandomnessManager::GetInstance()->GetInt(minY, maxY);

        if (!aiGrid[(newEndPoint.x + AI_GRID_SIZE / 2) + (newEndPoint.y + AI_GRID_SIZE / 2) * AI_GRID_SIZE])
            if (IsSpawnPointAvailable(newEndPoint))
                break;
    }

    currentPosition = {newEndPoint.x, 0.01f, newEndPoint.y};
    parent->transform->SetLocalPosition(currentPosition);
}

/**
 * @annotation
 * Sets random spawn point on AI grid.
 */
void CharacterMovement::SetRandomSpawnPoint() {
    int minX, maxX, minY, maxY;
    playerPosition = playerTransform->GetLocalPosition();
    glm::ivec2 newEndPoint = {playerPosition.x, playerPosition.z};

    minX = newEndPoint.x - AI_SPAWN_X_MIN_DISTANCE;
    maxX = newEndPoint.x + AI_SPAWN_X_MAX_DISTANCE;
    minY = newEndPoint.y - AI_SPAWN_Y_MIN_DISTANCE;
    maxY = newEndPoint.y + AI_SPAWN_Y_MAX_DISTANCE;

    while (true) {
        while (true) {
            newEndPoint.x = RandomnessManager::GetInstance()->GetInt(minSpawnCoords.x, maxSpawnCoords.x);

            if (newEndPoint.x <= minX || newEndPoint.x >= maxX)
                break;
        }

        while (true) {
            newEndPoint.y = RandomnessManager::GetInstance()->GetInt(minSpawnCoords.y, maxSpawnCoords.y);

            if (newEndPoint.y <= minY|| newEndPoint.y >= maxY)
                break;
        }

        if (!aiGrid[(newEndPoint.x + AI_GRID_SIZE / 2) + (newEndPoint.y + AI_GRID_SIZE / 2) * AI_GRID_SIZE])
            break;
    }

    currentPosition = {newEndPoint.x, 0.01f, newEndPoint.y};
    parent->transform->SetLocalPosition(currentPosition);
}

/**
 * @annotation
 * Sets new random point as the end point on AI grid.
 */
void CharacterMovement::SetRandomEndPoint() {
    int minX, maxX, minY, maxY;
    currentPosition = parent->transform->GetLocalPosition();
    glm::ivec2 newEndPoint = {currentPosition.x, currentPosition.z};

    minX = std::clamp(newEndPoint.x - AI_POINT_DISTANCE, minSpawnCoords.x, maxSpawnCoords.x);
    maxX = std::clamp(newEndPoint.x + AI_POINT_DISTANCE, minSpawnCoords.x, maxSpawnCoords.x);
    minY = std::clamp(newEndPoint.y - AI_POINT_DISTANCE, minSpawnCoords.y, maxSpawnCoords.y);
    maxY = std::clamp(newEndPoint.y + AI_POINT_DISTANCE, minSpawnCoords.y, maxSpawnCoords.y);

    while (true) {
        newEndPoint.x = RandomnessManager::GetInstance()->GetInt(minX, maxX);
        newEndPoint.y = RandomnessManager::GetInstance()->GetInt(minY, maxY);

        if (!aiGrid[(newEndPoint.x + AI_GRID_SIZE / 2) + (newEndPoint.y + AI_GRID_SIZE / 2) * AI_GRID_SIZE])
            break;
    }

    speed = 0.0f;
    movementState = OnPathToTarget;
    endPoint = {newEndPoint.x, newEndPoint.y};
    CalculatePath(endPoint);
}

/**
 * @annotation
 * Sets new end point near player position.
 */
void CharacterMovement::SetNewPathToPlayer() {
    speedMultiplier = 2.0f;
    previousEndPoint = endPoint;
    playerPosition = AIManager::GetInstance()->sessionPositions[id];
    endPoint = {playerPosition.x, playerPosition.z};
    movementState = OnPathToPlayer;

    CalculatePath(endPoint);
}

/**
 * @annotation
 * Sets new end point near player position (when dueling).
 */
void CharacterMovement::SetNewPathToDuel() {
    speedMultiplier = 2.0f;
    previousEndPoint = endPoint;
    playerPosition = AIManager::GetInstance()->sessionPositions[id];
    endPoint = {playerPosition.x, playerPosition.z};
    movementState = OnPathToDuel;

    CalculatePath(endPoint);
}

/**
 * @annotation
 * Sets previous end point as the current one.
 */
void CharacterMovement::ReturnToPreviousPath() {
    speedMultiplier = 1.0f;
    movementState = OnPathToTarget;
    CalculatePath(previousEndPoint);
}

/**
 * @annotation
 * Calculates path using Pathfinding component.
 */
void CharacterMovement::CalculatePath(const glm::ivec2& toPoint) {
#ifdef DEBUG
    ZoneScopedNC("CalculatePath", 0xfc0f03);
#endif

    if (path != nullptr) {
        path->clear();
        delete path;
    }

    path = pathfinding->FindNewPath({currentPosition.x, currentPosition.z},toPoint);

    if (path == nullptr) {
        movementState = NearTargetPosition;
        --nullPathCounter;
        if (nullPathCounter == 0)
            movementState = Stuck;
    } else {
        nullPathCounter = 3;
        pathIterator = (int) path->size() - 1;
    }
}

/**
 * @annotation
 * Sets new movement state.
 * @param newState - new state to set
 */
void CharacterMovement::SetState(const AI_MOVEMENT_STATE& newState) {
    movementState = newState;
}

/**
 * @annotation
 * Returns current movement state.
 * @returns AI_MOVEMENTSTATE - current state
 */
const AI_MOVEMENT_STATE CharacterMovement::GetState() const {
    return movementState;
}

/**
 * @annotation
 * Returns current spawn point.
 * @returns glm::ivec2 - currentPosition
 */
const glm::ivec2 CharacterMovement::GetSpawnPoint() const {
    return {currentPosition.x, currentPosition.z};
}

/**
 * @annotation
 * Returns current position.
 * @returns glm::vec3 - currentPosition
 */
const glm::vec3 CharacterMovement::GetCurrentPosition() const {
    return currentPosition;
}

/**
 * @annotation
 * Returns distance to player.
 * @returns float - distance to player
 */
const float CharacterMovement::GetDistanceToPlayer() const {
    return glm::distance(currentPosition, playerPosition);
}

/**
 * @annotation
 * Checks whether this spawn point is already chosen by another CharacterMovement object.
 * @param position - position to check
 * @returns bool - false if not available, otherwise true
 */
const bool CharacterMovement::IsSpawnPointAvailable(const glm::ivec2& position) {
    bool isAvailable = true;

    for (const auto& mov : AIManager::GetInstance()->tempCharacters) {
        if (position == mov.second->GetSpawnPoint() && mov.first != id) {
            isAvailable = false;
            break;
        }
    }

    return isAvailable;
}
