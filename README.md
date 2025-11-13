# Action RPG

A fast-paced action RPG with party-based tactical combat, built with OpenGL and voxel-based rendering.

## Building

### Prerequisites

- CMake 3.15+
- C++17 compiler (GCC, Clang, or MSVC)
- GLFW 3.3+
- OpenGL 3.3+
- GLM (header-only)

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libglfw3-dev libgl1-mesa-dev libglm-dev

# Build
mkdir build && cd build
cmake ..
make -j4

# Run
./bin/ActionRPG
```

### macOS

```bash
# Install dependencies
brew install cmake glfw glm

# Build
mkdir build && cd build
cmake ..
make -j4

# Run
./bin/ActionRPG
```

### Windows

```bash
# Using vcpkg for dependencies
vcpkg install glfw3 glm

# Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# Run
bin\Release\ActionRPG.exe
```

## Controls

- **Right Mouse Button (hold)**: Move player to cursor position
- **ESC**: Close window

## Current Features

- OpenGL 3.3 rendering
- Entity-based game architecture
- Right-mouse-button movement control
- Isometric overhead camera
- Dynamic grid rendering
- Player entity (placeholder circle)

## Planned Features

See [PLAN.md](PLAN.md) for the full development roadmap.
