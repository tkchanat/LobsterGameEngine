#include "pch.h"
#include "Application.h"
#include "AudioComponent.h"

namespace Lobster {
	// ========= Members of AudioSource ==========
	std::vector<AudioSource*> AudioSource::sourceList = std::vector<AudioSource*>();

	AudioSource::AudioSource() : 
		Component(AUDIO_SOURCE_COMPONENT) 
	{
		// append itself to sourceList
		sourceList.push_back(this);
	}

	AudioSource::~AudioSource() {
		// clear itself from static sourceList
		sourceList.erase(std::remove(sourceList.begin(), sourceList.end(), this), sourceList.end());		
	}

	void AudioSource::SetSource(AudioClip* ac) {
		m_clip = ac;
	}

	glm::vec3 AudioSource::GetPosition() {
		return position;
	}

	void AudioSource::OnUpdate(double deltaTime) {
		// update source position
		if (transform && m_clip && m_enable3d) {
			position = transform->WorldPosition;
			alSource3f(m_clip->GetSource(), AL_POSITION, position[0], position[1], position[2]);
		}
	}

	void AudioSource::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Audio Source", &m_open, ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginCombo("Audio Clip", m_clipName.c_str())) {
				// TODO switch to a more efficient way of display
				fs::path subdir = FileSystem::Join(FileSystem::GetCurrentWorkingDirectory(), "audio");
				for (const auto& dirEntry : fs::recursive_directory_iterator(subdir)) {					
					std::string displayName = dirEntry.path().filename().string();
					if (ImGui::Selectable(displayName.c_str(), displayName == m_clipName)) {
						m_clipName = displayName;
					}
				}
				ImGui::EndCombo();
			}			
			if (ImGui::Checkbox("Mute", &m_muted))
				if (m_clip) m_clip->Mute(m_muted);
			if (ImGui::SliderFloat("Gain", &m_gain, 0.0f, 1.0f))
				if (m_clip) m_clip->SetGain(m_gain);
			if (ImGui::SliderFloat("Pitch", &m_pitch, 0.5f, 2.0f))
				if (m_clip) m_clip->SetPitch(m_pitch);
			if (ImGui::TreeNode("3D Sound Setting")) {				
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f); // reduce width
				if (ImGui::Checkbox("Enable", &m_enable3d));
				const char* rolloff[] = { "Linear", "Inverse Square", "Exponential" };
				if (ImGui::Combo("Volume Rolloff", (int*)&m_rolloff, rolloff, IM_ARRAYSIZE(rolloff))) {
					AudioSystem::SetRolloffType(m_rolloff);
				}
				if (ImGui::InputFloat("Min Distance", &m_minDistance)) {
					SetMinMaxDistance();
				}
				if (ImGui::InputFloat("Max Distance", &m_maxDistance)) {
					SetMinMaxDistance();
				}
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}
		}
		
		// Remove the component upon the cross button click
		if (!m_open) {
			gameObject->RemoveComponent(this);
		}
	}

	void AudioSource::SetMinMaxDistance() {
		if (m_clip) {
			alSourcef(m_clip->GetSource(), AL_REFERENCE_DISTANCE, m_minDistance);
			alSourcef(m_clip->GetSource(), AL_MAX_DISTANCE, m_maxDistance);
		}
	}

	void AudioSource::OnSimulationBegin() {		
		// do nothing if no audio is selected
		if (m_clipName == "None") return;
		// load audios only when it is not loaded
		std::string path = FileSystem::Join("audio", m_clipName);
		AudioClip* ac = AudioSystem::AddAudioClip(FileSystem::Path(path).c_str());
		m_clip = ac;
	}

	void AudioSource::OnSimulationEnd() {
		// stop all audios playing
		for (AudioSource* src : sourceList) {
			if (src->m_clip)
				src->m_clip->Stop();
			// delete the audio clip as well
			delete src->m_clip;
			src->m_clip = nullptr;
		}		
	}

	// ========= Members of AudioListener ==========
	AudioListener::AudioListener() :
		Component(AUDIO_LISTENER_COMPONENT)
	{

	}

	void AudioListener::OnUpdate(double deltaTime) {
		// play audio
		if (Application::GetMode() != ApplicationMode::EDITOR) {
			for (const AudioSource* src : AudioSource::sourceList) {
				if (!src->m_clip) continue;
				if (src->m_enable3d) {
					// Note: AudioClip::position will not update itself
					// so we update it here according to the object's actual position
					
					// set the listener position of OpenAL
					alSourcei(src->m_clip->GetSource(), AL_SOURCE_RELATIVE, AL_FALSE);
					m_position = transform->WorldPosition;
					alListener3f(AL_POSITION, m_position[0], m_position[1], m_position[2]);
					
					// set the listener orientation
					float ori[6];
					ori[0] = transform->Forward()[0]; ori[1] = transform->Forward()[1]; ori[2] = -transform->Forward()[2];
					ori[3] = transform->Up()[0]; ori[4] = transform->Up()[1]; ori[5] = transform->Up()[2];
					alListenerfv(AL_ORIENTATION, ori);
				}
				else {
					alSourcei(src->m_clip->GetSource(), AL_SOURCE_RELATIVE, AL_TRUE);
					alSource3f(src->m_clip->GetSource(), AL_POSITION, 0.0f, 0.0f, 0.0f);
				}
				if (src->m_clip->GetSourceState() != AL_PLAYING)
					src->m_clip->Play();
			}
		}
	}

	void AudioListener::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Audio Listener", &m_open, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Hi I am listening");
		}

		// Remove the component upon the cross button click
		if (!m_open) {
			gameObject->RemoveComponent(this);
		}
	}	

}