#include "PrismAudioManager.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>


// WARNING: NEED TO FREE wav_data.data MANUALLY
wav_data parse_wav_file(std::string ifname) {
	std::ifstream ifile(ifname, std::ios::in | std::ios::binary);
	if (!ifile.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	char type[4];
	wav_data outdata;

	ifile.read(type, 4 * sizeof(char));
	ifile.read(reinterpret_cast<char*>(&outdata.size), 1 * sizeof(uint32_t));
	ifile.read(type, 4 * sizeof(char));
	ifile.read(type, 4 * sizeof(char));
	ifile.read(reinterpret_cast<char*>(&outdata.chunkSize), 1 * sizeof(uint32_t));
	ifile.read(reinterpret_cast<char*>(&outdata.formatType), 1 * sizeof(short));
	ifile.read(reinterpret_cast<char*>(&outdata.channels), 1 * sizeof(short));
	ifile.read(reinterpret_cast<char*>(&outdata.sampleRate), 1 * sizeof(uint32_t));
	ifile.read(reinterpret_cast<char*>(&outdata.avgBytesPerSec), 1 * sizeof(uint32_t));
	ifile.read(reinterpret_cast<char*>(&outdata.bytesPerSample), 1 * sizeof(short));
	ifile.read(reinterpret_cast<char*>(&outdata.bitsPerSample), 1 * sizeof(short));
	ifile.read(type, 4 * sizeof(char));
	ifile.read(reinterpret_cast<char*>(&outdata.dataSize), 1 * sizeof(uint32_t));

	outdata.data = (uint8_t*)malloc((outdata.dataSize) * sizeof(uint8_t));

	ifile.read(reinterpret_cast<char*>(outdata.data), outdata.dataSize * sizeof(uint8_t));

	return outdata;
}

void load_wavfile_to_buffer(std::string ifname, ALuint sbuffer) {
	wav_data file_data = parse_wav_file(ifname);
	ALuint frequency = file_data.sampleRate;
	ALenum format = 0;
	if (file_data.bitsPerSample == 8)
	{
		if (file_data.channels == 1)
			format = AL_FORMAT_MONO8;
		else if (file_data.channels == 2)
			format = AL_FORMAT_STEREO8;
	}
	else if (file_data.bitsPerSample == 16)
	{
		if (file_data.channels == 1)
			format = AL_FORMAT_MONO16;
		else if (file_data.channels == 2)
			format = AL_FORMAT_STEREO16;
	}
	alBufferData(sbuffer, format, file_data.data, file_data.dataSize, frequency);
	free(file_data.data);
}

PrismAudioManager::PrismAudioManager(uint32_t max_audio_buffers, uint32_t max_sound_sources)
{
	MAX_AUD_BUFFS = max_audio_buffers;
	MAX_AUD_SOURCES = max_sound_sources;

	adevice = alcOpenDevice(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
	if (!adevice) {
		std::runtime_error("error creating audio context");
	}
	acontext = alcCreateContext(adevice, NULL);
	alcMakeContextCurrent(acontext);

	bool g_bEAX = alIsExtensionPresent("EAX2.0");
	alGetError();
}

void PrismAudioManager::add_aud_source(std::string id)
{
	if (aud_sources.size() < MAX_AUD_SOURCES) {
		ALuint source;
		alGenSources(1, &source);
		aud_sources[id] = source;
	}
}

void PrismAudioManager::add_aud_buffer(std::string id, std::string wavfname)
{
	if (aud_buffers.size() < MAX_AUD_BUFFS) {
		ALuint sbuffer;
		alGenBuffers(1, &sbuffer);
		load_wavfile_to_buffer(wavfname, sbuffer);
		aud_buffers[id] = sbuffer;
	}
}

void PrismAudioManager::update_aud_source(std::string id, glm::vec3 pos, glm::vec3 vel)
{
	ALuint source = aud_sources[id];
	alSourcef(source, AL_PITCH, 1.0f);
	alSourcef(source, AL_GAIN, 1.0f);
	alSourcefv(source, AL_POSITION, glm::value_ptr(pos));
	alSourcefv(source, AL_VELOCITY, glm::value_ptr(vel));
}

void PrismAudioManager::update_listener(glm::vec3 pos, glm::vec3 vel, glm::vec3 ldir, glm::vec3 lup)
{
	alListenerfv(AL_POSITION, glm::value_ptr(pos));
	alListenerfv(AL_VELOCITY, glm::value_ptr(vel));
	ALfloat orientation[] = { ldir.x, ldir.y, ldir.z, lup.x, lup.y, lup.z };
	alListenerfv(AL_ORIENTATION, orientation);
}

void PrismAudioManager::play_aud_buffer_from_source(std::string source_id, std::string buffer_id, bool loop)
{
	ALuint source = aud_sources[source_id];
	ALuint sbuffer = aud_buffers[buffer_id];
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, sbuffer);
	alSourcei(source, AL_LOOPING, (loop) ? AL_TRUE : AL_FALSE);
	alSourcePlay(source);
}

PrismAudioManager::~PrismAudioManager()
{
	for (auto it : aud_sources) alDeleteSources(1, &it.second);
	for (auto it : aud_buffers) alDeleteBuffers(1, &it.second);
	aud_sources.clear();
	aud_buffers.clear();
	alcMakeContextCurrent(NULL);
	alcDestroyContext(acontext);
	alcCloseDevice(adevice);
}