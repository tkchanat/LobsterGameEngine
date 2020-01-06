#pragma once
#include <al.h>
#include <alc.h>
#include "audio/AudioClip.h"

namespace Lobster
{

	enum class AudioType {
		UNKNOWN, WAV, OGG, MP3, AUDIOTYPE_SIZE
	};

	enum VolumeRolloff { LINEAR, INVERSE_SQUARE, EXPONENTIAL };

	class AudioSystem
	{
	private:
		ALCcontext* m_context;
		ALCdevice* m_device;
		static AudioSystem* s_instance;
		std::vector<AudioClip*> m_audioClips;
	public:
		~AudioSystem();
		static void Initialize();
		static void ListAllDevices(const ALCchar* devices);
		static std::vector<AudioClip*>& GetAudioList();
		static AudioClip* AddAudioClip(const char* file, AudioType type = AudioType::UNKNOWN);
		static void RemoveAudioClip(AudioClip* target);
		static void RemoveAudioClip(std::string name);
		static void SetRolloffType(VolumeRolloff type);
		
	private:
		AudioSystem();
		static void LoadWAVFile(const char * path, ALsizei * format, ALvoid ** data, ALsizei * size, ALsizei * freq);
	};

}