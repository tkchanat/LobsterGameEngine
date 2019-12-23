#include "pch.h"
#include "AudioClip.h"

namespace Lobster {

	AudioClip::AudioClip(const char* name, float pitch, float gain, glm::vec3 pos, glm::vec3 velo, 
		bool loop) :
		m_name(name), m_pitch(pitch), m_gain(gain), m_position(pos), m_velocity(velo), m_looping(loop)
	{
		alGenSources((ALsizei)1, &m_source);
		alGenBuffers((ALsizei)1, &m_buffer);
		Update();
	}

	void AudioClip::Update() {
		alSourcef(m_source, AL_PITCH, m_pitch);
		alSourcef(m_source, AL_GAIN, m_gain);
		alSource3f(m_source, AL_POSITION, m_position[0], m_position[1], m_position[2]);
		alSource3f(m_source, AL_VELOCITY, m_velocity[0], m_velocity[1], m_velocity[2]);
		alSourcei(m_source, AL_LOOPING, m_looping);
	}

	void AudioClip::SetPitch(float pitch) {
		// perform clipping
		if (pitch < 0.f) pitch = 0.f;
		else if (pitch > 2.f) pitch = 1.f;
		alSourcef(m_source, AL_PITCH, pitch);
		m_pitch = pitch;		
	}

	void AudioClip::SetGain(float gain) {
		// perform clipping
		if (gain < 0.f) gain = 0.f;
		else if (gain > 1.f) gain = 1.f;
		alSourcef(m_source, AL_GAIN, gain);
		m_gain = gain;
	}

	void AudioClip::SetPosition(glm::vec3 pos) {
		alSource3f(m_source, AL_POSITION, pos[0], pos[1], pos[2]);
		m_position = pos;
	}

	void AudioClip::SetVelocity(glm::vec3 velo) {
		alSource3f(m_source, AL_VELOCITY, velo[0], velo[1], velo[2]);
		m_velocity = velo;
	}

	void AudioClip::SetLooping(bool looping) {
		alSourcef(m_source, AL_LOOPING, looping);
		m_looping = looping;
	}

	void AudioClip::BindBuffer(ALenum format, ALvoid* data, ALsizei size, ALsizei freq) {
		alBufferData(m_buffer, format, data, size, freq);
		alSourcei(m_source, AL_BUFFER, m_buffer);
	}	

	void AudioClip::Play() {
		alSourcePlay(m_source);
		alGetSourcei(m_source, AL_SOURCE_STATE, &m_sourceState);
		while (m_sourceState == AL_PLAYING) {
			alGetSourcei(m_source, AL_SOURCE_STATE, &m_sourceState);
		}		
	}

	void AudioClip::Pause() {
		alSourcePause(m_source);
	}

	void AudioClip::Stop() {
		alSourceStop(m_source);
	}
}