#include "audio_manager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

// WAV file header structure
struct WAVHeader {
    char riff[4];           // "RIFF"
    uint32_t fileSize;      // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmtSize;       // Format chunk size
    uint16_t audioFormat;   // Audio format (1 = PCM)
    uint16_t numChannels;   // Number of channels
    uint32_t sampleRate;    // Sample rate
    uint32_t byteRate;      // Byte rate
    uint16_t blockAlign;    // Block align
    uint16_t bitsPerSample; // Bits per sample
};

AudioManager::AudioManager()
    : m_device(nullptr)
    , m_context(nullptr)
    , m_masterVolume(1.0f)
    , m_positionScale(0.01f)
    , m_listenerPosition(0.0f, 0.0f)
    , m_initialized(false)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (m_initialized) {
        std::cerr << "AudioManager: Already initialized" << std::endl;
        return true;
    }

    // Open the default audio device
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        std::cerr << "AudioManager: Failed to open audio device" << std::endl;
        return false;
    }

    // Create audio context
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        std::cerr << "AudioManager: Failed to create audio context" << std::endl;
        alcCloseDevice(m_device);
        m_device = nullptr;
        return false;
    }

    // Make context current
    if (!alcMakeContextCurrent(m_context)) {
        std::cerr << "AudioManager: Failed to make context current" << std::endl;
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_context = nullptr;
        m_device = nullptr;
        return false;
    }

    // Set up listener (default position and orientation)
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f,  // Forward
                              0.0f, 1.0f, 0.0f };  // Up
    alListenerfv(AL_ORIENTATION, listenerOri);

    // Set master volume
    alListenerf(AL_GAIN, m_masterVolume);

    // Pre-allocate some sources for the pool
    for (size_t i = 0; i < MAX_SOURCES / 2; ++i) {
        ALuint source;
        alGenSources(1, &source);
        if (alGetError() == AL_NO_ERROR) {
            m_availableSources.push_back(source);
        }
    }

    m_initialized = true;
    std::cout << "AudioManager: Initialized successfully with "
              << m_availableSources.size() << " pre-allocated sources" << std::endl;

    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Stop all sounds
    stopAllSounds();

    // Delete all active sources
    for (ALuint source : m_activeSources) {
        alDeleteSources(1, &source);
    }
    m_activeSources.clear();

    // Delete all available sources
    for (ALuint source : m_availableSources) {
        alDeleteSources(1, &source);
    }
    m_availableSources.clear();

    // Delete all sound buffers
    for (auto& pair : m_soundBuffers) {
        alDeleteBuffers(1, &pair.second.bufferId);
    }
    m_soundBuffers.clear();

    // Cleanup OpenAL
    if (m_context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }

    if (m_device) {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }

    m_initialized = false;
    std::cout << "AudioManager: Shutdown complete" << std::endl;
}

bool AudioManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Not initialized" << std::endl;
        return false;
    }

    // Check if already loaded
    if (m_soundBuffers.find(name) != m_soundBuffers.end()) {
        std::cout << "AudioManager: Sound '" << name << "' already loaded" << std::endl;
        return true;
    }

    // Generate buffer
    ALuint bufferId;
    alGenBuffers(1, &bufferId);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to generate buffer for '" << name << "'" << std::endl;
        return false;
    }

    // Load WAV file
    if (!loadWAV(filepath, bufferId)) {
        alDeleteBuffers(1, &bufferId);
        return false;
    }

    // Store in map
    m_soundBuffers[name] = { bufferId, filepath };
    std::cout << "AudioManager: Loaded sound '" << name << "' from '" << filepath << "'" << std::endl;

    return true;
}

void AudioManager::unloadSound(const std::string& name) {
    auto it = m_soundBuffers.find(name);
    if (it != m_soundBuffers.end()) {
        alDeleteBuffers(1, &it->second.bufferId);
        m_soundBuffers.erase(it);
        std::cout << "AudioManager: Unloaded sound '" << name << "'" << std::endl;
    }
}

ALuint AudioManager::playSound(const std::string& name,
                               const glm::vec2& position,
                               float volume,
                               bool loop) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Not initialized" << std::endl;
        return 0;
    }

    // Find the sound buffer
    auto it = m_soundBuffers.find(name);
    if (it == m_soundBuffers.end()) {
        std::cerr << "AudioManager: Sound '" << name << "' not loaded" << std::endl;
        return 0;
    }

    // Get an available source
    ALuint source = getAvailableSource();
    if (source == 0) {
        std::cerr << "AudioManager: No available sources to play sound" << std::endl;
        return 0;
    }

    // Configure source
    alSourcei(source, AL_BUFFER, it->second.bufferId);
    alSourcef(source, AL_GAIN, volume * m_masterVolume);
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);

    // Set 3D position
    glm::vec3 pos3D = convert2DTo3D(position);
    alSource3f(source, AL_POSITION, pos3D.x, pos3D.y, pos3D.z);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    // Set attenuation (how sound fades with distance)
    alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(source, AL_MAX_DISTANCE, 100.0f);
    alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);

    // Play the sound
    alSourcePlay(source);

    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to play sound '" << name << "'" << std::endl;
        returnSource(source);
        return 0;
    }

    return source;
}

ALuint AudioManager::playSoundSimple(const std::string& name, float volume, bool loop) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Not initialized" << std::endl;
        return 0;
    }

    // Find the sound buffer
    auto it = m_soundBuffers.find(name);
    if (it == m_soundBuffers.end()) {
        std::cerr << "AudioManager: Sound '" << name << "' not loaded" << std::endl;
        return 0;
    }

    // Get an available source
    ALuint source = getAvailableSource();
    if (source == 0) {
        std::cerr << "AudioManager: No available sources to play sound" << std::endl;
        return 0;
    }

    // Configure source (non-spatial)
    alSourcei(source, AL_BUFFER, it->second.bufferId);
    alSourcef(source, AL_GAIN, volume * m_masterVolume);
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);  // Relative to listener
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Play the sound
    alSourcePlay(source);

    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to play sound '" << name << "'" << std::endl;
        returnSource(source);
        return 0;
    }

    return source;
}

void AudioManager::stopSound(ALuint sourceId) {
    if (sourceId == 0) return;

    alSourceStop(sourceId);
    returnSource(sourceId);
}

void AudioManager::stopAllSounds() {
    for (ALuint source : m_activeSources) {
        alSourceStop(source);
    }

    // Move all active sources back to available pool
    m_availableSources.insert(m_availableSources.end(),
                             m_activeSources.begin(),
                             m_activeSources.end());
    m_activeSources.clear();
}

void AudioManager::pauseSound(ALuint sourceId) {
    if (sourceId == 0) return;
    alSourcePause(sourceId);
}

void AudioManager::resumeSound(ALuint sourceId) {
    if (sourceId == 0) return;

    ALint state;
    alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) {
        alSourcePlay(sourceId);
    }
}

void AudioManager::setSoundVolume(ALuint sourceId, float volume) {
    if (sourceId == 0) return;
    alSourcef(sourceId, AL_GAIN, volume * m_masterVolume);
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    alListenerf(AL_GAIN, m_masterVolume);
}

void AudioManager::setListenerPosition(const glm::vec2& position) {
    m_listenerPosition = position;
    glm::vec3 pos3D = convert2DTo3D(position);
    alListener3f(AL_POSITION, pos3D.x, pos3D.y, pos3D.z);
}

void AudioManager::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
    ALfloat listenerOri[] = { forward.x, forward.y, forward.z,
                              up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, listenerOri);
}

void AudioManager::update() {
    if (!m_initialized) return;

    // Check active sources and return finished ones to the pool
    auto it = m_activeSources.begin();
    while (it != m_activeSources.end()) {
        ALint state;
        alGetSourcei(*it, AL_SOURCE_STATE, &state);

        if (state == AL_STOPPED) {
            // Source has finished playing
            m_availableSources.push_back(*it);
            it = m_activeSources.erase(it);
        } else {
            ++it;
        }
    }
}

ALuint AudioManager::getAvailableSource() {
    // Try to get from available pool
    if (!m_availableSources.empty()) {
        ALuint source = m_availableSources.back();
        m_availableSources.pop_back();
        m_activeSources.push_back(source);
        return source;
    }

    // Try to create a new source if under limit
    if (m_activeSources.size() + m_availableSources.size() < MAX_SOURCES) {
        ALuint source;
        alGenSources(1, &source);
        if (alGetError() == AL_NO_ERROR) {
            m_activeSources.push_back(source);
            return source;
        }
    }

    // No sources available
    return 0;
}

void AudioManager::returnSource(ALuint sourceId) {
    // Remove from active sources
    auto it = std::find(m_activeSources.begin(), m_activeSources.end(), sourceId);
    if (it != m_activeSources.end()) {
        m_activeSources.erase(it);
    }

    // Reset source state
    alSourcei(sourceId, AL_BUFFER, 0);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);
    alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_FALSE);

    // Add back to available pool
    m_availableSources.push_back(sourceId);
}

bool AudioManager::loadWAV(const std::string& filepath, ALuint& bufferId) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "AudioManager: Failed to open file '" << filepath << "'" << std::endl;
        return false;
    }

    // Read WAV header
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    // Validate RIFF header
    if (std::strncmp(header.riff, "RIFF", 4) != 0 ||
        std::strncmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "AudioManager: Invalid WAV file (bad RIFF/WAVE header): '"
                  << filepath << "'" << std::endl;
        return false;
    }

    // Validate format
    if (std::strncmp(header.fmt, "fmt ", 4) != 0) {
        std::cerr << "AudioManager: Invalid WAV file (bad fmt header): '"
                  << filepath << "'" << std::endl;
        return false;
    }

    // Only support PCM format
    if (header.audioFormat != 1) {
        std::cerr << "AudioManager: Unsupported audio format (only PCM supported): '"
                  << filepath << "'" << std::endl;
        return false;
    }

    // Skip any extra format bytes
    if (header.fmtSize > 16) {
        file.seekg(header.fmtSize - 16, std::ios::cur);
    }

    // Find data chunk
    char chunkId[4];
    uint32_t chunkSize;
    bool foundData = false;

    while (file.read(chunkId, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), sizeof(uint32_t));

        if (std::strncmp(chunkId, "data", 4) == 0) {
            foundData = true;
            break;
        } else {
            // Skip this chunk
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (!foundData) {
        std::cerr << "AudioManager: No data chunk found in WAV file: '"
                  << filepath << "'" << std::endl;
        return false;
    }

    // Read audio data
    std::vector<char> audioData(chunkSize);
    file.read(audioData.data(), chunkSize);

    if (!file) {
        std::cerr << "AudioManager: Failed to read audio data from: '"
                  << filepath << "'" << std::endl;
        return false;
    }

    file.close();

    // Determine OpenAL format
    ALenum format;
    if (header.numChannels == 1) {
        if (header.bitsPerSample == 8) {
            format = AL_FORMAT_MONO8;
        } else if (header.bitsPerSample == 16) {
            format = AL_FORMAT_MONO16;
        } else {
            std::cerr << "AudioManager: Unsupported bits per sample: "
                      << header.bitsPerSample << std::endl;
            return false;
        }
    } else if (header.numChannels == 2) {
        if (header.bitsPerSample == 8) {
            format = AL_FORMAT_STEREO8;
        } else if (header.bitsPerSample == 16) {
            format = AL_FORMAT_STEREO16;
        } else {
            std::cerr << "AudioManager: Unsupported bits per sample: "
                      << header.bitsPerSample << std::endl;
            return false;
        }
    } else {
        std::cerr << "AudioManager: Unsupported number of channels: "
                  << header.numChannels << std::endl;
        return false;
    }

    // Upload to OpenAL buffer
    alBufferData(bufferId, format, audioData.data(), chunkSize, header.sampleRate);

    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to upload audio data to buffer" << std::endl;
        return false;
    }

    std::cout << "AudioManager: Loaded WAV - "
              << header.numChannels << " channels, "
              << header.bitsPerSample << " bits, "
              << header.sampleRate << " Hz, "
              << chunkSize << " bytes" << std::endl;

    return true;
}

glm::vec3 AudioManager::convert2DTo3D(const glm::vec2& position2D) const {
    // Convert 2D screen coordinates to 3D audio space
    // X stays X, Y stays Y, Z is always 0 (flat 2D plane)
    // Scale down screen coordinates to reasonable audio space
    return glm::vec3(
        (position2D.x - m_listenerPosition.x) * m_positionScale,
        (position2D.y - m_listenerPosition.y) * m_positionScale,
        0.0f
    );
}
