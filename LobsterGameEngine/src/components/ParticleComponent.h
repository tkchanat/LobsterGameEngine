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
	public:
		ParticleComponent();
		virtual ~ParticleComponent() override;
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
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
			ar(m_particleCount);
			ar(m_particleSize);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			ar(m_particleCount);
			ar(m_particleSize);
		}
	};

}