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

                    float minDistance = radius + otherMob->radius;

                    // Calculate current distance
                    glm::vec3 currentToOther = otherMob->position - position;
                    float currentDistance = glm::length(currentToOther);

                    // Calculate desired distance
                    glm::vec3 toOther = otherMob->position - desiredPosition;
                    float desiredDistance = glm::length(toOther);

                    // Only block movement if:
                    // 1. Desired position would overlap (desiredDistance < minDistance)
                    // 2. AND we're not moving away (desiredDistance <= currentDistance)
                    if (desiredDistance < minDistance && desiredDistance <= currentDistance) {
                        // Find the point where circles just touch
                        if (currentDistance > 0.001f) {
                            // Move along the line between centers to just touching
                            glm::vec3 currentToOtherNorm = currentToOther / currentDistance;
                            finalPosition = otherMob->position - currentToOtherNorm * minDistance;
                        } else {
                            // Centers are on top of each other, push away in movement direction
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

glm::vec3 MobEntity::calculateSteeringForce(const glm::vec3& targetPos, float avoidanceRadius) {
    glm::vec3 desiredDirection = targetPos - position;
    float distToTarget = glm::length(desiredDirection);

    if (distToTarget < 0.01f) {
        return glm::vec3(0.0f);
    }

    // Attraction toward target
    glm::vec3 seekForce = glm::normalize(desiredDirection);

    // Repulsion from nearby obstacles
    glm::vec3 avoidanceForce(0.0f);

    if (entityManager) {
        const auto& entities = entityManager->getEntities();
        for (const auto& other : entities) {
            if (other.get() == this) continue;

            auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
            if (!otherMob || !otherMob->active) continue;

            glm::vec3 toOther = otherMob->position - position;
            float distance = glm::length(toOther);

            // Only avoid obstacles within avoidance radius
            if (distance < avoidanceRadius && distance > 0.01f) {
                // Repulsion strength increases as we get closer
                float repulsionStrength = (avoidanceRadius - distance) / avoidanceRadius;
                glm::vec3 awayFromOther = -glm::normalize(toOther);
                avoidanceForce += awayFromOther * repulsionStrength;
            }
        }
    }

    // Combine forces (avoidance gets higher priority when close to obstacles)
    glm::vec3 combinedForce = seekForce + avoidanceForce * 2.0f;

    if (glm::length(combinedForce) > 0.01f) {
        return glm::normalize(combinedForce);
    }

    return seekForce;
}

void BasicShooterEnemy::update(float deltaTime) {
    // AI: Follow the closest player character using steering behaviors
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

        // Use steering behaviors to move toward the closest PC
        if (closestPC) {
            glm::vec3 targetPos = closestPC->position;

            // Stop when we're close enough (our radius + their radius + small gap)
            float stopDistance = radius + closestPC->radius + 0.2f;
            if (closestDistance > stopDistance) {
                // Calculate steering direction with obstacle avoidance
                glm::vec3 steeringDir = calculateSteeringForce(targetPos, 3.0f);

                // Move in the steering direction
                glm::vec3 nextPos = position + steeringDir * movementSpeed * deltaTime;
                moveTo(nextPos);
            } else {
                // We're close enough, stop moving
                stop();
            }
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
