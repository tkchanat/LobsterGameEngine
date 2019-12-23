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
		glm::vec3 m_position;
		glm::vec3 m_velocity;
		bool m_looping;
	public:
		AudioClip(const char* name, float pitch=1.f, float gain=1.f, glm::vec3 pos=glm::vec3(0, 0, 0),
			glm::vec3 velo=glm::vec3(0, 0, 0), bool loop=false);
		void Update();
		// Bind the data loaded from AudioSystem into into buffer
		void BindBuffer(ALenum format, ALvoid* data, ALsizei size, ALsizei freq);
		// Play the loaded audio. Must be run in a separate thread.
		void Play();
		// Pause/Stop any audio playing
		void Pause();
		void Stop();

		// Getters
		inline std::string GetName() { return m_name; }
		inline float GetPitch() { return m_pitch; }
		inline float GetGain() { return m_gain; }
		inline glm::vec3 GetPosition() { return m_position; }
		inline glm::vec3 GetVelocity() { return m_velocity; }
		inline bool GetLooping() { return m_looping; }
		inline ALint GetSourceState() { return m_sourceState; }
		// Setters
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetPosition(glm::vec3 pos);
		void SetVelocity(glm::vec3 velo);
		void SetLooping(bool loop);
	};
}