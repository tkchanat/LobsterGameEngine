#pragma once
#include "Component.h"

namespace Lobster
{

	class Material;
	class VertexArray;
	class VertexBuffer;
	class Texture2D;

	class ParticleComponent : public Component
	{
	private:
		bool b_animated;
		int m_particleCount;
		float m_particleSize;
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
	private:
		inline float RandomNumber() const;
	};

}