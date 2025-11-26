#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

enum class EntityState {
    Idle,
    Moving,
    Attacking,
    Casting,
    Drinking,
    Dead
};

// Base renderable entity
struct Entity {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    bool active{true};

    double stateTimeRemaining{0.0};
    EntityState actionState{EntityState::Idle};

    virtual ~Entity() = default;
    virtual void update(float deltaTime) {}
};

// Forward declaration
class EntityManager;

// MOB entity with stats (placeholders for now)
struct MobEntity : public Entity {
    // Stats
    float health{100.0f};
    float maxHealth{100.0f};
    float energy{100.0f};
    float maxEnergy{100.0f};
    float movementSpeed{5.0f};
    float attackSpeed{1.0f};
    float radius{0.5f}; // Collision radius

    // Movement
    glm::vec3 targetPosition{0.0f, 0.0f, 0.0f};
    bool isMoving{false};

    // Reference to entity manager for collision detection
    EntityManager* entityManager{nullptr};

    void update(float deltaTime) override;
    void moveTo(const glm::vec3& target);
    void stop();

    // Steering behavior for obstacle avoidance
    glm::vec3 calculateSteeringForce(const glm::vec3& targetPos, float avoidanceRadius);

    // Collision resolution with sliding
    glm::vec3 resolveCollisions(const glm::vec3& desiredPosition, float deltaTime);

    // Apply separation forces to prevent overlapping
    void applySeparationForces(float deltaTime);
};

// Player-controlled entity
struct PlayerEntity : public MobEntity {
    PlayerEntity() = default;
    ~PlayerEntity() override = default;
};

// Base enemy entity
struct EnemyEntity : public MobEntity {
    EnemyEntity() = default;
    ~EnemyEntity() override = default;
};

// Basic shooter enemy that follows the closest PC
struct BasicShooterEnemy : public EnemyEntity {
    BasicShooterEnemy() = default;
    ~BasicShooterEnemy() override = default;

    void update(float deltaTime) override;

    // Reference to party for AI targeting
    const std::vector<std::shared_ptr<PlayerEntity>>* party{nullptr};
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
