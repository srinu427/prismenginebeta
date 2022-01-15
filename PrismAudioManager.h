#pragma once
#include <iostream>
#include <unordered_map>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct wav_data {
	uint32_t size;
	uint32_t chunkSize;
	short formatType;
	short channels;
	uint32_t sampleRate;
	uint32_t avgBytesPerSec;
	short bytesPerSample;
	short bitsPerSample;
	uint32_t dataSize;
	uint8_t* data;
};

class PrismAudioManager
{
public:
	PrismAudioManager(uint32_t max_audio_buffers = 50, uint32_t max_sound_sources = 10);
	~PrismAudioManager();

	void add_aud_source(std::string id);
	void update_aud_source(std::string id, glm::vec3 pos, glm::vec3 vel);
	void play_aud_buffer_from_source(std::string source_id, std::string buffer_id, bool loop=false);
	void add_aud_buffer(std::string id, std::string wavfname);
	void update_listener(glm::vec3 pos, glm::vec3 vel, glm::vec3 ldir, glm::vec3 lup);
private:
	ALCdevice* adevice;
	ALCcontext* acontext;
	uint32_t MAX_AUD_BUFFS;
	uint32_t MAX_AUD_SOURCES;
	std::unordered_map<std::string, ALuint> aud_sources;
	std::unordered_map<std::string, ALuint> aud_buffers;
};

