#include "input.h"
#include <glm/gtc/matrix_transform.hpp>

InputManager::InputManager(GLFWwindow* window)
    : window(window)
    , rightMouseDown(false)
    , rightMousePressed(false)
    , rightMouseReleased(false)
    , prevRightMouseDown(false)
    , mousePosition(0.0f)
    , prevMousePosition(0.0f)
    , mouseDelta(0.0f)
{
}

void InputManager::update() {
    // Store previous states
    prevRightMouseDown = rightMouseDown;
    prevMousePosition = mousePosition;

    // Get current mouse button state
    rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Calculate pressed/released states
    rightMousePressed = rightMouseDown && !prevRightMouseDown;
    rightMouseReleased = !rightMouseDown && prevRightMouseDown;

    // Get mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));

    // Calculate delta
    mouseDelta = mousePosition - prevMousePosition;
}

glm::vec3 InputManager::screenToWorld(const glm::vec2& screenPos, const glm::mat4& view, const glm::mat4& projection) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen coordinates to NDC
    float x = (2.0f * screenPos.x) / width - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y) / height;

    // Create ray in NDC space
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Transform to eye space
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Transform to world space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    // Get camera position from view matrix
    glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]);

    // Intersect ray with y=0 plane (ground)
    float t = -cameraPos.y / rayWorld.y;
    glm::vec3 worldPos = cameraPos + rayWorld * t;

    return worldPos;
}
