# Action RPG Game Plan

## Overview
A fast-paced action RPG similar to Diablo, built with OpenGL for cross-platform support (Windows, Mac, Linux). Focus on fluid gameplay, high frame rate, and minimal asset requirements.

## Core Design Principles
- **Fast-paced combat**: Quick actions, short cooldowns, responsive controls
- **High frame rate**: Target 144+ FPS, with smooth animations
- **Minimal assets**: Simple 3D models with procedural/decal-based effects
- **Mouse-driven**: Primary control with keyboard shortcuts for abilities/inventory

## Basic Gameplay Loop

### 1. Core Loop (per session)
```
Player spawns → Explore area → Encounter enemies → Combat →
Collect loot → Upgrade character → Repeat (increasing difficulty)
```

### 2. Combat Loop (moment-to-moment)
```
Spot enemy → Position/Dodge → Attack/Cast ability →
Enemy reacts → Player reacts → Victory → Loot drops
```

### 3. Progression Loop
```
Gain XP → Level up → Unlock abilities → Find better gear →
Face stronger enemies → Require better tactics
```

## Technical Architecture

### Graphics System
**Rendering Approach:**
- Modern OpenGL (3.3+ core profile for compatibility)
- Deferred rendering for multiple light sources
- Simple geometric shapes for base models (cubes, spheres, cylinders)
- Particle systems for effects (hits, spells, loot drops)
- Decal system for visual variety (burns, blood, magic circles)
- Screen-space effects (bloom, motion blur for fast movement)

**Asset Strategy:**
- Base character: ~100 triangles (humanoid made from primitives)
- Procedural animations using skeletal system
- Texture-less or minimal textures (flat colors + lighting)
- Heavy use of shaders for visual effects
- Particle effects for "juice" (satisfaction feedback)

### Core Systems

#### 1. Character System
- Player stats: Health, Energy, Movement Speed, Attack Speed
- Simple skill tree (5-7 active abilities)
- Passive modifiers from gear
- Quick weapon switching (2-3 weapon sets)

#### 2. Combat System
- Click-to-move pathfinding (A* or navmesh)
- Click-to-attack targeting
- Area of effect abilities
- Damage types: Physical, Fire, Ice, Lightning
- Simple enemy AI (aggro, chase, attack patterns)

#### 3. Loot System
- Randomized item drops
- Rarity tiers (Common, Rare, Epic, Legendary)
- Stat modifiers (+damage, +speed, +health, etc.)
- Visual feedback for item quality (color-coded particles)

#### 4. Level System
- Procedurally generated or hand-crafted tile-based maps
- Fog of war (unexplored areas hidden)
- Environmental hazards (fire, ice patches, traps)
- Checkpoints/Waypoints for fast travel

## Development Phases

### Phase 1: Foundation (Weeks 1-2)
- [ ] OpenGL context and window management (GLFW)
- [ ] Basic camera system (isometric/top-down view)
- [ ] Simple 3D renderer (cubes/spheres)
- [ ] Input handling (mouse position, clicks, keyboard)
- [ ] Basic player movement (click-to-move)

### Phase 2: Core Gameplay (Weeks 3-4)
- [ ] Player character model and animation system
- [ ] Combat system (basic melee attack)
- [ ] Enemy AI (one enemy type)
- [ ] Health/damage system
- [ ] Simple UI (health bar, XP bar)

### Phase 3: Polish & Systems (Weeks 5-6)
- [ ] Particle effects system
- [ ] Multiple abilities (3-4 skills)
- [ ] Loot drops and inventory
- [ ] Enemy variety (3-4 types)
- [ ] Level progression/XP system

### Phase 4: Content & Effects (Weeks 7-8)
- [ ] Decal system for visual variety
- [ ] Sound effects integration
- [ ] Multiple levels/areas
- [ ] Boss encounters
- [ ] Polish animations and effects

### Phase 5: Optimization & Release (Weeks 9-10)
- [ ] Performance optimization (60+ FPS on mid-range hardware)
- [ ] Cross-platform testing
- [ ] Menu system (settings, save/load)
- [ ] Balance tuning
- [ ] Bug fixes and final polish

## Technology Stack

### Core Libraries
- **GLFW**: Window management and input
- **GLAD/GLEW**: OpenGL loading
- **GLM**: Math library (vectors, matrices)
- **stb_image**: Texture loading (if needed)
- **ImGui**: Debug UI and potentially game UI

### Build System
- **CMake**: Cross-platform build configuration
- **C++17**: Modern C++ features for clean code

### Optional Libraries
- **OpenAL**: Audio (can be added later)
- **PhysicsFS**: Virtual file system for assets
- **JSON library**: Save data and configuration

## Minimal Asset Requirements

### 3D Models
- Player: Simple humanoid (spheres + cylinders)
- Enemies: 3-4 basic types (variations of simple shapes)
- Weapons: Simple geometric shapes (sword = elongated box, staff = cylinder)
- Environment: Modular tiles (floor, walls, props)

### Effects
- Particle textures: 2-3 simple shapes (circle, diamond, star)
- Decals: Procedurally generated or simple patterns
- UI elements: Geometric shapes with shaders

### Textures
- Minimal or none on models (use vertex colors + lighting)
- Simple gradients for particles
- UI icons can be procedural or basic shapes

## Key Features for Fast Gameplay

1. **Instant feedback**: Hit effects, screen shake, particles
2. **Smooth movement**: Interpolated position updates
3. **Responsive controls**: No input lag, immediate reactions
4. **Clear visuals**: Enemy telegraphing, danger zones visible
5. **Satisfying combat**: Impact sounds, visual effects, knockback
6. **Fast progression**: Quick level-ups, frequent loot drops

## Success Metrics
- Maintain 144 FPS on mid-range GPU (GTX 1060 / RX 580 equivalent)
- Player movement latency < 16ms
- Visual clarity maintained during intense combat (30+ enemies)
- Asset creation time: < 30 minutes per enemy type
- Build time: < 2 minutes for full compilation

## Next Steps
1. **Review and approve this plan**
2. Set up the project structure and build system
3. Implement Phase 1 (Foundation)
4. Iterate based on gameplay feel
