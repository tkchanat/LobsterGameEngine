#include "pch.h"
#include "AudioComponent.h"

namespace Lobster {
	// ========= Members of AudioSource ==========
	AudioSource::AudioSource() {

	}

	void AudioSource::SetSource(AudioClip* source) {
		m_source = source;
	}

	void AudioSource::OnUpdate(double deltaTime) {

	}

	void AudioSource::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginCombo("Audio Clip", (m_source ? m_source->GetName().c_str() : "None"))) {
				for (AudioClip* audioClip : AudioSystem::GetAudioList()) {
					if (ImGui::Selectable(audioClip->GetName().c_str(), audioClip == m_source))
						m_source = audioClip;
				}
				ImGui::EndCombo();
			}			
			ImGui::Checkbox("Mute", &m_muted);
			if (ImGui::SliderFloat("Gain", &m_gain, 0.0f, 1.0f))
				if (m_source) m_source->SetGain(m_gain);
			if (ImGui::SliderFloat("Pitch", &m_pitch, 0.5f, 2.0f))
				if (m_source) m_source->SetPitch(m_pitch);
			if (ImGui::TreeNode("3D Sound Setting")) {				
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f); // reduce width
				ImGui::Checkbox("Enable", &m_enable3d);
				const char* rolloff[] = { "Linear", "Inverse Square", "Logarithmic" };
				ImGui::Combo("Volume Rolloff", &m_rolloff, rolloff, IM_ARRAYSIZE(rolloff));
				ImGui::InputFloat("Min Distance", &m_minDistance);
				ImGui::InputFloat("Max Distance", &m_maxDistance);
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}
		}
	}


	// ========= Members of AudioListener ==========
	AudioListener::AudioListener() {

	}

	void AudioListener::OnUpdate(double deltaTime) {

	}

	void AudioListener::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Hi I am listening");
		}
	}

}