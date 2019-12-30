#pragma once
#include "Component.h"
#include "audio/AudioSystem.h"
#include "audio/AudioClip.h"
#include "objects/GameObject.h"

namespace Lobster {

	enum VolumeRolloff { LINEAR, INVERSE_SQUARE, LOGARITHMIC };
	
	class AudioSource : public Component {
	private:		
		AudioClip* m_source = nullptr;
		// basic setting
		bool m_open = true;		// Remove the component when this is false
		bool m_muted = false;
		float m_gain = 1.0;
		float m_pitch = 1.0;		
		// 3d sound setting
		bool m_enable3d = true;
		float m_minDistance = 1.f;	// the distance starting to weaken volume
		float m_maxDistance = 100.f;	// the distance with zero volume
		int m_rolloff = LOGARITHMIC;
	public:
		AudioSource();
		void SetSource(AudioClip* source);
		virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
	};

	class AudioListener : public Component {
		bool m_open = true;		// Remove the component when this is false
	public:
		AudioListener();
		virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
	};

}