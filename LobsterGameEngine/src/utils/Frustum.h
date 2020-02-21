#pragma once

namespace Lobster
{

	class Material;
	class VertexArray;
	class VertexBuffer;
	
	class Plane
	{
	private:
		glm::vec3 m_normal;
		float m_constant;
	public:
		Plane(glm::vec3 normal = glm::vec3(1, 0, 0), float constant = 0.0f) : m_normal(normal), m_constant(constant) {}
		inline void Set(glm::vec3 normal, float constant) { m_normal = normal; m_constant = constant; }
		inline void Normalize() { float mag = glm::length(m_normal); m_normal /= mag; m_constant /= mag; }
		inline float DistanceToPoint(glm::vec3 point) { return glm::dot(m_normal, point) + m_constant; }
		inline float DistanceToSphere(glm::vec3 center, float radius) { return DistanceToPoint(center) - radius; }
	};

	class Frustum
	{
	private:
		Plane m_planes[6];
		Material* m_material;
		VertexArray* m_vertexArray;
		VertexBuffer* m_vertexBuffer;
		glm::mat4 m_matrix;
	public:
		Frustum();
		~Frustum();
		void Draw(glm::mat4 offset = glm::mat4(1.0f));
		void Update();
		inline void SetFromMatrix(glm::mat4 matrix) { m_matrix = matrix; }
	};

}