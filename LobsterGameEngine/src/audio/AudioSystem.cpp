#include "pch.h"
#include "AudioSystem.h"

namespace Lobster
{

	Lobster::AudioSystem* AudioSystem::s_instance = nullptr;

	AudioSystem::AudioSystem()
	{
		// Creating OpenAL related resources
		{
			// Get OpenAL Device
			m_device = alcOpenDevice(NULL);
			if (!m_device) {
				CRITICAL("AudioSystem failed to initialize. No sound will be played.");
				return;
			}
			// Check for Enumeration Support
			ALboolean enumeration = alcIsExtensionPresent(m_device, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_TRUE) {
				ListAllDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
			}
			else {
				CRITICAL("OpenAL enumeration is not supported.");
				return;
			}
			// Creating OpenAL Context
			m_context = alcCreateContext(m_device, NULL);
			if (!alcMakeContextCurrent(m_context)) {
				CRITICAL("OpenAL context creation failed! Error code: {}", alGetError());
			}
		}

		INFO("AudioSystem initialized!");
	}

	AudioSystem::~AudioSystem()
	{
		// Clear all audio clips
		for (AudioClip* ac : m_audioClips) {
			delete ac;
		}
		// Release all OpenAL related resources
		INFO("AudioSystem shutting down...");
		m_device = alcGetContextsDevice(m_context);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(m_context);
		alcCloseDevice(m_device);
	}

	void AudioSystem::Initialize()
	{
		if (s_instance) {
			throw std::runtime_error("AudioSystem already initialized!");
			return;
		}
		s_instance = new AudioSystem();
	}

	AudioClip* AudioSystem::AddAudioClip(const char* file, AudioType type) {
		// TODO base on type, classify into different loading method
		// load the file
		ALsizei size, freq;
		ALenum format;
		ALvoid *data;
		LoadWAVFile(file, &format, &data, &size, &freq);
		// TODO check if loaded successfully
		AudioClip* ac = new AudioClip(fs::path(file).filename().string().c_str());
		ac->BindBuffer(format, data, size, freq);
		s_instance->m_audioClips.push_back(ac);
		return ac;
	}

	void AudioSystem::RemoveAudioClip(AudioClip* target) {	
		if (!target) return;
		auto it = std::remove(s_instance->m_audioClips.begin(), s_instance->m_audioClips.end(), target);
		s_instance->m_audioClips.erase(it, s_instance->m_audioClips.end());
		delete target;
	}

	void AudioSystem::RemoveAudioClip(std::string name) {
		AudioClip* tracer = nullptr;
		for (AudioClip* ac : s_instance->m_audioClips) {
			if (ac->GetName().compare(name) == 0) {
				tracer = ac;
				break;
			}
		}
		if (tracer) RemoveAudioClip(tracer);
	}

	void AudioSystem::ListAllDevices(const ALCchar * devices)
	{
		const ALCchar *device = devices, *next = devices + 1;
		size_t len = 0;

		LOG("=========================DEVICE LIST===========================");
		while (device && *device != '\0' && next && *next != '\0') {
			LOG("{}", device);
			len = strlen(device);
			device += (len + 1);
			next += (len + 2);
		}		
		LOG("===============================================================");
	}

	std::vector<AudioClip*>& AudioSystem::GetAudioList() {
		return s_instance->m_audioClips;
	}

	void AudioSystem::SetRolloffType(VolumeRolloff type) {
		switch (type) {
		case LINEAR:
			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
			break;
		case INVERSE_SQUARE:
			alDistanceModel(AL_INVERSE_DISTANCE);
			break;
		case EXPONENTIAL:
			alDistanceModel(AL_EXPONENT_DISTANCE);
			break;
		}
	}

	void AudioSystem::LoadWAVFile(const char * path, ALsizei * format, ALvoid ** data, ALsizei * size, ALsizei * freq)
	{
		struct WAVHeader {
			/* RIFF Chunk Descriptor */
			uint8_t         RIFF[4];        // RIFF Header Magic header
			uint32_t        ChunkSize;      // RIFF Chunk Size
			uint8_t         WAVE[4];        // WAVE Header
			/* "fmt" sub-chunk */
			uint8_t         fmt[4];         // FMT header
			uint32_t        Subchunk1Size;  // Size of the fmt chunk
			uint16_t        AudioFormat;    // Audio format 1=PCM,6=Mu-Law,7=A-Law,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
			uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Stereo
			uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
			uint32_t        bytesPerSec;    // bytes per second
			uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
			uint16_t        bitsPerSample;  // Number of bits per sample
		} header;

		struct DataChunk
		{
			uint8_t ID[4]; //"data" = 0x61746164
			uint32_t size;  //Chunk data bytes
		};

		// reading header
		FILE* file = fopen(path, "rb");
		if (!file) {
			throw std::runtime_error("Audio file not found!");
			return;
		}
		fread(&header, sizeof(header), 1, file);
		// reading data
		DataChunk chunk;
		while (true) {
			fread(&chunk, sizeof(chunk), 1, file);
			if (*(unsigned int *)&chunk.ID == 0x61746164)
				break;
			fseek(file, chunk.size, SEEK_CUR);
		}
		byte* value = new byte[chunk.size];
		fread(value, 1, chunk.size, file);
		fclose(file);

		// return value
		if (header.NumOfChan == 1) {
			// mono
			if (header.bitsPerSample == 8) {
				*format = AL_FORMAT_MONO8;
			}
			else if (header.bitsPerSample == 16) {
				*format = AL_FORMAT_MONO16;
			}
		}
		else if (header.NumOfChan == 2) {
			// stereo
			if (header.bitsPerSample == 8) {
				*format = AL_FORMAT_STEREO8;
			}
			else if (header.bitsPerSample == 16) {
				*format = AL_FORMAT_STEREO16;
			}
		}
		*data = (ALvoid*)value;
		*size = (ALsizei)chunk.size;
		*freq = (ALsizei)header.SamplesPerSec;
	}

}