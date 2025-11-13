#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class Entity;
class EntityManager;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool initialize(int width, int height, const char* title);
    void shutdown();

    void beginFrame();
    void endFrame();

    void renderGrid(float gridSize, int gridCount, const glm::vec3& color);
    void renderCircle(const glm::vec3& position, float radius, const glm::vec3& color, int segments = 32);
    void renderEntities(const EntityManager& entityManager);

    bool shouldClose() const;
    GLFWwindow* getWindow() const { return window; }

    void setViewMatrix(const glm::mat4& view) { viewMatrix = view; }
    void setProjectionMatrix(const glm::mat4& projection) { projectionMatrix = projection; }

private:
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // Shader program
    GLuint shaderProgram;
    GLuint VAO, VBO;

    void createShaderProgram();
    void setupBuffers();
};
