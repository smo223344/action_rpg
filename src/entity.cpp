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

bool MobEntity::isPointBlocked(const glm::vec3& point, float minClearance) const {
    if (!entityManager) return false;

    const auto& entities = entityManager->getEntities();
    for (const auto& other : entities) {
        // Skip self
        if (other.get() == this) continue;

        // Only check collision with other MobEntities
        auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
        if (!otherMob || !otherMob->active) continue;

        // Check if point is inside the other MOB's radius plus clearance
        float distance = glm::length(otherMob->position - point);
        if (distance < otherMob->radius + minClearance) {
            return true;
        }
    }
    return false;
}

glm::vec3 MobEntity::findIntermediateWaypoint(const glm::vec3& target) {
    // If we're targeting a MOB, account for its radius
    float targetMobRadius = 0.0f;
    if (entityManager) {
        const auto& entities = entityManager->getEntities();
        for (const auto& other : entities) {
            if (other.get() == this) continue;
            auto otherMob = std::dynamic_pointer_cast<MobEntity>(other);
            if (otherMob && otherMob->active) {
                float distToTarget = glm::length(otherMob->position - target);
                if (distToTarget < 0.1f) { // Target is at this MOB's position
                    targetMobRadius = otherMob->radius;
                    break;
                }
            }
        }
    }

    float currentRadius = searchRadius;
    const int numSamples = 16; // Sample points around the circle
    const float radiusShrinkFactor = 0.7f;
    const float minRadiusBeforeBackward = radius * 1.5f;
    bool allowBackward = false;

    glm::vec3 toTarget = target - position;
    float distanceToTarget = glm::length(toTarget);

    // If we're already at the target (accounting for MOB radius), return current position
    if (distanceToTarget < radius + targetMobRadius + 0.1f) {
        return position;
    }

    glm::vec3 targetDir = glm::normalize(toTarget);

    while (currentRadius >= radius) {
        glm::vec3 bestPoint = position;
        float bestScore = std::numeric_limits<float>::max();
        bool foundValid = false;

        // Sample points on the circle
        for (int i = 0; i < numSamples; i++) {
            float angle = (2.0f * glm::pi<float>() * i) / numSamples;

            // Create a perpendicular basis for the circle
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(targetDir, up));
            glm::vec3 forward = glm::normalize(glm::cross(right, up));

            // Sample point on circle
            glm::vec3 samplePoint = position +
                (right * std::cos(angle) + forward * std::sin(angle)) * currentRadius;
            samplePoint.y = 0.0f; // Keep on ground plane

            // Check if this point is blocked
            if (!isPointBlocked(samplePoint, radius * 0.5f)) {
                // Calculate how close this point is to the target
                float distToTarget = glm::length(target - samplePoint);

                // If not allowing backward, check if we're making progress
                glm::vec3 toSample = samplePoint - position;
                float dotProduct = glm::dot(glm::normalize(toSample), targetDir);

                if (allowBackward || dotProduct > 0.0f) {
                    // Score is distance to target (lower is better)
                    if (distToTarget < bestScore) {
                        bestScore = distToTarget;
                        bestPoint = samplePoint;
                        foundValid = true;
                    }
                }
            }
        }

        if (foundValid) {
            return bestPoint;
        }

        // Shrink radius and try again
        currentRadius *= radiusShrinkFactor;

        // If we've shrunk to collision radius, start over allowing backward movement
        if (currentRadius < minRadiusBeforeBackward && !allowBackward) {
            allowBackward = true;
            currentRadius = searchRadius;
        }
    }

    // No valid waypoint found, just move directly toward target
    return position + targetDir * radius;
}

void BasicShooterEnemy::update(float deltaTime) {
    // AI: Follow the closest player character using pathfinding
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

        // Use pathfinding to move toward the closest PC
        if (closestPC) {
            // Find intermediate waypoint that avoids obstacles
            glm::vec3 targetPos = closestPC->position;

            // Stop when we're close enough (our radius + their radius + small gap)
            float stopDistance = radius + closestPC->radius + 0.2f;
            if (closestDistance > stopDistance) {
                glm::vec3 waypointPos = findIntermediateWaypoint(targetPos);
                moveTo(waypointPos);
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
