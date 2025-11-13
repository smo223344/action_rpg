#include "voxel_model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

// VoxelModel Implementation

VoxelModel::VoxelModel()
    : position(0.0f)
    , rotation(0.0f)
    , scale(1.0f)
    , size(0)
    , VAO(0)
    , VBO(0)
    , EBO(0)
    , meshGenerated(false)
    , buffersInitialized(false)
{
}

VoxelModel::~VoxelModel() {
    cleanupBuffers();
}

void VoxelModel::setVoxel(int x, int y, int z, const Voxel& voxel) {
    setVoxel(VoxelPos(x, y, z), voxel);
}

void VoxelModel::setVoxel(const VoxelPos& pos, const Voxel& voxel) {
    if (voxel.isVisible()) {
        voxels[pos] = voxel;
        updateBoundingBox();
    } else {
        removeVoxel(pos);
    }
}

Voxel VoxelModel::getVoxel(int x, int y, int z) const {
    return getVoxel(VoxelPos(x, y, z));
}

Voxel VoxelModel::getVoxel(const VoxelPos& pos) const {
    auto it = voxels.find(pos);
    if (it != voxels.end()) {
        return it->second;
    }
    return Voxel(); // Empty voxel
}

void VoxelModel::removeVoxel(int x, int y, int z) {
    removeVoxel(VoxelPos(x, y, z));
}

void VoxelModel::removeVoxel(const VoxelPos& pos) {
    voxels.erase(pos);
    updateBoundingBox();
}

void VoxelModel::clear() {
    voxels.clear();
    clearMesh();
    size = glm::ivec3(0);
}

void VoxelModel::clearMesh() {
    vertices.clear();
    indices.clear();
    meshGenerated = false;
}

bool VoxelModel::isVoxelSolid(int x, int y, int z) const {
    return getVoxel(x, y, z).isVisible();
}

void VoxelModel::updateBoundingBox() {
    if (voxels.empty()) {
        size = glm::ivec3(0);
        return;
    }

    glm::ivec3 minPos(INT_MAX);
    glm::ivec3 maxPos(INT_MIN);

    for (const auto& pair : voxels) {
        const VoxelPos& pos = pair.first;
        minPos.x = std::min(minPos.x, pos.x);
        minPos.y = std::min(minPos.y, pos.y);
        minPos.z = std::min(minPos.z, pos.z);
        maxPos.x = std::max(maxPos.x, pos.x);
        maxPos.y = std::max(maxPos.y, pos.y);
        maxPos.z = std::max(maxPos.z, pos.z);
    }

    size = maxPos - minPos + glm::ivec3(1);
}

void VoxelModel::initBuffers() {
    if (buffersInitialized) {
        return;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    buffersInitialized = true;
}

void VoxelModel::updateBuffers() {
    if (!buffersInitialized) {
        initBuffers();
    }

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelVertex),
                 vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                         (void*)offsetof(VoxelVertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                         (void*)offsetof(VoxelVertex, normal));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex),
                         (void*)offsetof(VoxelVertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void VoxelModel::cleanupBuffers() {
    if (buffersInitialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        buffersInitialized = false;
    }
}

void VoxelModel::addQuad(const glm::vec3& pos, const glm::vec3& normal,
                         const glm::vec2& size, const glm::vec3& color) {
    // Determine quad orientation based on normal
    glm::vec3 v0, v1, v2, v3;

    if (normal.x != 0) { // X-axis face
        float x = pos.x + (normal.x > 0 ? 1.0f : 0.0f);
        v0 = glm::vec3(x, pos.y, pos.z);
        v1 = glm::vec3(x, pos.y + size.y, pos.z);
        v2 = glm::vec3(x, pos.y + size.y, pos.z + size.x);
        v3 = glm::vec3(x, pos.y, pos.z + size.x);
    } else if (normal.y != 0) { // Y-axis face
        float y = pos.y + (normal.y > 0 ? 1.0f : 0.0f);
        v0 = glm::vec3(pos.x, y, pos.z);
        v1 = glm::vec3(pos.x, y, pos.z + size.y);
        v2 = glm::vec3(pos.x + size.x, y, pos.z + size.y);
        v3 = glm::vec3(pos.x + size.x, y, pos.z);
    } else { // Z-axis face
        float z = pos.z + (normal.z > 0 ? 1.0f : 0.0f);
        v0 = glm::vec3(pos.x, pos.y, z);
        v1 = glm::vec3(pos.x + size.x, pos.y, z);
        v2 = glm::vec3(pos.x + size.x, pos.y + size.y, z);
        v3 = glm::vec3(pos.x, pos.y + size.y, z);
    }

    // Add vertices
    GLuint baseIndex = static_cast<GLuint>(vertices.size());
    vertices.emplace_back(v0, normal, color);
    vertices.emplace_back(v1, normal, color);
    vertices.emplace_back(v2, normal, color);
    vertices.emplace_back(v3, normal, color);

    // Add indices (two triangles)
    if (normal.x > 0 || normal.y > 0 || normal.z > 0) {
        // Counter-clockwise winding
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    } else {
        // Clockwise winding for back faces
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 2);
    }
}

void VoxelModel::generateMesh() {
    clearMesh();

    if (voxels.empty()) {
        return;
    }

    // Simple naive meshing - create a cube for each voxel with visible faces
    // This is not optimized but works correctly
    for (const auto& pair : voxels) {
        const VoxelPos& pos = pair.first;
        const Voxel& voxel = pair.second;

        if (!voxel.isVisible()) {
            continue;
        }

        // Convert color from 0-255 to 0.0-1.0
        glm::vec3 color(
            voxel.color.r / 255.0f,
            voxel.color.g / 255.0f,
            voxel.color.b / 255.0f
        );

        glm::vec3 voxelPos(pos.x, pos.y, pos.z);

        // Check each face and add if not occluded
        // +X face
        if (!isVoxelSolid(pos.x + 1, pos.y, pos.z)) {
            addQuad(voxelPos, glm::vec3(1, 0, 0), glm::vec2(1, 1), color);
        }
        // -X face
        if (!isVoxelSolid(pos.x - 1, pos.y, pos.z)) {
            addQuad(voxelPos, glm::vec3(-1, 0, 0), glm::vec2(1, 1), color);
        }
        // +Y face
        if (!isVoxelSolid(pos.x, pos.y + 1, pos.z)) {
            addQuad(voxelPos, glm::vec3(0, 1, 0), glm::vec2(1, 1), color);
        }
        // -Y face
        if (!isVoxelSolid(pos.x, pos.y - 1, pos.z)) {
            addQuad(voxelPos, glm::vec3(0, -1, 0), glm::vec2(1, 1), color);
        }
        // +Z face
        if (!isVoxelSolid(pos.x, pos.y, pos.z + 1)) {
            addQuad(voxelPos, glm::vec3(0, 0, 1), glm::vec2(1, 1), color);
        }
        // -Z face
        if (!isVoxelSolid(pos.x, pos.y, pos.z - 1)) {
            addQuad(voxelPos, glm::vec3(0, 0, -1), glm::vec2(1, 1), color);
        }
    }

    updateBuffers();
    meshGenerated = true;

    std::cout << "Generated mesh with " << vertices.size() << " vertices and "
              << (indices.size() / 3) << " triangles" << std::endl;
}

void VoxelModel::render(GLuint shaderProgram, const glm::mat4& modelMatrix,
                       const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (!meshGenerated || vertices.empty()) {
        return;
    }

    glUseProgram(shaderProgram);

    // Build model transformation
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);
    model = modelMatrix * model;

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                      glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                      glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE,
                      glm::value_ptr(projectionMatrix));

    // Render mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// MagicaVoxel .vox file format implementation
bool VoxelModel::readVoxChunk(FILE* file, VoxChunk& chunk) {
    if (fread(chunk.id, 1, 4, file) != 4) return false;
    if (fread(&chunk.contentSize, sizeof(int), 1, file) != 1) return false;
    if (fread(&chunk.childrenSize, sizeof(int), 1, file) != 1) return false;
    return true;
}

bool VoxelModel::loadFromVox(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // Read VOX header
    char magic[4];
    int version;
    if (fread(magic, 1, 4, file) != 4 || strncmp(magic, "VOX ", 4) != 0) {
        std::cerr << "Invalid VOX file format" << std::endl;
        fclose(file);
        return false;
    }

    if (fread(&version, sizeof(int), 1, file) != 1) {
        std::cerr << "Failed to read version" << std::endl;
        fclose(file);
        return false;
    }

    std::cout << "Loading VOX file version " << version << std::endl;

    // Default MagicaVoxel palette
    Voxel palette[256];
    bool paletteLoaded = false;

    // Initialize with default palette (MagicaVoxel default)
    const uint32_t defaultPalette[256] = {
        0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff,
        0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
        0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
        0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
        0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc,
        0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
        0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc,
        0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
        0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
        0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
        0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999,
        0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
        0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099,
        0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
        0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
        0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
        0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366,
        0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
        0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33,
        0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
        0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
        0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
        0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00,
        0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
        0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600,
        0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
        0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
        0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
        0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700,
        0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
        0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd,
        0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
    };

    for (int i = 0; i < 256; i++) {
        uint32_t col = defaultPalette[i];
        palette[i] = Voxel((col >> 16) & 0xFF, (col >> 8) & 0xFF, col & 0xFF, (col >> 24) & 0xFF);
    }

    clear();

    // Read chunks
    while (!feof(file)) {
        VoxChunk chunk;
        if (!readVoxChunk(file, chunk)) {
            break;
        }

        if (strncmp(chunk.id, "SIZE", 4) == 0) {
            // Model size
            int sx, sy, sz;
            fread(&sx, sizeof(int), 1, file);
            fread(&sy, sizeof(int), 1, file);
            fread(&sz, sizeof(int), 1, file);
            std::cout << "Model size: " << sx << "x" << sy << "x" << sz << std::endl;
        }
        else if (strncmp(chunk.id, "XYZI", 4) == 0) {
            // Voxel data
            int numVoxels;
            fread(&numVoxels, sizeof(int), 1, file);
            std::cout << "Loading " << numVoxels << " voxels" << std::endl;

            for (int i = 0; i < numVoxels; i++) {
                uint8_t x, y, z, colorIndex;
                fread(&x, 1, 1, file);
                fread(&y, 1, 1, file);
                fread(&z, 1, 1, file);
                fread(&colorIndex, 1, 1, file);

                if (colorIndex > 0 && colorIndex <= 255) {
                    setVoxel(x, y, z, palette[colorIndex]);
                }
            }
        }
        else if (strncmp(chunk.id, "RGBA", 4) == 0) {
            // Custom palette
            for (int i = 0; i < 256; i++) {
                uint8_t r, g, b, a;
                fread(&r, 1, 1, file);
                fread(&g, 1, 1, file);
                fread(&b, 1, 1, file);
                fread(&a, 1, 1, file);
                palette[i] = Voxel(r, g, b, a);
            }
            paletteLoaded = true;
            std::cout << "Custom palette loaded" << std::endl;
        }
        else {
            // Skip unknown chunks
            fseek(file, chunk.contentSize, SEEK_CUR);
        }

        // Skip children
        if (chunk.childrenSize > 0) {
            fseek(file, chunk.childrenSize, SEEK_CUR);
        }
    }

    fclose(file);

    std::cout << "Loaded " << getVoxelCount() << " voxels from " << filename << std::endl;
    return getVoxelCount() > 0;
}

bool VoxelModel::saveToVox(const std::string& filename) const {
    // TODO: Implement VOX file saving
    std::cerr << "VOX file saving not yet implemented" << std::endl;
    return false;
}

// VoxelModelManager Implementation

VoxelModelManager::VoxelModelManager() {
}

VoxelModelManager::~VoxelModelManager() {
    clear();
}

std::shared_ptr<VoxelModel> VoxelModelManager::createModel() {
    auto model = std::make_shared<VoxelModel>();
    models.push_back(model);
    return model;
}

std::shared_ptr<VoxelModel> VoxelModelManager::loadModel(const std::string& filename) {
    auto model = std::make_shared<VoxelModel>();
    if (model->loadFromVox(filename)) {
        model->generateMesh();
        models.push_back(model);
        return model;
    }
    return nullptr;
}

void VoxelModelManager::removeModel(std::shared_ptr<VoxelModel> model) {
    models.erase(std::remove(models.begin(), models.end(), model), models.end());
}

void VoxelModelManager::clear() {
    models.clear();
}

void VoxelModelManager::renderAll(GLuint shaderProgram, const glm::mat4& viewMatrix,
                                 const glm::mat4& projectionMatrix) {
    glm::mat4 identity(1.0f);
    for (auto& model : models) {
        if (model && model->hasMesh()) {
            model->render(shaderProgram, identity, viewMatrix, projectionMatrix);
        }
    }
}
