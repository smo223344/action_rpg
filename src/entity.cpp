#include "entity.h"
#include <algorithm>
#include <limits>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>

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

            // Apply collision resolution with sliding
            glm::vec3 finalPosition = resolveCollisions(desiredPosition, deltaTime);
            position = finalPosition;

            // Check if we've reached close enough to target after collision resolution
            if (glm::length(targetPosition - position) < 0.1f) {
                position = targetPosition;
                isMoving = false;
            }
        } else {
            position = targetPosition;
            isMoving = false;
        }
    }

    // Apply continuous separation forces even when not explicitly moving
    applySeparationForces(deltaTime);
}

void MobEntity::moveTo(const glm::vec3& target) {
    targetPosition = target;
    isMoving = true;
}

void MobEntity::stop() {
    isMoving = false;
}

glm::vec3 MobEntity::resolveCollisions(const glm::vec3& desiredPosition, float deltaTime) {
    if (!entityManager) return desiredPosition;

    glm::vec3 movement = desiredPosition - position;
    glm::vec3 finalPosition = desiredPosition;

    // Collect all overlapping entities
    std::vector<std::pair<MobEntity*, float>> collisions;
    const auto& entities = entityManager->getEntities();

    for (const auto& other : entities) {
        if (other.get() == this) continue;

        auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
        if (!otherMob || !otherMob->active) continue;

        glm::vec3 toOther = otherMob->position - desiredPosition;
        float distance = glm::length(toOther);
        float minDistance = radius + otherMob->radius;

        if (distance < minDistance) {
            collisions.push_back({otherMob.get(), distance});
        }
    }

    // Process collisions with sliding
    for (const auto& [otherMob, distance] : collisions) {
        glm::vec3 toOther = otherMob->position - finalPosition;
        float currentDist = glm::length(toOther);
        float minDistance = radius + otherMob->radius;

        if (currentDist < minDistance && currentDist > 0.001f) {
            // Calculate penetration depth
            float penetration = minDistance - currentDist;
            glm::vec3 separationDir = -glm::normalize(toOther);

            // Instead of stopping, slide along the collision surface
            // Project movement onto the plane tangent to collision
            glm::vec3 collisionNormal = -toOther / currentDist;
            glm::vec3 slideDirection = movement - collisionNormal * glm::dot(movement, collisionNormal);

            // Apply sliding with some friction
            float slideFactor = 0.7f; // Friction coefficient (0 = full stop, 1 = perfect slide)
            finalPosition = position + slideDirection * slideFactor;

            // Ensure we're not still penetrating after slide
            glm::vec3 afterSlideToOther = otherMob->position - finalPosition;
            float afterSlideDist = glm::length(afterSlideToOther);
            if (afterSlideDist < minDistance && afterSlideDist > 0.001f) {
                // Push out to minimum distance
                finalPosition = otherMob->position - glm::normalize(afterSlideToOther) * minDistance;
            }
        }
    }

    return finalPosition;
}

void MobEntity::applySeparationForces(float deltaTime) {
    if (!entityManager) return;

    glm::vec3 separationForce(0.0f);
    int nearbyCount = 0;

    const auto& entities = entityManager->getEntities();
    for (const auto& other : entities) {
        if (other.get() == this) continue;

        auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
        if (!otherMob || !otherMob->active) continue;

        glm::vec3 toOther = position - otherMob->position;
        float distance = glm::length(toOther);
        float preferredDistance = (radius + otherMob->radius) * 1.2f; // Add some buffer

        // Apply gentle separation force when entities are too close
        if (distance < preferredDistance && distance > 0.001f) {
            float strength = (preferredDistance - distance) / preferredDistance;
            separationForce += (toOther / distance) * strength;
            nearbyCount++;
        }
    }

    // Apply the averaged separation force
    if (nearbyCount > 0) {
        separationForce /= static_cast<float>(nearbyCount);
        float separationSpeed = 2.0f; // Gentle push speed
        position += separationForce * separationSpeed * deltaTime;
    }
}

glm::vec3 MobEntity::calculateSteeringForce(const glm::vec3& targetPos, float avoidanceRadius) {
    glm::vec3 desiredDirection = targetPos - position;
    float distToTarget = glm::length(desiredDirection);

    if (distToTarget < 0.01f) {
        return glm::vec3(0.0f);
    }

    // Attraction toward target
    glm::vec3 seekForce = glm::normalize(desiredDirection);

    // Predictive avoidance - look ahead to where we'll be
    glm::vec3 futurePos = position + seekForce * movementSpeed * 0.5f; // Look 0.5 seconds ahead

    // Avoidance from nearby obstacles with perpendicular steering
    glm::vec3 avoidanceForce(0.0f);
    bool needsAvoidance = false;

    if (entityManager) {
        const auto& entities = entityManager->getEntities();
        for (const auto& other : entities) {
            if (other.get() == this) continue;

            auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
            if (!otherMob || !otherMob->active) continue;

            // Check both current and future positions
            glm::vec3 toOtherFuture = otherMob->position - futurePos;
            float futureDist = glm::length(toOtherFuture);

            glm::vec3 toOtherCurrent = otherMob->position - position;
            float currentDist = glm::length(toOtherCurrent);

            // Determine if we need to avoid this obstacle
            float effectiveRadius = radius + otherMob->radius + 0.3f; // Add buffer

            if (futureDist < effectiveRadius || currentDist < avoidanceRadius) {
                needsAvoidance = true;

                if (currentDist > 0.01f) {
                    // Calculate perpendicular avoidance direction
                    glm::vec3 toObstacle = glm::normalize(toOtherCurrent);

                    // Get perpendicular direction (left or right based on relative positions)
                    glm::vec3 perpendicular;
                    glm::vec3 cross = glm::cross(toObstacle, glm::vec3(0, 1, 0));
                    if (glm::length(cross) > 0.01f) {
                        perpendicular = glm::normalize(cross);
                    } else {
                        // Fallback if obstacle is directly above/below
                        perpendicular = glm::vec3(1, 0, 0);
                    }

                    // Choose direction based on which side has more room
                    glm::vec3 leftCheck = position + perpendicular * effectiveRadius;
                    glm::vec3 rightCheck = position - perpendicular * effectiveRadius;

                    float leftClearance = glm::length(otherMob->position - leftCheck);
                    float rightClearance = glm::length(otherMob->position - rightCheck);

                    if (rightClearance > leftClearance) {
                        perpendicular = -perpendicular;
                    }

                    // Stronger avoidance for closer obstacles
                    float avoidanceStrength = 1.0f - (currentDist / avoidanceRadius);
                    avoidanceStrength = glm::clamp(avoidanceStrength, 0.0f, 1.0f);

                    // Combine perpendicular steering with slight push away
                    avoidanceForce += (perpendicular * 0.8f - toObstacle * 0.2f) * avoidanceStrength;
                }
            }
        }
    }

    // Combine forces with dynamic weighting
    glm::vec3 combinedForce;
    if (needsAvoidance) {
        // When avoiding, reduce seek force and increase avoidance
        float avoidanceWeight = glm::clamp(glm::length(avoidanceForce), 0.0f, 3.0f);
        combinedForce = seekForce * 0.3f + avoidanceForce * avoidanceWeight;
    } else {
        // No obstacles, full speed toward target
        combinedForce = seekForce;
    }

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

            // Desired engagement distance (stop a bit away from the player)
            float desiredDistance = radius + closestPC->radius + 1.0f; // Keep some combat distance

            if (closestDistance > desiredDistance) {
                // Calculate steering direction with dynamic avoidance radius
                float avoidanceRadius = glm::max(3.0f, movementSpeed * 0.8f); // Scale with speed
                glm::vec3 steeringDir = calculateSteeringForce(targetPos, avoidanceRadius);

                // Smoother movement using steering direction
                glm::vec3 nextPos = position + steeringDir * movementSpeed * deltaTime;
                moveTo(nextPos);
            } else if (closestDistance < desiredDistance * 0.7f) {
                // Too close, back away slightly
                glm::vec3 awayDir = glm::normalize(position - targetPos);
                glm::vec3 backPos = position + awayDir * movementSpeed * 0.5f * deltaTime;
                moveTo(backPos);
            } else {
                // We're at a good distance, stop moving but face the target
                stop();
                // Could add rotation to face target here if needed
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
