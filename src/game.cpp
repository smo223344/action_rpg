#include "game.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Game::Game()
    : running(false)
    , lastFrameTime(0.0)
    , lastWindowWidth(0)
    , lastWindowHeight(0)
{
}

Game::~Game() {
    shutdown();
}

bool Game::initialize() {
    // Create renderer
    renderer = std::make_unique<Renderer>();
    if (!renderer->initialize(1280, 720, "Action RPG")) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    // Create input manager
    inputManager = std::make_unique<InputManager>(renderer->getWindow());

    // Create entity manager
    entityManager = std::make_unique<EntityManager>();

    // Create player entity
    player = std::make_shared<PlayerEntity>();
    player->position = glm::vec3(0.0f, 0.0f, 0.0f);
    player->color = glm::vec3(0.2f, 0.8f, 0.3f); // Green
    entityManager->addEntity(player);

    // Setup camera (isometric-style overhead view)
    cameraPosition = glm::vec3(0.0f, 15.0f, 15.0f);
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

    viewMatrix = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    renderer->setViewMatrix(viewMatrix);

    // Initialize window size tracking and projection matrix
    lastWindowWidth = renderer->getWindowWidth();
    lastWindowHeight = renderer->getWindowHeight();
    updateProjectionMatrix();

    lastFrameTime = glfwGetTime();
    running = true;

    std::cout << "Game initialized successfully" << std::endl;
    std::cout << "Right-click and hold to move the player" << std::endl;

    return true;
}

void Game::run() {
    while (running && !renderer->shouldClose()) {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        // Update
        handleInput();
        update(deltaTime);

        // Render
        render();
    }
}

void Game::shutdown() {
    entityManager.reset();
    inputManager.reset();
    renderer.reset();
}

void Game::handleInput() {
    inputManager->update();

    // Right mouse button hold to move
    if (inputManager->isRightMouseButtonDown()) {
        glm::vec2 mousePos = inputManager->getMousePosition();
        glm::vec3 worldPos = inputManager->screenToWorld(mousePos, viewMatrix, projectionMatrix);

        // Move player to cursor position
        if (player) {
            player->moveTo(worldPos);
        }
    }

    // Stop moving when right mouse button is released
    if (inputManager->isRightMouseButtonReleased()) {
        if (player) {
            player->stop();
        }
    }
}

void Game::update(float deltaTime) {
    // Check for window resize and update projection matrix if needed
    int currentWidth = renderer->getWindowWidth();
    int currentHeight = renderer->getWindowHeight();
    if (currentWidth != lastWindowWidth || currentHeight != lastWindowHeight) {
        lastWindowWidth = currentWidth;
        lastWindowHeight = currentHeight;
        updateProjectionMatrix();
        std::cout << "Window resized, updated projection matrix" << std::endl;
    }

    // Update all entities
    entityManager->updateAll(deltaTime);

    // Update camera to follow player
    if (player) {
        cameraTarget = player->position;
        cameraPosition = player->position + glm::vec3(0.0f, 15.0f, 15.0f);
        viewMatrix = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        renderer->setViewMatrix(viewMatrix);
    }
}

void Game::render() {
    renderer->beginFrame();

    // Render grid
    renderer->renderGrid(1.0f, 40, glm::vec3(0.3f, 0.3f, 0.35f));

    // Render entities
    renderer->renderEntities(*entityManager);

    renderer->endFrame();
}

void Game::updateProjectionMatrix() {
    // Calculate aspect ratio from actual window dimensions
    int width = renderer->getWindowWidth();
    int height = renderer->getWindowHeight();
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    std::cout << "Updating projection matrix - Width: " << width
              << ", Height: " << height
              << ", Aspect: " << aspectRatio << std::endl;

    projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    renderer->setProjectionMatrix(projectionMatrix);
}
