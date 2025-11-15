#include "entity.h"
#include <algorithm>
#include <glm/gtc/constants.hpp>

void MobEntity::update(float deltaTime) {
    if (isMoving) {
        glm::vec3 direction = targetPosition - position;
        float distance = glm::length(direction);

        if (distance > 0.1f) {
            direction = glm::normalize(direction);
            float moveDistance = movementSpeed * deltaTime;

            if (moveDistance >= distance) {
                position = targetPosition;
                isMoving = false;
            } else {
                position += direction * moveDistance;
            }
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

void EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    entities.push_back(entity);
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
