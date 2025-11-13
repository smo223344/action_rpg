#pragma once

#include <glad/glad.h>
#include <string>

/**
 * VoxelShader - Shader program for rendering voxel models
 * Supports per-vertex colors, normals, and basic directional lighting
 */
class VoxelShader {
public:
    VoxelShader();
    ~VoxelShader();

    bool initialize();
    void use() const;
    void cleanup();

    GLuint getProgram() const { return shaderProgram; }
    bool isInitialized() const { return initialized; }

private:
    GLuint shaderProgram;
    bool initialized;

    GLuint compileShader(GLenum type, const char* source);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);

    // Shader source code
    static const char* vertexShaderSource;
    static const char* fragmentShaderSource;
};
