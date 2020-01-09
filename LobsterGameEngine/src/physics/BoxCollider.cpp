#include "pch.h"
#include "BoxCollider.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/VertexLayout.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"

namespace Lobster {
	BoxCollider::BoxCollider(PhysicsComponent* physics, Transform transform) :
		Collider(physics, transform)
	{
		// member variable initialization
		m_debugColor = glm::vec4(0, 0, 1, 1);
		SetColliderType(1);

		memset(m_debugData, 0, sizeof(float) * 24);
		memset(m_debugInitialData, 0, sizeof(float) * 24);
		//memset(m_debugTranslatedData, 0, sizeof(float) * 24);
		m_debugMaterial = MaterialLibrary::UseShader("shaders/SolidColor.glsl");
		m_debugMaterial->SetRawUniform("color", glm::value_ptr(m_debugColor));
		m_debugVertexBuffer = new VertexBuffer(DrawMode::DYNAMIC_DRAW);
		IndexBuffer* indexBuffer = new IndexBuffer();
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;

		// construct vertex buffer data
		vb.push_back(m_debugVertexBuffer);

		// construct index buffer data
		uint indices[24] = {
			0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4
		};
		indexBuffer->SetData(indices, 24);
		ib.push_back(indexBuffer);

		m_debugMesh = new VertexArray(layout, vb, ib, PrimitiveType::LINES);
	}

	void BoxCollider::SetOwner(GameObject* owner) {
		std::pair<glm::vec3, glm::vec3> pair = owner->GetBound();

		m_debugInitialData[0] = m_debugInitialData[3] = m_debugInitialData[12] = m_debugInitialData[15] = pair.first.x;
		m_debugInitialData[1] = m_debugInitialData[10] = m_debugInitialData[13] = m_debugInitialData[22] = pair.first.y;
		m_debugInitialData[2] = m_debugInitialData[5] = m_debugInitialData[8] = m_debugInitialData[11] = pair.first.z;
		m_debugInitialData[6] = m_debugInitialData[9] = m_debugInitialData[18] = m_debugInitialData[21] = pair.second.x;
		m_debugInitialData[4] = m_debugInitialData[7] = m_debugInitialData[16] = m_debugInitialData[19] = pair.second.y;
		m_debugInitialData[14] = m_debugInitialData[17] = m_debugInitialData[20] = m_debugInitialData[23] = pair.second.z;
	}

	void BoxCollider::OnUpdate(double deltaTime) {
		//	First update the matrices.
		//	Both matrices has to be updated here for physics calculation.
		transform->UpdateMatrix();
		m_transform.UpdateMatrix();
		UpdateRotation();
	}

	//	Undefined query, remove this function in future version.
	bool BoxCollider::Intersects(glm::vec3 pos, glm::vec3 dir, float& t) {
		return false;
	}

	void BoxCollider::DebugDraw() {
#ifdef LOBSTER_BUILD_DEBUG
		// validate data, and return if we haven't define game object yet
		if (!physics) return;
		if (!m_debugMaterial || !m_debugMesh)
		{
			throw std::runtime_error("Oops... This BoxCollider is not ready to be drawn!");
			// Possible causes:
			// 1. BoxCollider was probably not correctly initialized
			// 2. This collider is of zero size, we don't accept an invisible collider
			return;
		}

		//	issue draw call
		RenderCommand command;
		command.UseMaterial = m_debugMaterial;
		command.UseVertexArray = m_debugMesh;
		command.UseWorldTransform = glm::translate(glm::vec3(0, 0, 0));
		Renderer::Submit(command);
#endif
	}

	std::vector<glm::vec3> BoxCollider::GetVertices() const {
		std::vector<glm::vec3> vertices;

		for (int i = 0; i < 24; i += 3) {
			vertices.push_back(glm::vec3(m_debugData[i], m_debugData[i + 1], m_debugData[i + 2]));
		}
		return vertices;
	}

	void BoxCollider::UpdateRotation() {
		for (int i = 0; i < 24; i += 3) {
			glm::vec3 vertices = glm::vec3(m_debugInitialData[i], m_debugInitialData[i + 1], m_debugInitialData[i + 2]);

			glm::vec3 boundCorner = transform->GetMatrix() * m_transform.GetMatrix() * glm::vec4(vertices, 1.0);

			m_debugData[i] = boundCorner.x;
			m_debugData[i + 1] = boundCorner.y;
			m_debugData[i + 2] = boundCorner.z;
		}

		m_debugVertexBuffer->SetData(m_debugData, sizeof(float) * 24);
	}
}