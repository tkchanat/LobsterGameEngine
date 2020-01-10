#include "pch.h"
#include "AudioClip.h"

namespace Lobster {

	AudioClip::AudioClip(const char* name, float pitch, float gain, glm::vec3 pos, glm::vec3 velo, 
		bool loop) :
		m_name(name), m_pitch(pitch), m_gain(gain), m_looping(loop)
	{
		alGenSources((ALsizei)1, &m_source);
		alGenBuffers((ALsizei)1, &m_buffer);
		alUpdate();
	}

	AudioClip::~AudioClip() {
		// unbind the source
		alSourcei(m_source, AL_BUFFER, NULL);
		// remove source and buffer
		alDeleteSources(1, &m_source);
		alDeleteBuffers(1, &m_buffer);
	}

	void AudioClip::alUpdate() {
		alSourcef(m_source, AL_PITCH, m_pitch);
		alSourcef(m_source, AL_GAIN, m_gain);
		alSourcei(m_source, AL_LOOPING, m_looping);
	}

	void AudioClip::SetPitch(float pitch) {
		// perform clipping
		if (pitch < 0.f) pitch = 0.f;
		else if (pitch > 2.f) pitch = 1.f;
		m_pitch = pitch;
		alSourcef(m_source, AL_PITCH, pitch);		
	}

	void AudioClip::SetGain(float gain) {		
		// perform clipping		
		if (gain < 0.f) gain = 0.f;
		else if (gain > 1.f) gain = 1.f;		
		m_gain = gain;
		if (m_mute) { // set the variable but not to set the volume when muted
			Mute();
			return;	
		}
		alSourcef(m_source, AL_GAIN, gain);
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
	}

	void AudioClip::Play(const std::function<void()>& callback) {
		ThreadPool::Enqueue([&, callback]() {
			alSourcePlay(m_source);
			alGetSourcei(m_source, AL_SOURCE_STATE, &m_sourceState);
			while (m_sourceState == AL_PLAYING) {
				alGetSourcei(m_source, AL_SOURCE_STATE, &m_sourceState);
			}
			callback();
		});		
	}

	void AudioClip::Pause() {
		alSourcePause(m_source);
	}

	void AudioClip::Stop() {
		alSourceStop(m_source);
	}

	void AudioClip::Mute(bool mute) {
		m_mute = mute;
		if (mute)
			alSourcef(m_source, AL_GAIN, 0.f);
		else
			alSourcef(m_source, AL_GAIN, m_gain);
	}
}