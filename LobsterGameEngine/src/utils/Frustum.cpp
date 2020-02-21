#include "pch.h"
#include "Frustum.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/VertexLayout.h"
#undef near
#undef far

namespace Lobster
{

	Frustum::Frustum() : 
		m_matrix(glm::mat4(1.0)),
		m_material(MaterialLibrary::UseShader("shaders/SolidColor.glsl")),
		m_vertexArray(nullptr),
		m_vertexBuffer(nullptr)
	{
		// Debug mesh
		glm::vec4 color(0, 1, 1, 1);
		m_material->SetRawUniform("color", glm::value_ptr(color));
		m_vertexBuffer = new VertexBuffer(DrawMode::DYNAMIC_DRAW);
		IndexBuffer* indexBuffer = new IndexBuffer();
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		float vertices[24] = {0};
		uint indices[24] = { 0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };
		m_vertexBuffer->SetData(vertices, 24 * sizeof(float));
		indexBuffer->SetData(indices, 24);
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		vb.push_back(m_vertexBuffer);
		ib.push_back(indexBuffer);
		m_vertexArray = new VertexArray(layout, vb, ib, PrimitiveType::LINES);
	}

	Frustum::~Frustum()
	{
		if (m_material) delete m_material;
		if (m_vertexArray) delete m_vertexArray;
		m_material = nullptr;
		m_vertexArray = nullptr;
	}

	void Frustum::Draw(glm::mat4 offset)
	{
#ifdef LOBSTER_BUILD_DEBUG
		RenderCommand command;
		command.UseMaterial = m_material;
		command.UseVertexArray = m_vertexArray;
		command.UseWorldTransform = offset;
		Renderer::SubmitDebug(command);
#endif
	}

	void Frustum::Update()
	{
		// Planes
		m_planes[0].Set(glm::vec3(m_matrix[3][0] + m_matrix[0][0], m_matrix[3][1] + m_matrix[0][1], m_matrix[3][2] + m_matrix[0][2]), m_matrix[3][3] + m_matrix[0][3]);
		m_planes[1].Set(glm::vec3(m_matrix[3][0] - m_matrix[0][0], m_matrix[3][1] - m_matrix[0][1], m_matrix[3][2] - m_matrix[0][2]), m_matrix[3][3] - m_matrix[0][3]);
		m_planes[2].Set(glm::vec3(m_matrix[3][0] + m_matrix[1][0], m_matrix[3][1] + m_matrix[1][1], m_matrix[3][2] + m_matrix[1][2]), m_matrix[3][3] + m_matrix[1][3]);
		m_planes[3].Set(glm::vec3(m_matrix[3][0] - m_matrix[1][0], m_matrix[3][1] - m_matrix[1][1], m_matrix[3][2] - m_matrix[1][2]), m_matrix[3][3] - m_matrix[1][3]);
		m_planes[4].Set(glm::vec3(m_matrix[3][0] + m_matrix[2][0], m_matrix[3][1] + m_matrix[2][1], m_matrix[3][2] + m_matrix[2][2]), m_matrix[3][3] + m_matrix[2][3]);
		m_planes[5].Set(glm::vec3(m_matrix[3][0] - m_matrix[2][0], m_matrix[3][1] - m_matrix[2][1], m_matrix[3][2] - m_matrix[2][2]), m_matrix[3][3] - m_matrix[2][3]);
		for (int i = 0; i < 6; ++i) {
			m_planes[i].Normalize();
		}
		// Debug mesh
		float vertices[24];
		vertices[0] = vertices[3] = vertices[12] = vertices[15] = -1.0f;
		vertices[1] = vertices[10] = vertices[13] = vertices[22] = -1.0f;
		vertices[2] = vertices[5] = vertices[8] = vertices[11] = -1.0f;
		vertices[6] = vertices[9] = vertices[18] = vertices[21] = 1.0f;
		vertices[4] = vertices[7] = vertices[16] = vertices[19] = 1.0f;
		vertices[14] = vertices[17] = vertices[20] = vertices[23] = 1.0f;
		for (int i = 0; i < 24; i += 3) {
			glm::vec4 corner(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
			corner = glm::inverse(m_matrix) * corner;
			corner /= corner.w;
			vertices[i] = corner.x;
			vertices[i + 1] = corner.y;
			vertices[i + 2] = corner.z;
		}
		m_vertexBuffer->SetData(vertices, 24 * sizeof(float));
	}

}