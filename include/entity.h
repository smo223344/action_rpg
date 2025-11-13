#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

// Base renderable entity
struct Entity {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    bool active{true};

    virtual ~Entity() = default;
    virtual void update(float deltaTime) {}
};

// Player entity with stats (placeholders for now)
struct PlayerEntity : public Entity {
    // Stats
    float health{100.0f};
    float maxHealth{100.0f};
    float energy{100.0f};
    float maxEnergy{100.0f};
    float movementSpeed{5.0f};
    float attackSpeed{1.0f};

    // Movement
    glm::vec3 targetPosition{0.0f, 0.0f, 0.0f};
    bool isMoving{false};

    void update(float deltaTime) override;
    void moveTo(const glm::vec3& target);
};

// Entity manager to hold all renderable entities
class EntityManager {
public:
    EntityManager() = default;

    void addEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);
    void updateAll(float deltaTime);

    const std::vector<std::shared_ptr<Entity>>& getEntities() const { return entities; }

private:
    std::vector<std::shared_ptr<Entity>> entities;
};
