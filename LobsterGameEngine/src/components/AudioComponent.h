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
		std::string m_clipName = "None"; // to record the audio name for further loading
		bool done = false;
		// basic setting
		bool m_autoPlay = false;
		bool m_loop = true;		
		bool m_muted = false;
		float m_gain = 1.0;
		float m_pitch = 1.0;		
		// 3d sound setting
		bool m_enable3d = true;
		float m_minDistance = 1.f;		// the distance starting to weaken volume
		float m_maxDistance = 100.f;	// the distance with zero volume
		VolumeRolloff m_rolloff = INVERSE_SQUARE;
	private:
		void SetMinMaxDistance();
	public:
		AudioSource();
		virtual ~AudioSource();		
		inline bool IsAutoPlay() { return m_autoPlay; }
		void SetSource(AudioClip* ac);
		glm::vec3 GetPosition();				
		virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
		virtual void OnBegin();
		virtual void OnEnd();
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
		// functions provided for Lua call
		void Play(); // only works when clip is loaded
		void Stop(); // only works when clip is loaded
		void Mute();
		void Unmute();
		void SetGain(float gain);
		void SetPitch(float pitch);
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			ar(m_autoPlay);
			ar(m_clipName);
			ar(m_muted);
			ar(m_gain);
			ar(m_pitch);
			ar(m_enable3d);
			ar(m_minDistance, m_maxDistance);
			ar(m_rolloff);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			ar(m_autoPlay);
			ar(m_clipName);
			ar(m_muted);
			ar(m_gain);
			ar(m_pitch);
			ar(m_enable3d);
			ar(m_minDistance, m_maxDistance);
			ar(reinterpret_cast<VolumeRolloff>(m_rolloff));
		}
	};

	class AudioListener : public Component {
	public:
		AudioListener();
		virtual ~AudioListener() = default;
		virtual void OnBegin() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
		}
	};

}