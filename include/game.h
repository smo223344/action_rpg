#pragma once

#include "renderer.h"
#include "entity.h"
#include "input.h"
#include <memory>

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

    std::shared_ptr<PlayerEntity> player;

    // Camera
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // Timing
    double lastFrameTime;

    // Window size tracking for resize handling
    int lastWindowWidth;
    int lastWindowHeight;

    bool running;

    void updateProjectionMatrix();
};
