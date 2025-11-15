#pragma once

#include "renderer.h"
#include "entity.h"
#include "input.h"
#include <memory>
#include <vector>

class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void run();
    void shutdown();

private:
    void update(float deltaTime);
    void render();
    void handleInput();

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<EntityManager> entityManager;

    // Party system
    static constexpr size_t MIN_PARTY_SIZE = 1;
    static constexpr size_t MAX_PARTY_SIZE = 10;
    std::vector<std::shared_ptr<PlayerEntity>> party;
    size_t activePlayerIndex;

    // Camera
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // Camera transition system
    bool cameraTransitioning;
    float transitionTimer;
    static constexpr float CAMERA_TRANSITION_DURATION = 1.0f; // seconds
    glm::vec3 cameraVelocity;
    glm::vec3 cameraAcceleration;
    size_t transitionTargetIndex;

    void startCameraTransition(size_t targetIndex);
    void updateCameraTransition(float deltaTime);

    // Timing
    double lastFrameTime;

    // Window size tracking for resize handling
    int lastWindowWidth;
    int lastWindowHeight;

    bool running;

    void updateProjectionMatrix();
};
