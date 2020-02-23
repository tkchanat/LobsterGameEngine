#pragma once
#include <al.h>
#include <alc.h>

namespace Lobster {	

	class AudioClip {
	private:
		// to track the source and buffer of this audio clip in OpenAL
		ALuint m_source;
		ALuint m_buffer;
		ALint m_sourceState;

		std::string m_name;		
		float m_pitch;
		float m_gain;
		bool m_mute = false;
		bool m_looping;
	public:
		AudioClip(const char* name, float pitch=1.f, float gain=1.f, glm::vec3 pos=glm::vec3(0, 0, 0),
			glm::vec3 velo=glm::vec3(0, 0, 0), bool loop=false);
		~AudioClip();
		void alUpdate();
		// Bind the data loaded from AudioSystem into into buffer
		void BindBuffer(ALenum format, ALvoid* data, ALsizei size, ALsizei freq);
		// Just to play the sound directly
		void Play();
		// Play the sound, and run callback when finished. This function will be passed to ThreadPool.
		void Play(const std::function<void()>& callback);
		// Pause/Stop any audio playing
		void Pause();
		void Stop();

		// Getters
		inline std::string GetName() { return m_name; }
		inline float GetPitch() { return m_pitch; }
		inline float GetGain() { return m_gain; }
		inline bool GetLooping() { return m_looping; }
		inline ALuint GetSource() { return m_source; }
		inline ALint GetSourceState() { 
			alGetSourcei(m_source, AL_SOURCE_STATE, &m_sourceState);
			return m_sourceState; 
		}
		// Setters
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetLooping(bool loop);
		void Mute(bool mute=true);
	};
}