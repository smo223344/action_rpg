#include "game.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Game::Game()
    : running(false)
    , lastFrameTime(0.0)
    , lastWindowWidth(0)
    , lastWindowHeight(0)
    , activePlayerIndex(0)
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

    // Create party with 3 player characters
    // Character 1 - Red
    auto player1 = std::make_shared<PlayerEntity>();
    player1->position = glm::vec3(0.0f, 0.0f, 0.0f);
    player1->color = glm::vec3(0.9f, 0.2f, 0.2f); // Red
    party.push_back(player1);
    entityManager->addEntity(player1);

    // Character 2 - Green
    auto player2 = std::make_shared<PlayerEntity>();
    player2->position = glm::vec3(2.0f, 0.0f, 0.0f);
    player2->color = glm::vec3(0.2f, 0.9f, 0.2f); // Green
    party.push_back(player2);
    entityManager->addEntity(player2);

    // Character 3 - Blue
    auto player3 = std::make_shared<PlayerEntity>();
    player3->position = glm::vec3(-2.0f, 0.0f, 0.0f);
    player3->color = glm::vec3(0.2f, 0.2f, 0.9f); // Blue
    party.push_back(player3);
    entityManager->addEntity(player3);

    // Start with the first character active
    activePlayerIndex = 0;

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
    std::cout << "Party size: " << party.size() << " characters" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Right-click and hold to move the active character" << std::endl;
    std::cout << "  Tab to switch between party members" << std::endl;

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

    // Tab key to switch between party members
    if (inputManager->isTabPressed()) {
        activePlayerIndex = (activePlayerIndex + 1) % party.size();
        std::cout << "Switched to character " << (activePlayerIndex + 1) << " / " << party.size() << std::endl;
    }

    // Get the active player
    auto activePlayer = party[activePlayerIndex];

    // Right mouse button hold to move
    if (inputManager->isRightMouseButtonDown()) {
        glm::vec2 mousePos = inputManager->getMousePosition();
        glm::vec3 worldPos = inputManager->screenToWorld(mousePos, viewMatrix, projectionMatrix);

        // Move active player to cursor position
        if (activePlayer) {
            activePlayer->moveTo(worldPos);
        }
    }

    // Stop moving when right mouse button is released
    if (inputManager->isRightMouseButtonReleased()) {
        if (activePlayer) {
            activePlayer->stop();
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

    // Update camera to follow active player
    if (!party.empty() && activePlayerIndex < party.size()) {
        auto activePlayer = party[activePlayerIndex];
        cameraTarget = activePlayer->position;
        cameraPosition = activePlayer->position + glm::vec3(0.0f, 15.0f, 15.0f);
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
