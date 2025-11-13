# Action RPG Game Plan

## Overview
A fast-paced action RPG similar to Diablo, built with OpenGL for cross-platform support (Windows, Mac, Linux). Focus on fluid gameplay, high frame rate, and minimal asset requirements.

## Core Design Principles
- **Party-based tactical combat**: Control multiple characters, switch with Tab key
- **Fast-paced combat**: Quick actions, short cooldowns, responsive controls
- **Strategic depth**: Active character uses full abilities, allies act semi-autonomously
- **High frame rate**: Target 144+ FPS, with smooth animations
- **Minimal assets**: Simple 3D models with procedural/decal-based effects
- **Mouse-driven**: Primary control with keyboard shortcuts for abilities/inventory

## Basic Gameplay Loop

### 1. Core Loop (per session)
```
Party spawns → Explore area → Encounter enemies → Tactical combat →
Collect loot → Upgrade party members → Repeat (increasing difficulty)
```

### 2. Combat Loop (moment-to-moment)
```
Spot enemies → Position party → Switch active character (Tab) →
Use abilities/attacks → Allies auto-attack and dodge →
Coordinate party abilities → Victory → Loot drops
```

### 3. Party Management Loop
```
Active character: Full control (movement, all abilities, special actions) →
Tab to switch → Previous character becomes semi-autonomous →
Semi-autonomous allies: Basic attacks, dodge AOEs, follow/assist →
Strategic switching based on situation
```

### 4. Progression Loop
```
Gain XP → Level up → Unlock abilities → Find better gear →
Face stronger enemies → Require better tactics
```

## Technical Architecture

### Graphics System
**Rendering Approach:**
- Modern OpenGL (3.3+ core profile for compatibility)
- **Unified voxel rendering pipeline** for all solid objects (characters, items, environment)
- Instanced rendering for efficient voxel drawing (thousands of voxels per frame)
- Greedy meshing for static environment voxels (optimize GPU memory)
- Deferred rendering for multiple light sources
- Separate particle systems for non-solid effects (magic, fire, blood)
- Decal system for surface details (burns, blood stains, magic circles)
- Screen-space effects (bloom, motion blur for fast movement)

**Voxel Architecture:**
- Characters built from interchangeable voxel primitives
- Named attachment points for skeletal animation (e.g., "head", "torso", "left_arm", "right_hand")
- Each primitive is a named entity attached at fixed points to other primitives
- Dynamic damage system (voxels can be destroyed/modified during combat)
- Interchangeable armor pieces (swap voxel groups by attachment point)
- Procedural variation (modify voxel colors, sizes, arrangements)

**Scale & Perspective:**
- Small character size (relative to battlefield) for wide isometric view
- Enables visibility of many entities simultaneously (30+ mobs, items, effects)
- Camera positioned for overhead tactical view of battlefield
- Clear silhouettes and color coding for entity identification at small scale

**Asset Strategy:**
- **All solid objects are voxels** - unified rendering and asset pipeline
- Base character: ~8-16 voxels (head, torso, 4 limbs, hands, feet)
- Environment: voxel-based tiles, walls, props
- Items/loot: voxel objects (weapons, armor, potions, gold)
- Skeletal animation via transform hierarchy of named attachment points
- Texture-less rendering (voxel colors + lighting only)
- Heavy use of shaders for visual effects and voxel rendering optimization
- Particle effects for non-solid effects (magic, sparks, smoke, blood)
- Greedy meshing or instancing for voxel rendering performance

### Core Systems

#### 1. Character System
**Party Management:**
- Party size: 2-4 characters
- Tab key cycles through party members
- Active character: Full player control (movement, all abilities, items)
- Semi-autonomous allies: AI-controlled with limited behavior
  - Follow active character or hold position
  - Use basic attacks on nearby enemies
  - Dodge telegraphed attacks and AOE zones
  - Do NOT use powerful/limited abilities (cooldowns, ultimates)
  - Do NOT use consumables or special actions
- Visual indicator for active character (highlight, glow, UI marker)

**Character Stats:**
- Health, Energy, Movement Speed, Attack Speed
- Simple skill tree (5-7 active abilities per character)
- Passive modifiers from gear
- Quick weapon switching (2-3 weapon sets)
- Modular armor system (helmet, chest, gloves, boots - each replaces voxel primitives)
- Visual damage feedback (voxels show damage, can be targeted by enemy attacks)

**Party Composition:**
- Different character classes/roles (warrior, mage, archer, etc.)
- Procedural character variations (different voxel arrangements for visual distinction)
- Each character has unique abilities and playstyle

#### 2. Combat System
**Player Control (Active Character):**
- Click-to-move pathfinding (A* or navmesh)
- Click-to-attack targeting
- Optional targeted attacks (aim at specific body parts/voxel primitives)
- Full access to all abilities (1-6 keys for skills)
- Tactical positioning and ability combos

**Semi-Autonomous Ally AI:**
- Basic attack nearest enemy in range
- Dodge visible AOE zones (red circles, telegraphed attacks)
- Follow active character at medium distance
- Maintain formation or spread out to avoid clustered AOE
- Simple threat assessment (prioritize low-health enemies)
- No cooldown ability usage, no consumables

**Combat Mechanics:**
- Area of effect abilities (affects both enemies and party positioning)
- Damage types: Physical, Fire, Ice, Lightning
- Dynamic damage visualization (damaged voxels change color, can be destroyed)
- Destructible enemies (enough damage to a limb can disable/remove it)
- Enemy AI (aggro, chase, attack patterns, target priority)
- Friendly fire consideration (optional difficulty modifier)

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

## Strategic Party Gameplay

### Why Party-Based?
- **Tactical depth**: Player must decide which character to control each moment
- **Resource management**: Powerful abilities only usable when actively controlling that character
- **Positioning matters**: Allies will dodge but not optimally position themselves
- **Risk/reward**: Switching characters during combat has strategic implications
- **Class synergy**: Different characters complement each other (tank, DPS, support)

### Key Strategic Decisions
1. **When to switch**: Balance controlling damage dealers vs. tanks vs. support
2. **Ability timing**: Coordinate big abilities across party members
3. **Positioning**: Active character controls formation and tactical positioning
4. **Survival**: Switch to characters in danger to use defensive abilities
5. **Boss fights**: Requires switching to use all party members' ultimate abilities

### Semi-Autonomous AI Limitations (By Design)
- **Forces switching**: Players must actively control characters for optimal play
- **Basic survival**: Allies won't die easily but won't maximize damage either
- **No waste**: Allies won't waste limited-use abilities or consumables
- **Predictable**: Simple AI means player can anticipate ally behavior
- **Strategic challenge**: Maximizing party effectiveness requires active management

## Development Phases

### Phase 1: Foundation (Weeks 1-2)
- [ ] OpenGL context and window management (GLFW)
- [ ] Basic camera system (isometric overhead view, wide battlefield)
- [ ] Voxel renderer (instanced rendering or greedy meshing)
- [ ] Simple voxel primitive system (create, position, color)
- [ ] Input handling (mouse position, clicks, keyboard, Tab for switching)
- [ ] Basic character movement (click-to-move)
- [ ] Multiple character entities (party system foundation)

### Phase 2: Core Gameplay (Weeks 3-4)
- [ ] Skeletal attachment system (named attachment points, transform hierarchy)
- [ ] Character voxel models (8-16 primitives with attachment hierarchy)
- [ ] Basic animation system (rotate/translate primitives at attachment points)
- [ ] Party switching system (Tab key cycles active character)
- [ ] Active character control (movement, attacks)
- [ ] Semi-autonomous ally AI (follow, basic attack, dodge)
- [ ] Combat system (basic melee attack with hit detection)
- [ ] Enemy AI (one enemy type with voxel model)
- [ ] Health/damage system with visual voxel damage
- [ ] Party UI (health bars for all party members, active indicator)

### Phase 3: Polish & Systems (Weeks 5-6)
- [ ] Particle effects system
- [ ] Multiple abilities per character (3-4 skills each)
- [ ] Improved ally AI (AOE dodge, better positioning)
- [ ] Different character classes (warrior, mage, archer)
- [ ] Modular armor system (interchangeable voxel primitives)
- [ ] Loot drops and inventory (shared party inventory)
- [ ] Enemy variety (3-4 types with procedural voxel variations)
- [ ] Level progression/XP system (shared party XP)

### Phase 4: Content & Effects (Weeks 7-8)
- [ ] Advanced voxel destruction system (targetable body parts)
- [ ] Decal system for visual variety
- [ ] Sound effects integration
- [ ] Multiple levels/areas
- [ ] Boss encounters (larger voxel models)
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

**Design Philosophy: Everything solid is voxels**
- Unified asset pipeline and rendering system
- Consistent art style across all game objects
- Single workflow for creating any game object
- All objects can be procedurally modified, colored, and destroyed

### Voxel Models

#### Mobs (Mobile Objects)
- **Player**: 8-16 voxel primitives forming humanoid
  - Named primitives: head, neck, torso, upper_arm_L/R, lower_arm_L/R, hand_L/R, upper_leg_L/R, lower_leg_L/R, foot_L/R
  - Attachment hierarchy: torso → limbs → extremities
  - Base size: ~4-6 screen pixels tall at default zoom
- **Enemies**: Procedural variations of base voxel templates
  - Humanoid: Same structure as player, different colors/proportions
  - Beast: 6-10 voxels (head, body, 4 legs, tail)
  - Flying: 4-6 voxels (body, wings, head)
  - Boss: 16-32 voxels (larger, more complex arrangements)

#### Equipment (Attached Voxel Objects)
- **Armor Pieces**: Voxel groups that replace/augment base primitives
  - Helmet: replaces/overlays head primitive (1-3 voxels)
  - Chest: replaces/overlays torso primitive (2-4 voxels)
  - Gloves, boots: replace hand/foot primitives (1-2 voxels each)
- **Weapons**: Voxel primitives attached to hand attachment point
  - Sword: 2-4 voxels (blade, hilt, guard)
  - Staff: 3-5 voxels (shaft, ornament, base)
  - Bow: 3-4 voxels (limbs, grip, string as line)
  - Axe, mace, dagger: 2-5 voxels each

#### Items (World Objects)
- **Loot drops**: Small voxel objects on ground
  - Health potion: 1-2 voxels (bottle shape, red)
  - Mana potion: 1-2 voxels (bottle shape, blue)
  - Gold: 1 voxel (yellow, sparkle shader)
  - Weapons/Armor: Same models as equipment, lying on ground
- **Interactables**:
  - Chest: 4-8 voxels (box with lid)
  - Lever: 2-3 voxels
  - Door: 6-12 voxels (can animate open/close)

#### Environment (Static Voxel Objects)
- **Floor tiles**: Voxel grid (can be single layer or multi-layer for depth)
  - Grass: green voxel layer with variation
  - Stone: gray voxel tiles
  - Lava: red/orange animated voxels
- **Walls**: Stacked voxels forming barriers
  - Simple wall: 1 voxel wide, 3-4 voxels tall
  - Thick wall: 2-3 voxels wide
- **Props**: Decorative voxel objects
  - Tree: 5-10 voxels (trunk + canopy)
  - Rock: 2-4 voxels (irregular arrangement)
  - Barrel: 3-5 voxels (cylinder shape)
  - Pillar: 4-8 voxels (vertical stack with details)

### Effects (Non-Solid Objects)
Particles are used for ephemeral effects, not solid objects:
- **Particle systems**: For magic, fire, sparks, blood splatter, smoke
- **Particle textures**: 2-3 simple shapes (circle, diamond, star)
- **Decals**: Procedurally generated or simple patterns (scorch marks, blood stains)
- **Projectiles**: Can be voxels (arrow) or particles (magic missile trail)
- **UI elements**: Simple geometric shapes with shaders

### Textures
- **No textures on voxels** - use vertex colors + lighting only
- Simple gradients for particles
- UI icons can be procedural or basic shapes
- All visual variety comes from: voxel arrangement, colors, shaders, lighting

## Key Features for Fast Gameplay

1. **Instant feedback**: Hit effects, screen shake, particles
2. **Smooth movement**: Interpolated position updates for all party members
3. **Responsive controls**: No input lag, immediate reactions, instant Tab switching
4. **Clear visuals**: Enemy telegraphing, danger zones visible, active character clearly marked
5. **Satisfying combat**: Impact sounds, visual effects, knockback
6. **Fast progression**: Quick level-ups, frequent loot drops
7. **Strategic party switching**: Seamless Tab cycling between characters mid-combat
8. **Tactical depth**: Balance controlling multiple characters with fast-paced action
9. **Smart ally AI**: Semi-autonomous allies handle basics, freeing player to focus on tactics

## Success Metrics
- Maintain 144 FPS on mid-range GPU (GTX 1060 / RX 580 equivalent)
- Render 50+ voxel-based mobs simultaneously (party + enemies) without frame drops
- Character switching latency < 16ms (instant Tab response)
- Player movement latency < 16ms
- Visual clarity maintained during intense combat (party + 30+ enemies visible on screen)
- Voxel destruction/modification with no noticeable performance impact
- Semi-autonomous AI updates at 60Hz without impacting frame rate
- Asset creation time: < 15 minutes per character class (define voxel arrangement + colors)
- Asset creation time: < 10 minutes per enemy type
- Procedural armor generation: < 5 minutes per piece
- Build time: < 2 minutes for full compilation

## Next Steps
1. **Review and approve this plan**
2. Set up the project structure and build system
3. Implement Phase 1 (Foundation)
4. Iterate based on gameplay feel
