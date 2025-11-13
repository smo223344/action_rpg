# VoxelModel Usage Guide

## Overview

The VoxelModel class provides a complete voxel-based 3D model system with:
- Sparse voxel storage for memory efficiency
- MagicaVoxel .vox file format support
- Automatic mesh generation with face culling
- OpenGL rendering with lighting and per-vertex colors

## Quick Start

### 1. Basic Usage - Creating Voxels Manually

```cpp
#include "voxel_model.h"
#include "voxel_shader.h"

// Create a voxel model
VoxelModel model;

// Add some voxels
model.setVoxel(0, 0, 0, Voxel(255, 0, 0));     // Red voxel at (0,0,0)
model.setVoxel(1, 0, 0, Voxel(0, 255, 0));     // Green voxel at (1,0,0)
model.setVoxel(0, 1, 0, Voxel(0, 0, 255));     // Blue voxel at (0,1,0)

// Generate mesh from voxels
model.generateMesh();

// Set position and scale
model.position = glm::vec3(0.0f, 0.0f, 0.0f);
model.scale = glm::vec3(0.1f, 0.1f, 0.1f);     // Scale down voxels to 0.1 units

// Create voxel shader
VoxelShader shader;
shader.initialize();

// In your render loop:
shader.use();
model.render(shader.getProgram(), modelMatrix, viewMatrix, projectionMatrix);
```

### 2. Loading from MagicaVoxel .vox Files

```cpp
#include "voxel_model.h"
#include "voxel_shader.h"

// Load model from .vox file
VoxelModel model;
if (model.loadFromVox("models/character.vox")) {
    model.generateMesh();
    std::cout << "Loaded " << model.getVoxelCount() << " voxels" << std::endl;
    std::cout << "Generated " << model.getTriangleCount() << " triangles" << std::endl;
}

// Render
VoxelShader shader;
shader.initialize();
shader.use();
model.render(shader.getProgram(), modelMatrix, viewMatrix, projectionMatrix);
```

### 3. Using VoxelModelManager

```cpp
#include "voxel_model.h"
#include "voxel_shader.h"

VoxelModelManager manager;
VoxelShader shader;
shader.initialize();

// Load multiple models
auto model1 = manager.loadModel("models/character.vox");
model1->position = glm::vec3(-5.0f, 0.0f, 0.0f);

auto model2 = manager.loadModel("models/enemy.vox");
model2->position = glm::vec3(5.0f, 0.0f, 0.0f);

// Create a procedural model
auto model3 = manager.createModel();
for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 10; y++) {
        model3->setVoxel(x, 0, y, Voxel(128, 64, 32));
    }
}
model3->generateMesh();

// Render all models at once
shader.use();
manager.renderAll(shader.getProgram(), viewMatrix, projectionMatrix);
```

## API Reference

### VoxelModel Class

#### Voxel Manipulation
- `void setVoxel(int x, int y, int z, const Voxel& voxel)` - Set a voxel at position
- `Voxel getVoxel(int x, int y, int z)` - Get voxel at position
- `void removeVoxel(int x, int y, int z)` - Remove voxel at position
- `void clear()` - Clear all voxels

#### File I/O
- `bool loadFromVox(const std::string& filename)` - Load from MagicaVoxel .vox file
- `bool saveToVox(const std::string& filename)` - Save to .vox file (not yet implemented)

#### Mesh Generation
- `void generateMesh()` - Generate optimized mesh from voxels
- `void clearMesh()` - Clear generated mesh
- `bool hasMesh()` - Check if mesh is generated

#### Rendering
- `void render(GLuint shaderProgram, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)` - Render the model

#### Properties
- `glm::vec3 position` - Model position in world space
- `glm::vec3 rotation` - Model rotation (Euler angles in radians)
- `glm::vec3 scale` - Model scale

#### Getters
- `size_t getVoxelCount()` - Get number of voxels
- `size_t getVertexCount()` - Get number of mesh vertices
- `size_t getTriangleCount()` - Get number of triangles
- `const glm::ivec3& getSize()` - Get bounding box size

### Voxel Struct

```cpp
struct Voxel {
    glm::u8vec4 color; // RGBA color (0-255)

    Voxel();                                    // Default (transparent)
    Voxel(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    bool isVisible() const;                     // Check if voxel is visible
};
```

### VoxelShader Class

- `bool initialize()` - Initialize shader program
- `void use()` - Activate shader for rendering
- `GLuint getProgram()` - Get OpenGL shader program ID
- `void cleanup()` - Clean up shader resources

## MagicaVoxel File Format

The implementation supports the MagicaVoxel .vox file format (version 150+):

- **SIZE chunk**: Model dimensions
- **XYZI chunk**: Voxel positions and color indices
- **RGBA chunk**: Custom 256-color palette

You can create .vox files using:
- [MagicaVoxel](https://ephtracy.github.io/) - Free voxel editor
- [Goxel](https://goxel.xyz/) - Open source voxel editor

## Mesh Generation

The mesh generation algorithm:
1. Iterates through all voxels
2. For each voxel, checks 6 adjacent positions
3. Only generates faces that are not occluded by other voxels
4. Creates optimized vertex and index buffers

This results in:
- Efficient rendering (only visible faces)
- Lower memory usage
- Better performance

## Shader Details

The VoxelShader includes:
- Per-vertex colors from voxel data
- Normal calculations for lighting
- Simple directional light (from direction [0.5, 1.0, 0.3])
- Ambient + Diffuse + Specular lighting
- Configurable via shader uniforms

## Performance Tips

1. **Batch operations**: Set multiple voxels before calling `generateMesh()`
2. **Scale appropriately**: Use `model.scale` to size voxels for your scene
3. **Reuse shaders**: Create one VoxelShader and reuse for all models
4. **Use VoxelModelManager**: Simplifies managing multiple models

## Example: Creating a Simple House

```cpp
VoxelModel house;

// Floor (10x10)
for (int x = 0; x < 10; x++) {
    for (int z = 0; z < 10; z++) {
        house.setVoxel(x, 0, z, Voxel(139, 69, 19)); // Brown floor
    }
}

// Walls
for (int x = 0; x < 10; x++) {
    for (int y = 1; y < 5; y++) {
        house.setVoxel(x, y, 0, Voxel(180, 180, 180));  // Front wall
        house.setVoxel(x, y, 9, Voxel(180, 180, 180));  // Back wall
    }
}

for (int z = 0; z < 10; z++) {
    for (int y = 1; y < 5; y++) {
        house.setVoxel(0, y, z, Voxel(180, 180, 180));  // Left wall
        house.setVoxel(9, y, z, Voxel(180, 180, 180));  // Right wall
    }
}

// Roof
for (int x = 0; x < 10; x++) {
    for (int z = 0; z < 10; z++) {
        house.setVoxel(x, 5, z, Voxel(139, 0, 0)); // Red roof
    }
}

house.generateMesh();
house.position = glm::vec3(0.0f, 0.0f, 0.0f);
house.scale = glm::vec3(0.2f, 0.2f, 0.2f);
```

## Integration with Existing Renderer

If you're using the existing Renderer class, you can integrate VoxelShader:

```cpp
// In your Game class or main loop:
VoxelShader voxelShader;
voxelShader.initialize();

VoxelModelManager voxelManager;
auto playerModel = voxelManager.loadModel("models/player.vox");

// In render loop:
renderer.beginFrame();

// Render existing entities
renderer.renderEntities(entityManager);

// Render voxel models
voxelShader.use();
voxelManager.renderAll(voxelShader.getProgram(), viewMatrix, projectionMatrix);

renderer.endFrame();
```

## Future Enhancements

Possible improvements to consider:
- Greedy meshing for further optimization
- LOD (Level of Detail) system
- Voxel animation support
- .vox file saving
- Multiple material types
- Transparency support
- Instanced rendering for repeated models
