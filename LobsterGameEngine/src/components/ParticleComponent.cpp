#include "pch.h"
#include "ParticleComponent.h"
#include "graphics/Renderer.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexLayout.h"
#include "objects/GameObject.h"

namespace Lobster
{

	ParticleComponent::ParticleComponent() :
		Component(PARTICLE_COMPONENT),
		b_animated(false),
		m_particleCount(MAX_PARTICLES / 2),
		m_particleSize(0.125f),
		m_particleTexture(nullptr),
		m_material(nullptr),
		m_vertexArray(nullptr),
		m_vertexBuffer(nullptr)
	{
		srand(time(NULL));
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			float x = RandomNumber();
			float y = RandomNumber();
			float z = RandomNumber();
			m_particlePositions[i] = glm::vec3(x, y, z);
		}

		m_material = MaterialLibrary::UseShader("shaders/GPUParticle.glsl");
		m_material->GetShader()->SetUniform("sys_particleSize", m_particleSize);
		m_vertexBuffer = new VertexBuffer(DrawMode::DYNAMIC_DRAW);
		IndexBuffer* indexBuffer = new IndexBuffer();
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		m_vertexBuffer->SetData(m_particlePositions, sizeof(m_particlePositions));
		vb.push_back(m_vertexBuffer);
		std::vector<uint> indices(MAX_PARTICLES);
		std::iota(std::begin(indices), std::end(indices), 0);
		indexBuffer->SetData(indices.data(), MAX_PARTICLES);
		ib.push_back(indexBuffer);

		m_vertexArray = new VertexArray(layout, vb, ib, PrimitiveType::POINTS);
	}

	ParticleComponent::~ParticleComponent()
	{
		if (m_material) delete m_material;
		if (m_vertexArray) delete m_vertexArray;
		m_material = nullptr;
		m_vertexArray = nullptr;
	}

	void ParticleComponent::OnAttach()
	{
		PhysicsComponent* physics = new Rigidbody();
		physics->SetEnabled(false);
		gameObject->AddComponent(physics);
	}

	void ParticleComponent::OnUpdate(double deltaTime)
	{
		// Update particle position
		if (b_animated) {
			for (int i = 0; i < MAX_PARTICLES; ++i) {
				m_particlePositions[i].y -= deltaTime / 1000.f;
				if (m_particlePositions[i].y < -1.f) m_particlePositions[i].y = 1.f;
			}
			// Set vertex data
			m_vertexBuffer->SetData(m_particlePositions, sizeof(m_particlePositions));
		}

		// Update shader uniforms
		m_material->SetRawUniform("ParticleCount", &m_particleCount);
		m_material->SetRawUniform("ParticleSize", &m_particleSize);
		m_material->SetRawTexture2D(0, m_particleTexture ? m_particleTexture : TextureLibrary::Use("textures/ocornut.png"));

		// Submit render command
		RenderCommand command;
		command.UseMaterial = m_material;
		command.UseVertexArray = m_vertexArray;
		command.UseWorldTransform = transform->GetMatrix();
		Renderer::Submit(command);
	}

	void ParticleComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Fuck ocornut");
			ImGui::Checkbox("Animated?", &b_animated);
			ImGui::SliderInt("Particle Count", &m_particleCount, 0, MAX_PARTICLES);
			ImGui::SliderFloat("Particle Size", &m_particleSize, 0.f, 1.f);
			ImVec2 previewSize(24, 24);
			Texture2D* notFound = TextureLibrary::Placeholder();
			if (ImGui::ImageButton(m_particleTexture ? m_particleTexture->Get() : notFound->Get(), previewSize)) {
				std::string path = FileSystem::OpenFileDialog();
				if (!path.empty()) {
					m_particleTexture = TextureLibrary::Use(path.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Particle Texture");
		}
	}

	inline float ParticleComponent::RandomNumber() const
	{
		float number = (float(rand()) / float(RAND_MAX)) * 2.f - 1.f;
		return number;
	}

}