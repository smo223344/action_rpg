#include "entity.h"
#include <algorithm>
#include <limits>
#include <glm/gtc/constants.hpp>

void MobEntity::update(float deltaTime) {
    if (isMoving) {
        glm::vec3 direction = targetPosition - position;
        float distance = glm::length(direction);

        if (distance > 0.1f) {
            direction = glm::normalize(direction);
            float moveDistance = movementSpeed * deltaTime;

            glm::vec3 desiredPosition;
            if (moveDistance >= distance) {
                desiredPosition = targetPosition;
                isMoving = false;
            } else {
                desiredPosition = position + direction * moveDistance;
            }

            // Check for collisions with other MOBs
            glm::vec3 finalPosition = desiredPosition;
            if (entityManager) {
                const auto& entities = entityManager->getEntities();
                for (const auto& other : entities) {
                    // Skip self
                    if (other.get() == this) continue;

                    // Only check collision with other MobEntities
                    auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
                    if (!otherMob || !otherMob->active) continue;

                    // Calculate distance between centers at desired position
                    glm::vec3 toOther = otherMob->position - desiredPosition;
                    float centerDistance = glm::length(toOther);
                    float minDistance = radius + otherMob->radius;

                    // If collision would occur, clamp position
                    if (centerDistance < minDistance) {
                        // Find the point along the movement path where circles just touch
                        // Move as close as possible without overlapping
                        if (centerDistance > 0.001f) {
                            glm::vec3 toOtherNorm = toOther / centerDistance;
                            // Position this entity just outside the other's radius
                            finalPosition = otherMob->position - toOtherNorm * minDistance;
                        } else {
                            // If centers are exactly on top of each other, push away in movement direction
                            finalPosition = otherMob->position - direction * minDistance;
                        }
                        isMoving = false; // Stop moving since we hit an obstacle
                        break; // Only handle first collision
                    }
                }
            }

            position = finalPosition;
        } else {
            position = targetPosition;
            isMoving = false;
        }
    }
}

void MobEntity::moveTo(const glm::vec3& target) {
    targetPosition = target;
    isMoving = true;
}

void MobEntity::stop() {
    isMoving = false;
}

void BasicShooterEnemy::update(float deltaTime) {
    // AI: Follow the closest player character
    if (party && !party->empty()) {
        // Find the closest PC
        std::shared_ptr<PlayerEntity> closestPC = nullptr;
        float closestDistance = std::numeric_limits<float>::max();

        for (const auto& pc : *party) {
            if (pc && pc->active) {
                float distance = glm::length(pc->position - position);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestPC = pc;
                }
            }
        }

        // Move towards the closest PC
        if (closestPC) {
            moveTo(closestPC->position);
        }
    }

    // Call parent update to handle movement
    MobEntity::update(deltaTime);
}

void EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    entities.push_back(entity);

    // Set entity manager reference for MobEntity types (for collision detection)
    if (auto mob = std::dynamic_pointer_cast<MobEntity>(entity)) {
        mob->entityManager = this;
    }
}

void EntityManager::removeEntity(std::shared_ptr<Entity> entity) {
    entities.erase(
        std::remove(entities.begin(), entities.end(), entity),
        entities.end()
    );
}

void EntityManager::updateAll(float deltaTime) {
    for (auto& entity : entities) {
        if (entity && entity->active) {
            entity->update(deltaTime);
        }
    }
}
