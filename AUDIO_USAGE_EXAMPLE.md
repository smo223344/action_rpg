# AudioManager Usage Guide

The `AudioManager` class provides a comprehensive system for loading, managing, and playing sound effects with multi-channel mixing and spatial audio support.

## Features

- **Multi-channel mixing**: Play multiple sounds simultaneously (up to 32 concurrent sources)
- **Spatial audio**: Position sounds at X,Y coordinates for immersive gameplay
- **Resource management**: Efficient loading and unloading of sound effects
- **Volume control**: Per-sound and global master volume
- **Looping support**: For background music or ambient sounds
- **Auto-cleanup**: Automatically recycles finished sound sources

## Dependencies

- **OpenAL**: Cross-platform 3D audio API
- **GLM**: For vector math (already in project)

### Installing OpenAL

**Ubuntu/Debian:**
```bash
sudo apt-get install libopenal-dev
```

**macOS:**
```bash
brew install openal-soft
```

**Windows:**
OpenAL is typically included with graphics drivers, or download from:
https://www.openal.org/downloads/

## Basic Usage

### 1. Initialize the AudioManager

```cpp
#include "audio_manager.h"

// In your Game class or main
AudioManager audioManager;

if (!audioManager.initialize()) {
    std::cerr << "Failed to initialize audio system" << std::endl;
    return false;
}
```

### 2. Load Sound Effects

```cpp
// Load various sound effects
audioManager.loadSound("explosion", "assets/sounds/explosion.wav");
audioManager.loadSound("footstep", "assets/sounds/footstep.wav");
audioManager.loadSound("sword_hit", "assets/sounds/sword_hit.wav");
audioManager.loadSound("background_music", "assets/sounds/bgm.wav");
audioManager.loadSound("coin_pickup", "assets/sounds/coin.wav");
```

### 3. Play Sounds

**Simple playback (centered, no spatial audio):**
```cpp
// Play a UI sound effect
audioManager.playSoundSimple("coin_pickup", 1.0f, false);

// Play background music (looping)
audioManager.playSoundSimple("background_music", 0.3f, true);
```

**Spatial audio (positioned at X,Y coordinates):**
```cpp
// Play an explosion at screen position (500, 300)
audioManager.playSound("explosion", glm::vec2(500.0f, 300.0f), 1.0f, false);

// Play footstep at player's position
glm::vec2 playerPos = player.getPosition();
audioManager.playSound("footstep", playerPos, 0.5f, false);
```

### 4. Update Each Frame

```cpp
// In your game loop
void Game::update(float deltaTime) {
    // Update audio system (cleans up finished sounds)
    audioManager.update();

    // Update listener position to follow camera/player
    audioManager.setListenerPosition(camera.getPosition());

    // ... rest of game update logic
}
```

### 5. Cleanup

```cpp
// When shutting down
audioManager.shutdown();
```

## Advanced Usage

### Controlling Individual Sounds

```cpp
// Keep the source ID when playing a sound
ALuint sourceId = audioManager.playSound("background_music",
                                         glm::vec2(0, 0),
                                         0.5f,
                                         true);

// Later, control this specific sound
audioManager.pauseSound(sourceId);
audioManager.resumeSound(sourceId);
audioManager.setSoundVolume(sourceId, 0.8f);
audioManager.stopSound(sourceId);
```

### Master Volume Control

```cpp
// Set global volume (affects all sounds)
audioManager.setMasterVolume(0.7f);

// Get current master volume
float volume = audioManager.getMasterVolume();
```

### Stop All Sounds

```cpp
// Stop everything (useful for pausing or level transitions)
audioManager.stopAllSounds();
```

### Adjust Spatial Audio Scale

```cpp
// Adjust how screen coordinates map to audio space
// Default is 0.01f (1 screen unit = 0.01 audio units)
// Increase for more dramatic distance falloff
audioManager.setPositionScale(0.02f);
```

## Integration Example: Adding to Game Class

```cpp
// game.h
#include "audio_manager.h"

class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void run();
    void shutdown();

private:
    void update(float deltaTime);
    void render();
    void handleInput();

    // ... other members
    AudioManager m_audioManager;
};

// game.cpp
bool Game::initialize() {
    // ... existing initialization

    // Initialize audio
    if (!m_audioManager.initialize()) {
        std::cerr << "Warning: Audio system failed to initialize" << std::endl;
        // Continue anyway - game can work without audio
    }

    // Load game sounds
    m_audioManager.loadSound("player_jump", "assets/sounds/jump.wav");
    m_audioManager.loadSound("enemy_hit", "assets/sounds/hit.wav");
    m_audioManager.loadSound("level_music", "assets/sounds/level1.wav");

    // Start background music
    m_audioManager.playSoundSimple("level_music", 0.3f, true);

    return true;
}

void Game::update(float deltaTime) {
    // Update audio system
    m_audioManager.update();

    // Update listener to follow player
    if (m_player) {
        m_audioManager.setListenerPosition(m_player->getPosition());
    }

    // ... rest of update logic
}

void Game::shutdown() {
    m_audioManager.shutdown();
    // ... other cleanup
}
```

## Example: Playing Sound on Game Events

```cpp
// When player jumps
void Player::jump() {
    // ... jump logic
    m_game->getAudioManager().playSound("player_jump",
                                        getPosition(),
                                        0.7f,
                                        false);
}

// When enemy takes damage
void Enemy::takeDamage(int damage) {
    // ... damage logic
    m_game->getAudioManager().playSound("enemy_hit",
                                        getPosition(),
                                        1.0f,
                                        false);
}

// When collecting an item
void Game::collectItem(const glm::vec2& position) {
    // ... collection logic
    m_audioManager.playSoundSimple("coin_pickup", 1.0f, false);
}
```

## Audio File Requirements

The AudioManager supports **WAV files** with the following specifications:

- **Format**: PCM (uncompressed)
- **Channels**: Mono or Stereo
- **Sample Rate**: Any (22050 Hz, 44100 Hz, 48000 Hz recommended)
- **Bit Depth**: 8-bit or 16-bit

### Converting Audio Files to WAV

Using **FFmpeg**:
```bash
# Convert MP3 to WAV
ffmpeg -i input.mp3 -acodec pcm_s16le -ar 44100 output.wav

# Convert OGG to WAV
ffmpeg -i input.ogg -acodec pcm_s16le -ar 44100 output.wav

# Convert to mono (smaller file size, good for sound effects)
ffmpeg -i input.mp3 -acodec pcm_s16le -ar 44100 -ac 1 output.wav
```

## Performance Tips

1. **Pre-load sounds**: Load all sounds during initialization or level loading, not during gameplay
2. **Use mono for sound effects**: Mono files are smaller and work better with spatial audio
3. **Use stereo for music**: Background music benefits from stereo
4. **Limit concurrent sounds**: The system supports 32 sources, but fewer is better for performance
5. **Call update() regularly**: This cleans up finished sounds and prevents resource leaks
6. **Use playSoundSimple() for UI**: Non-spatial sounds are cheaper for interface elements

## Troubleshooting

### "Failed to open audio device"
- **Linux**: Install OpenAL: `sudo apt-get install libopenal1`
- **Windows**: Update audio drivers or install OpenAL runtime
- **macOS**: Install OpenAL: `brew install openal-soft`

### "Failed to open file"
- Check that the file path is correct
- Use absolute paths or paths relative to executable location
- Ensure the assets directory is copied to the build output

### "No available sources"
- Too many sounds playing simultaneously (limit is 32)
- Call `audioManager.update()` regularly to clean up finished sounds
- Consider stopping old sounds before playing new ones

### "Unsupported audio format"
- Only PCM WAV files are supported
- Convert compressed formats (MP3, OGG) to WAV using FFmpeg

## Future Enhancements

Potential additions to the AudioManager:

- **Compressed format support**: OGG, MP3 via external libraries
- **Audio streaming**: For large music files
- **Sound groups**: Categories with separate volume controls (music, SFX, voice)
- **3D audio**: Full 3D positioning (currently 2D on XY plane)
- **Effects**: Reverb, echo, filters
- **Fade in/out**: Smooth volume transitions

## API Reference Summary

### Initialization
- `bool initialize()` - Initialize OpenAL
- `void shutdown()` - Cleanup all resources
- `bool isInitialized()` - Check if initialized

### Loading Sounds
- `bool loadSound(name, filepath)` - Load WAV file
- `void unloadSound(name)` - Unload sound

### Playing Sounds
- `ALuint playSound(name, position, volume, loop)` - Play with spatial audio
- `ALuint playSoundSimple(name, volume, loop)` - Play centered

### Controlling Sounds
- `void stopSound(sourceId)` - Stop specific sound
- `void stopAllSounds()` - Stop all sounds
- `void pauseSound(sourceId)` - Pause sound
- `void resumeSound(sourceId)` - Resume paused sound
- `void setSoundVolume(sourceId, volume)` - Set sound volume

### Global Settings
- `void setMasterVolume(volume)` - Set global volume
- `float getMasterVolume()` - Get global volume
- `void setListenerPosition(position)` - Set listener position
- `void setPositionScale(scale)` - Set coordinate scaling

### Per-Frame
- `void update()` - Cleanup finished sounds (call each frame)
