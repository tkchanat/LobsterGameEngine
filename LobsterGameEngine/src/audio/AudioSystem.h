#pragma once
#include <al.h>
#include <alc.h>

namespace Lobster
{

	class AudioSystem
	{
	private:
		ALCcontext* m_context;
		ALCdevice* m_device;
		static AudioSystem* s_instance;
	public:
		~AudioSystem();
		static void Initialize();
		static void ListAllDevices(const ALCchar* devices);
	private:
		AudioSystem();
		static void LoadWAVFile(const char * path, ALsizei * format, ALvoid ** data, ALsizei * size, ALsizei * freq);
	};

}