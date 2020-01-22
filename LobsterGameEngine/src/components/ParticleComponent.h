#pragma once
#include "Component.h"

namespace Lobster
{

	class Material;
	class VertexArray;
	class VertexBuffer;
	class Texture2D;

	enum class EmitterShape : uint
	{
		BOX, CONE, SPHERE
	};

	class ParticleComponent : public Component
	{
	private:
		static const char* shapes[];
		int m_isChanging = -1;		//	Used for undo system. 0 = emission rate, 1 = emission angle, 2 = particle color, 3 = particle size, 4 = particle orientation.
		float m_prevProp[5];		//	Used for undo system.
		int m_isChangingColor = -1;	//	Used for undo system. 0 = changing start color, 1 = changing end color.
		glm::vec4 m_prevColor[2];	//	Used for undo system.
		Texture2D* m_prevTexture;	//	Used for undo system.

		EmitterShape m_shape;
		float _simulateElapsedTime;
		bool _volumeFilled;
		bool b_animated;
		bool b_emitOneByOne;
		float m_emissionRate;
		float m_emissionAngle;
		glm::vec4 m_colorStartTransition;
		glm::vec4 m_colorEndTransition;
		int m_particleCount;
		int m_particleCutoff;
		float m_particleSize;
		float m_particleOrientation;
		Texture2D* m_particleTexture;
		glm::vec3 m_particlePositions[MAX_PARTICLES];
		Material* m_material;
		VertexArray* m_vertexArray;
		VertexBuffer* m_vertexBuffer;
	private:
		void ResetParticleCount();
	public:
		ParticleComponent();
		virtual ~ParticleComponent() override;
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override;

		void Pause() { b_animated = true; }
		void Simulate() { b_animated = false; }

	private:
		void FillVolume();
		void BoxEmitter(double deltaTime);
		void ConeEmitter(double deltaTime);
		void SphereEmitter(double deltaTime);
		inline float RandomNumber() const;
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			ar(m_shape);
			ar(b_emitOneByOne);
			ar(m_emissionAngle);
			ar(m_emissionRate);
			ar(m_particleCount);
			ar(m_particleCutoff);
			ar(m_particleSize);
			ar(m_colorStartTransition);
			ar(m_colorEndTransition);
			ar(m_particleOrientation);
			std::string textureName = m_particleTexture == nullptr ? "" : m_particleTexture->GetName();
			ar(textureName);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			ar(reinterpret_cast<EmitterShape>(m_shape));
			ar(b_emitOneByOne);
			ar(m_emissionAngle);
			ar(m_emissionRate);
			ar(m_particleCount);
			ar(m_particleCutoff);
			ar(m_particleSize);
			ar(m_colorStartTransition);
			ar(m_colorEndTransition);
			ar(m_particleOrientation);
			std::string textureName;
			ar(textureName);
			m_particleTexture = textureName.empty() ? nullptr : TextureLibrary::Use(textureName.c_str());
		}
	};

}