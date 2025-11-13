#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Represents a single voxel with color information
struct Voxel {
    glm::u8vec4 color; // RGBA color (0-255)

    Voxel() : color(0, 0, 0, 0) {}
    Voxel(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : color(r, g, b, a) {}

    bool isVisible() const { return color.a > 0; }
};

// 3D position hash for sparse voxel storage
struct VoxelPos {
    int x, y, z;

    VoxelPos(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}

    bool operator==(const VoxelPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

// Hash function for VoxelPos
namespace std {
    template<>
    struct hash<VoxelPos> {
        size_t operator()(const VoxelPos& pos) const {
            size_t h1 = hash<int>()(pos.x);
            size_t h2 = hash<int>()(pos.y);
            size_t h3 = hash<int>()(pos.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

// Vertex data for mesh generation
struct VoxelVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    VoxelVertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec3& col)
        : position(pos), normal(norm), color(col) {}
};

/**
 * VoxelModel - Represents a voxel-based 3D model component
 *
 * Features:
 * - Sparse voxel storage for memory efficiency
 * - Loading from MagicaVoxel .vox format
 * - Greedy meshing for optimized rendering
 * - OpenGL rendering with normals and colors
 */
class VoxelModel {
public:
    VoxelModel();
    ~VoxelModel();

    // Voxel manipulation
    void setVoxel(int x, int y, int z, const Voxel& voxel);
    void setVoxel(const VoxelPos& pos, const Voxel& voxel);
    Voxel getVoxel(int x, int y, int z) const;
    Voxel getVoxel(const VoxelPos& pos) const;
    void removeVoxel(int x, int y, int z);
    void removeVoxel(const VoxelPos& pos);
    void clear();

    // File I/O
    bool loadFromVox(const std::string& filename);
    bool saveToVox(const std::string& filename) const;

    // Mesh generation
    void generateMesh();
    void clearMesh();

    // Rendering
    void render(GLuint shaderProgram, const glm::mat4& modelMatrix,
                const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    // Getters
    const glm::ivec3& getSize() const { return size; }
    size_t getVoxelCount() const { return voxels.size(); }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getTriangleCount() const { return indices.size() / 3; }
    bool hasMesh() const { return !vertices.empty(); }

    // Transformation
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

private:
    // Voxel data (sparse storage)
    std::unordered_map<VoxelPos, Voxel> voxels;
    glm::ivec3 size; // Bounding box size

    // Mesh data
    std::vector<VoxelVertex> vertices;
    std::vector<GLuint> indices;

    // OpenGL resources
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    bool meshGenerated;
    bool buffersInitialized;

    // Helper methods
    void initBuffers();
    void updateBuffers();
    void cleanupBuffers();

    // Mesh generation helpers
    void addQuad(const glm::vec3& pos, const glm::vec3& normal,
                 const glm::vec2& size, const glm::vec3& color);
    bool isVoxelSolid(int x, int y, int z) const;
    void updateBoundingBox();

    // MagicaVoxel .vox format helpers
    struct VoxChunk {
        char id[4];
        int contentSize;
        int childrenSize;
    };

    bool readVoxChunk(FILE* file, VoxChunk& chunk);
};

// Voxel model manager for handling multiple model components
class VoxelModelManager {
public:
    VoxelModelManager();
    ~VoxelModelManager();

    std::shared_ptr<VoxelModel> createModel();
    std::shared_ptr<VoxelModel> loadModel(const std::string& filename);
    void removeModel(std::shared_ptr<VoxelModel> model);
    void clear();

    void renderAll(GLuint shaderProgram, const glm::mat4& viewMatrix,
                   const glm::mat4& projectionMatrix);

private:
    std::vector<std::shared_ptr<VoxelModel>> models;
};
