#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputManager {
public:
    InputManager(GLFWwindow* window);

    void update();

    bool isRightMouseButtonDown() const { return rightMouseDown; }
    bool isRightMouseButtonPressed() const { return rightMousePressed; }
    bool isRightMouseButtonReleased() const { return rightMouseReleased; }

    bool isTabPressed() const { return tabPressed; }

    glm::vec2 getMousePosition() const { return mousePosition; }
    glm::vec2 getMouseDelta() const { return mouseDelta; }

    // Convert screen coordinates to world coordinates
    glm::vec3 screenToWorld(const glm::vec2& screenPos, const glm::mat4& view, const glm::mat4& projection);

private:
    GLFWwindow* window;

    bool rightMouseDown;
    bool rightMousePressed;
    bool rightMouseReleased;
    bool prevRightMouseDown;

    bool tabDown;
    bool tabPressed;
    bool prevTabDown;

    glm::vec2 mousePosition;
    glm::vec2 prevMousePosition;
    glm::vec2 mouseDelta;
};
