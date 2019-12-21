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

		// Configuring Listener
		// TODO: create an AudioListen class in one of two ways,
		// 1) make it as a component and attach the listener to a GameObject
		// 2) assume all cameras are listeners, no need to make it into a component
		ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		alListener3f(AL_POSITION, 0, 0, 1.0f);
		alListener3f(AL_VELOCITY, 0, 0, 0);
		alListenerfv(AL_ORIENTATION, listenerOri);

		// Source Generation
		// Note: keep every data stored in main thread
		// TODO: make a AudioClip class managing the source and audio buffer objects
		ALuint source;
		ALuint buffer;
		alGenSources((ALuint)1, &source);
		alGenBuffers(1, &buffer);

		alSourcef(source, AL_PITCH, 1);
		alSourcef(source, AL_GAIN, 1);
		alSource3f(source, AL_POSITION, 0, 0, 0);
		alSource3f(source, AL_VELOCITY, 0, 0, 0);
		alSourcei(source, AL_LOOPING, AL_FALSE);

		// Audio Loading
		// Note: do not ever load the audio when before it's played (loading takes time), pre-load helps
		// TODO: audio loading can be done in AudioSystem initialization stage, load all necessary AudioClip into memory
		ALsizei size, freq;
		ALenum format;
		ALvoid *data;
		ALboolean loop = AL_FALSE;
		LoadWAVFile(FileSystem::Path("audio/test.wav").c_str(), &format, &data, &size, &freq);

		// Bind to Buffer
		// Note: just like loading a CD into a CD player
		alBufferData(buffer, format, data, size, freq);
		alSourcei(source, AL_BUFFER, buffer);

		// Play the Audio
		// Note: the following code must be executed in a separated thread. otherwise, the playback will block the main thread execution
		ThreadPool::Enqueue([source, buffer]() {
			// This job should be running in a separate thread without blocking the main thread
			ALint source_state;
			alSourcePlay(source);
			alGetSourcei(source, AL_SOURCE_STATE, &source_state);
			while (source_state == AL_PLAYING) {
				alGetSourcei(source, AL_SOURCE_STATE, &source_state);
			}
		});

		// Release memory
		// Note: do not clean up immediately, since another thread is using this data to play audio
		// TODO: put these lines in AudioClip's destructor
		//alDeleteSources(1, &source);
		//alDeleteBuffers(1, &buffer);

		INFO("AudioSystem initialized!");
	}

	AudioSystem::~AudioSystem()
	{
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