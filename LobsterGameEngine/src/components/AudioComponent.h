#pragma once
#include "Component.h"
#include "audio/AudioSystem.h"
#include "audio/AudioClip.h"
#include "objects/GameObject.h"

namespace Lobster {
	
	class AudioSource : public Component {		
		friend class AudioListener;
	public:
		static std::vector<AudioSource*> sourceList;
	private:
		static const char* rolloff[];
		int m_isChanging = -1;		//	Used for undo system. 0 = gain, 1 = pitch, 2 = minDistance, 3 = maxDistance.
		float m_prevProp[4];		//	Used for undo system.

		// normally this is not set until OnBegin(), to specify a file to load,
		// set the name of the audio clip in m_clipName
		AudioClip* m_clip = nullptr;
		std::string m_clipName = "None";// to record the audio name for further loading
		// basic setting
		bool m_open = true;				// remove the component when this is false
		bool m_muted = false;
		float m_gain = 1.0;
		float m_pitch = 1.0;		
		// 3d sound setting
		bool m_enable3d = true;
		float m_minDistance = 1.f;		// the distance starting to weaken volume
		float m_maxDistance = 100.f;	// the distance with zero volume
		VolumeRolloff m_rolloff = INVERSE_SQUARE;
		// parameters to keep track of
		glm::vec3 position;
	private:
		void SetMinMaxDistance();
	public:
		AudioSource();
		virtual ~AudioSource();
		void SetSource(AudioClip* ac);
		glm::vec3 GetPosition();		
		virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
		virtual void OnBegin();
		virtual void OnEnd();
	};

	class AudioListener : public Component {
		bool m_open = true;		// Remove the component when this is false
		glm::vec3 m_position;
	public:
		AudioListener();
		virtual ~AudioListener() = default;
		virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
	};

}