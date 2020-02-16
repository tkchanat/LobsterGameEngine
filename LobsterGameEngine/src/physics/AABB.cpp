#include "pch.h"
#include "AABB.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/VertexLayout.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Renderer.h"
#include "objects/GameObject.h"

namespace Lobster
{

    // The second method involves creating a new AABB each frame to encompass
    // your objects new orientation. I believe that the simplest method is to
    // take ALL 8 points of the box, rotate them, and then find the min/max
    // in each axis from those 8 points to find the two points(min & max)
    // points of your new AABB.

    AABB::AABB(PhysicsComponent* physics, Transform transform) :
		Center(glm::vec3(0, 0, 0)),
		Min(glm::vec3(0, 0, 0)),
		Max(glm::vec3(0, 0, 0)),
		Collider(physics, transform)
    {
        // member variable initialization
		m_vertexColor = glm::vec4(0, 1, 0, 1);
		SetColliderType(0);

        memset(m_vertexData, 0, sizeof(float) * 24);
		memset(m_vertexInitialData, 0, sizeof(float) * 24);
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

	AABB::~AABB()
	{
		if (m_vertexMaterial) delete m_vertexMaterial;
		if (m_mesh) delete m_mesh;
		m_vertexMaterial = nullptr;
		m_mesh = nullptr;
		m_vertexBuffer = nullptr;
	}

	void AABB::SetOwner(GameObject* owner) {
		std::pair<glm::vec3, glm::vec3> pair = owner->GetBound();
		Min = pair.first;
		Max = pair.second;
		SetVertices(true);
	}

	void AABB::OnUpdate(double deltaTime) {
		// update AABB
		m_transform.UpdateMatrix();
		Center = transform->WorldPosition;
		UpdateRotation();
	}

    void AABB::Draw()
    {
#ifdef LOBSTER_BUILD_DEBUG
        // validate data, and return if we haven't define game object yet
		if (!physics) return;
        if(!m_vertexMaterial || !m_mesh || Min == Max)
        {
            throw std::runtime_error("Oops... This AABB is not ready to be drawn!");
			// Possible causes:
			// 1. AABB was probably not correctly initialized, please call AABB(glm::vec3 min, glm::vec3 max) instead of default constructor
			// 2. You accidentally set Min equals to Max, we don't accept zero volume AABB
            return;
        }

        //	issue draw call
        RenderCommand command;
        command.UseMaterial = m_vertexMaterial;
        command.UseVertexArray = m_mesh;
        command.UseWorldTransform = glm::translate(Center);
        Renderer::Submit(command);
#endif
    }

	bool AABB::Intersects(glm::vec3 pos, glm::vec3 dir, float& t)
	{
		static glm::vec3 epsilon(0.001, 0.001, 0.001);
		const static glm::vec3 normals[] = {
			glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),
			glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
		};
		const glm::vec3 centers[] = {
			glm::vec3(Center.x + Max.x, Center.y, Center.z), glm::vec3(Center.x + Min.x, Center.y, Center.z),
			glm::vec3(Center.x, Center.y + Max.y, Center.z), glm::vec3(Center.x, Center.y + Min.y, Center.z),
			glm::vec3(Center.x, Center.y, Center.z + Max.z), glm::vec3(Center.x, Center.y, Center.z + Min.z)
		};
		glm::vec3 gMin = Center + Min - epsilon; glm::vec3 gMax = Center + Max + epsilon;
		for (int i = 0; i < 6; i++) {
			float denom = glm::dot(normals[i], dir);
			if (abs(denom) > 0.0001f) {
				float rt = glm::dot(centers[i] - pos, normals[i]) / denom;
				// extent check
				glm::vec3 pt = rt * dir + pos;
				if (pt.x > gMin.x && pt.y > gMin.y && pt.z > gMin.z &&
					pt.x < gMax.x && pt.y < gMax.y && pt.z < gMax.z) {
					t = rt;
					return true;
				}				
			}				
		}
		return false;
	}

	std::vector<glm::vec3> AABB::GetVertices() const {
		std::vector<glm::vec3> vertices;

		for (int i = 0; i < 24; i += 3) {
			vertices.push_back(glm::vec3(m_vertexData[i], m_vertexData[i + 1], m_vertexData[i + 2]) + transform->WorldPosition);
		}
		return vertices;
	}

    void AABB::SetVertices(bool initialize = false)
    {
		m_vertexData[0] = m_vertexData[3] = m_vertexData[12] = m_vertexData[15] = Min.x;
        m_vertexData[1] = m_vertexData[10] = m_vertexData[13] = m_vertexData[22] = Min.y;
        m_vertexData[2] = m_vertexData[5] = m_vertexData[8] = m_vertexData[11] = Min.z;
        m_vertexData[6] = m_vertexData[9] = m_vertexData[18] = m_vertexData[21] = Max.x;
        m_vertexData[4] = m_vertexData[7] = m_vertexData[16] = m_vertexData[19] = Max.y;
        m_vertexData[14] = m_vertexData[17] = m_vertexData[20] = m_vertexData[23] = Max.z;

		if (initialize) {
			memcpy(m_vertexInitialData, m_vertexData, sizeof(m_vertexData));
		}
        m_vertexBuffer->SetData(m_vertexData, sizeof(float) * 24);
    }

    void AABB::UpdateRotation()
    {
		Min.x = Max.x = (m_vertexInitialData[0] + m_vertexInitialData[18]) / 2.0f;
		Min.y = Max.y = (m_vertexInitialData[1] + m_vertexInitialData[19]) / 2.0f;
		Min.z = Max.z = (m_vertexInitialData[2] + m_vertexInitialData[20]) / 2.0f;
		
		for(int i = 0; i < 24; i += 3)
        {
			glm::vec3 vertices = glm::vec3(m_vertexInitialData[i], m_vertexInitialData[i + 1], m_vertexInitialData[i + 2]);
			glm::vec3 rotatedCorner = transform->GetMatrix() * glm::vec4(vertices, 1.0);

            Min.x = rotatedCorner.x < Min.x ? rotatedCorner.x : Min.x;
            Min.y = rotatedCorner.y < Min.y ? rotatedCorner.y : Min.y;
            Min.z = rotatedCorner.z < Min.z ? rotatedCorner.z : Min.z;
            Max.x = rotatedCorner.x > Max.x ? rotatedCorner.x : Max.x;
            Max.y = rotatedCorner.y > Max.y ? rotatedCorner.y : Max.y;
            Max.z = rotatedCorner.z > Max.z ? rotatedCorner.z : Max.z;
        }

		Min += m_transform.WorldPosition;
		Max += m_transform.WorldPosition;
        SetVertices();
    }

}
