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
		m_vertexColor = glm::vec4(0, 0, 1, 1);
		SetColliderType(1);

		memset(m_vertexData, 0, sizeof(float) * 24);
		memset(m_vertexInitialData, 0, sizeof(float) * 24);
		//memset(m_debugTranslatedData, 0, sizeof(float) * 24);
		m_vertexMaterial = MaterialLibrary::UseShader("shaders/SolidColor.glsl");
		m_vertexMaterial->SetRawUniform("color", glm::value_ptr(m_vertexColor));
		m_vertexBuffer = new VertexBuffer(DrawMode::DYNAMIC_DRAW);
		IndexBuffer* indexBuffer = new IndexBuffer();
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;

		// construct vertex buffer data
		vb.push_back(m_vertexBuffer);

		// construct index buffer data
		uint indices[24] = {
			0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4
		};
		indexBuffer->SetData(indices, 24);
		ib.push_back(indexBuffer);

		m_mesh = new VertexArray(layout, vb, ib, PrimitiveType::LINES);
	}

	void BoxCollider::SetOwner(GameObject* owner) {
		std::pair<glm::vec3, glm::vec3> pair = owner->GetBound();

		m_vertexInitialData[0] = m_vertexInitialData[3] = m_vertexInitialData[12] = m_vertexInitialData[15] = pair.first.x;
		m_vertexInitialData[1] = m_vertexInitialData[10] = m_vertexInitialData[13] = m_vertexInitialData[22] = pair.first.y;
		m_vertexInitialData[2] = m_vertexInitialData[5] = m_vertexInitialData[8] = m_vertexInitialData[11] = pair.first.z;
		m_vertexInitialData[6] = m_vertexInitialData[9] = m_vertexInitialData[18] = m_vertexInitialData[21] = pair.second.x;
		m_vertexInitialData[4] = m_vertexInitialData[7] = m_vertexInitialData[16] = m_vertexInitialData[19] = pair.second.y;
		m_vertexInitialData[14] = m_vertexInitialData[17] = m_vertexInitialData[20] = m_vertexInitialData[23] = pair.second.z;
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

	void BoxCollider::Draw() {
#ifdef LOBSTER_BUILD_DEBUG
		// validate data, and return if we haven't define game object yet
		if (!physics) return;
		if (!m_vertexMaterial || !m_mesh)
		{
			throw std::runtime_error("Oops... This BoxCollider is not ready to be drawn!");
			// Possible causes:
			// 1. BoxCollider was probably not correctly initialized
			// 2. This collider is of zero size, we don't accept an invisible collider
			return;
		}

		//	issue draw call
		RenderCommand command;
		command.UseMaterial = m_vertexMaterial;
		command.UseVertexArray = m_mesh;
		command.UseWorldTransform = glm::translate(glm::vec3(0, 0, 0));
		Renderer::Submit(command);
#endif
	}

	std::vector<glm::vec3> BoxCollider::GetVertices() const {
		std::vector<glm::vec3> vertices;
		float epsilon = 0.001f;

		for (int i = 0; i < 24; i += 3) {
			//	First get the position without epsilon.
			glm::vec3 position = glm::vec3(m_vertexData[i], m_vertexData[i + 1], m_vertexData[i + 2]);

			//	Add the epsilon.
			glm::vec3 epsilonVec = glm::vec3(0, 0, 0);
			epsilonVec.x += ( (i / 6) % 2 ? epsilon : -epsilon );
			epsilonVec.y += ( ((i + 3) / 6) % 2 ? epsilon : -epsilon );
			epsilonVec.z += ( i / 12 ? epsilon : -epsilon );
			
			//	Epsilon apply transform.
			epsilonVec = transform->WorldRotation * m_transform.WorldRotation * glm::vec4(epsilonVec, 1.0);

			//	Push the position back.
			vertices.push_back(position + epsilonVec);
		}
		return vertices;
	}

	void BoxCollider::UpdateRotation() {
		for (int i = 0; i < 24; i += 3) {
			glm::vec3 vertices = glm::vec3(m_vertexInitialData[i], m_vertexInitialData[i + 1], m_vertexInitialData[i + 2]);

			glm::vec3 boundCorner = transform->GetMatrix() * m_transform.GetMatrix() * glm::vec4(vertices, 1.0);

			m_vertexData[i] = boundCorner.x;
			m_vertexData[i + 1] = boundCorner.y;
			m_vertexData[i + 2] = boundCorner.z;
		}

		m_vertexBuffer->SetData(m_vertexData, sizeof(float) * 24);
	}
}