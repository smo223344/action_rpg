#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>

/**
 * AudioManager - Manages loading, playing, and spatial positioning of sound effects
 *
 * Features:
 * - Multi-channel audio mixing (supports multiple simultaneous sounds)
 * - 2D spatial audio (X,Y positioning mapped to stereo/3D space)
 * - Sound effect caching and resource management
 * - Volume control per sound and globally
 * - Looping and one-shot playback
 *
 * Example Usage:
 *
 *   AudioManager audio;
 *   audio.initialize();
 *
 *   // Load sound effects
 *   audio.loadSound("explosion", "assets/sounds/explosion.wav");
 *   audio.loadSound("footstep", "assets/sounds/footstep.wav");
 *
 *   // Play a sound at a specific screen position
 *   audio.playSound("explosion", glm::vec2(100.0f, 200.0f), 1.0f, false);
 *
 *   // Play background music (looping)
 *   audio.playSound("bgm", glm::vec2(0.0f, 0.0f), 0.5f, true);
 *
 *   // Update listener position (typically the camera/player position)
 *   audio.setListenerPosition(glm::vec2(playerX, playerY));
 *
 *   audio.shutdown();
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Disable copy and assignment
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    /**
     * Initialize the OpenAL audio system
     * @return true if initialization succeeded, false otherwise
     */
    bool initialize();

    /**
     * Shutdown and cleanup all audio resources
     */
    void shutdown();

    /**
     * Load a sound effect from a WAV file
     * @param name Identifier for the sound effect
     * @param filepath Path to the WAV file
     * @return true if loading succeeded, false otherwise
     */
    bool loadSound(const std::string& name, const std::string& filepath);

    /**
     * Unload a previously loaded sound effect
     * @param name Identifier of the sound effect to unload
     */
    void unloadSound(const std::string& name);

    /**
     * Play a sound effect
     * @param name Identifier of the sound effect
     * @param position 2D position for spatial audio (screen coordinates)
     * @param volume Volume level (0.0 to 1.0)
     * @param loop Whether to loop the sound
     * @return Source ID for the playing sound (0 if failed)
     */
    ALuint playSound(const std::string& name,
                     const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                     float volume = 1.0f,
                     bool loop = false);

    /**
     * Play a sound effect without spatial positioning (centered)
     * @param name Identifier of the sound effect
     * @param volume Volume level (0.0 to 1.0)
     * @param loop Whether to loop the sound
     * @return Source ID for the playing sound (0 if failed)
     */
    ALuint playSoundSimple(const std::string& name, float volume = 1.0f, bool loop = false);

    /**
     * Stop a specific playing sound source
     * @param sourceId The source ID returned by playSound
     */
    void stopSound(ALuint sourceId);

    /**
     * Stop all currently playing sounds
     */
    void stopAllSounds();

    /**
     * Pause a specific playing sound source
     * @param sourceId The source ID returned by playSound
     */
    void pauseSound(ALuint sourceId);

    /**
     * Resume a paused sound source
     * @param sourceId The source ID returned by playSound
     */
    void resumeSound(ALuint sourceId);

    /**
     * Set the volume of a specific sound source
     * @param sourceId The source ID returned by playSound
     * @param volume Volume level (0.0 to 1.0)
     */
    void setSoundVolume(ALuint sourceId, float volume);

    /**
     * Set the global master volume
     * @param volume Volume level (0.0 to 1.0)
     */
    void setMasterVolume(float volume);

    /**
     * Get the current master volume
     * @return Current master volume (0.0 to 1.0)
     */
    float getMasterVolume() const { return m_masterVolume; }

    /**
     * Set the listener (camera/player) position for spatial audio
     * @param position 2D position in screen coordinates
     */
    void setListenerPosition(const glm::vec2& position);

    /**
     * Set the listener orientation (for 3D audio)
     * @param forward Forward direction vector
     * @param up Up direction vector
     */
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

    /**
     * Update the audio system (call each frame)
     * This cleans up finished sound sources
     */
    void update();

    /**
     * Check if the audio system is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * Set the scale factor for converting screen coordinates to audio space
     * @param scale Scale factor (default: 0.01f)
     */
    void setPositionScale(float scale) { m_positionScale = scale; }

private:
    struct SoundBuffer {
        ALuint bufferId;
        std::string filepath;
    };

    // OpenAL device and context
    ALCdevice* m_device;
    ALCcontext* m_context;

    // Sound buffers (loaded sound data)
    std::unordered_map<std::string, SoundBuffer> m_soundBuffers;

    // Active sound sources (for cleanup and management)
    std::vector<ALuint> m_activeSources;

    // Available sources pool (for reuse)
    std::vector<ALuint> m_availableSources;

    // Master volume
    float m_masterVolume;

    // Position scale for converting screen coords to 3D audio space
    float m_positionScale;

    // Listener position
    glm::vec2 m_listenerPosition;

    // Initialization flag
    bool m_initialized;

    // Maximum number of concurrent sound sources
    static constexpr size_t MAX_SOURCES = 32;

    /**
     * Get an available sound source (creates new or reuses stopped ones)
     * @return Source ID (0 if none available)
     */
    ALuint getAvailableSource();

    /**
     * Return a source to the available pool
     * @param sourceId The source to return
     */
    void returnSource(ALuint sourceId);

    /**
     * Load a WAV file into an OpenAL buffer
     * @param filepath Path to the WAV file
     * @param bufferId Output buffer ID
     * @return true if loading succeeded, false otherwise
     */
    bool loadWAV(const std::string& filepath, ALuint& bufferId);

    /**
     * Convert 2D screen position to 3D audio position
     * @param position2D 2D screen position
     * @return 3D position for OpenAL
     */
    glm::vec3 convert2DTo3D(const glm::vec2& position2D) const;
};

#endif // AUDIO_MANAGER_H
