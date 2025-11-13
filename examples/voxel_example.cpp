// Example program demonstrating VoxelModel usage
// This shows how to integrate voxel models into your game

#include "voxel_model.h"
#include "voxel_shader.h"
#include "renderer.h"
#include <iostream>

void createTestCube(VoxelModel& model) {
    // Create a colorful 5x5x5 cube
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            for (int z = 0; z < 5; z++) {
                // Color based on position
                uint8_t r = (x * 255) / 5;
                uint8_t g = (y * 255) / 5;
                uint8_t b = (z * 255) / 5;
                model.setVoxel(x, y, z, Voxel(r, g, b));
            }
        }
    }
}

void createTestPyramid(VoxelModel& model) {
    // Create a pyramid with decreasing layers
    int size = 9;
    for (int y = 0; y < 5; y++) {
        int layerSize = size - (y * 2);
        int offset = y;

        for (int x = 0; x < layerSize; x++) {
            for (int z = 0; z < layerSize; z++) {
                // Golden color
                model.setVoxel(x + offset, y, z + offset, Voxel(218, 165, 32));
            }
        }
    }
}

void createTestCharacter(VoxelModel& model) {
    // Simple character: body, head, arms, legs

    // Legs
    for (int y = 0; y < 4; y++) {
        model.setVoxel(1, y, 1, Voxel(100, 100, 255)); // Left leg
        model.setVoxel(3, y, 1, Voxel(100, 100, 255)); // Right leg
    }

    // Body
    for (int y = 4; y < 8; y++) {
        for (int x = 1; x < 4; x++) {
            model.setVoxel(x, y, 1, Voxel(255, 100, 100)); // Red torso
        }
    }

    // Arms
    for (int y = 5; y < 7; y++) {
        model.setVoxel(0, y, 1, Voxel(255, 200, 150)); // Left arm
        model.setVoxel(4, y, 1, Voxel(255, 200, 150)); // Right arm
    }

    // Head
    for (int y = 8; y < 10; y++) {
        for (int x = 1; x < 4; x++) {
            model.setVoxel(x, y, 1, Voxel(255, 220, 180)); // Skin color
        }
    }

    // Eyes
    model.setVoxel(1, 9, 1, Voxel(0, 0, 0));
    model.setVoxel(3, 9, 1, Voxel(0, 0, 0));
}

// Example of integrating with game loop
class VoxelGameExample {
public:
    bool initialize() {
        // Initialize voxel shader
        if (!voxelShader.initialize()) {
            std::cerr << "Failed to initialize voxel shader" << std::endl;
            return false;
        }

        // Create test models
        auto cube = voxelManager.createModel();
        createTestCube(*cube);
        cube->generateMesh();
        cube->position = glm::vec3(-3.0f, 0.0f, 0.0f);
        cube->scale = glm::vec3(0.2f, 0.2f, 0.2f);

        auto pyramid = voxelManager.createModel();
        createTestPyramid(*pyramid);
        pyramid->generateMesh();
        pyramid->position = glm::vec3(0.0f, 0.0f, 0.0f);
        pyramid->scale = glm::vec3(0.2f, 0.2f, 0.2f);

        auto character = voxelManager.createModel();
        createTestCharacter(*character);
        character->generateMesh();
        character->position = glm::vec3(3.0f, 0.0f, 0.0f);
        character->scale = glm::vec3(0.2f, 0.2f, 0.2f);

        std::cout << "Voxel models initialized successfully" << std::endl;
        return true;
    }

    void update(float deltaTime) {
        // Example: rotate models
        time += deltaTime;

        // You can update model transformations here
        // model->rotation.y = time;
    }

    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
        // Render all voxel models
        voxelShader.use();
        voxelManager.renderAll(voxelShader.getProgram(), viewMatrix, projectionMatrix);
    }

private:
    VoxelShader voxelShader;
    VoxelModelManager voxelManager;
    float time = 0.0f;
};

// Example of loading from file
void exampleLoadFromFile() {
    VoxelModel model;

    if (model.loadFromVox("assets/models/character.vox")) {
        std::cout << "Successfully loaded model:" << std::endl;
        std::cout << "  Voxels: " << model.getVoxelCount() << std::endl;
        std::cout << "  Size: " << model.getSize().x << "x"
                  << model.getSize().y << "x"
                  << model.getSize().z << std::endl;

        model.generateMesh();
        std::cout << "  Vertices: " << model.getVertexCount() << std::endl;
        std::cout << "  Triangles: " << model.getTriangleCount() << std::endl;
    } else {
        std::cerr << "Failed to load model file" << std::endl;
    }
}

// Example of procedural generation
void exampleProceduralTerrain(VoxelModel& model) {
    const int width = 32;
    const int depth = 32;

    // Simple height map terrain
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            // Simple sine wave pattern for height
            float height = 5.0f + 3.0f * sin(x * 0.2f) * cos(z * 0.2f);
            int h = static_cast<int>(height);

            for (int y = 0; y <= h; y++) {
                // Color based on height (grass = green, dirt = brown)
                Voxel voxel;
                if (y == h) {
                    voxel = Voxel(50, 200, 50); // Grass
                } else if (y >= h - 2) {
                    voxel = Voxel(139, 69, 19); // Dirt
                } else {
                    voxel = Voxel(128, 128, 128); // Stone
                }

                model.setVoxel(x, y, z, voxel);
            }
        }
    }

    model.generateMesh();
    std::cout << "Generated terrain with " << model.getVoxelCount()
              << " voxels" << std::endl;
}

int main() {
    std::cout << "=== VoxelModel Example ===" << std::endl;
    std::cout << std::endl;

    // Example 1: Create simple shapes
    std::cout << "1. Creating test cube..." << std::endl;
    VoxelModel cube;
    createTestCube(cube);
    cube.generateMesh();
    std::cout << "   Created cube with " << cube.getVoxelCount()
              << " voxels and " << cube.getTriangleCount() << " triangles" << std::endl;

    // Example 2: Create pyramid
    std::cout << "2. Creating pyramid..." << std::endl;
    VoxelModel pyramid;
    createTestPyramid(pyramid);
    pyramid.generateMesh();
    std::cout << "   Created pyramid with " << pyramid.getVoxelCount()
              << " voxels and " << pyramid.getTriangleCount() << " triangles" << std::endl;

    // Example 3: Create character
    std::cout << "3. Creating character..." << std::endl;
    VoxelModel character;
    createTestCharacter(character);
    character.generateMesh();
    std::cout << "   Created character with " << character.getVoxelCount()
              << " voxels and " << character.getTriangleCount() << " triangles" << std::endl;

    // Example 4: Procedural terrain
    std::cout << "4. Generating procedural terrain..." << std::endl;
    VoxelModel terrain;
    exampleProceduralTerrain(terrain);

    std::cout << std::endl;
    std::cout << "Example completed successfully!" << std::endl;
    std::cout << "See VOXEL_USAGE.md for integration guide." << std::endl;

    return 0;
}
