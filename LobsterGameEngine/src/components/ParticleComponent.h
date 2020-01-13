#pragma once
#include "Component.h"

namespace Lobster
{

	class Material;
	class VertexArray;
	class VertexBuffer;
	class Texture2D;

	enum EmitterShape : uint
	{
		BOX, CONE, SPHERE
	};

	class ParticleComponent : public Component
	{
	private:
		EmitterShape m_shape;
		bool b_animated;
		int m_particleCount;
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