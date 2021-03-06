#include "pch.h"
#include "ParticleComponent.h"
#include "graphics/Renderer.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexLayout.h"
#include "objects/GameObject.h"
#include "system/Input.h"
#include "system/UndoSystem.h"

namespace Lobster
{
	const char* ParticleComponent::shapes[] = { "Box", "Cone", "Sphere" };

	ParticleComponent::ParticleComponent() :
		Component(PARTICLE_COMPONENT),
		m_shape(EmitterShape::BOX),
		b_animated(false),
		b_emitOneByOne(false),
		_simulateElapsedTime(0.0f),
		_volumeFilled(false),
		_killExpiredParticle(false),
		_deathCount(0),
		m_emissionSpeed(1.0f),
		m_emissionRate(0.1f),
		m_emissionAngle(M_PI/6),
		m_colorStartTransition(glm::vec4(1.0)),
		m_colorEndTransition(glm::vec4(1.0)),
		m_particleCount(0),
		m_particleCutoff(MAX_PARTICLES / 2),
		m_particleSize(0.125f),
		m_particleOrientation(0.0f),
		m_particleTexture(nullptr),
		m_material(nullptr),
		m_vertexArray(nullptr),
		m_vertexBuffer(nullptr)
	{
		srand(time(NULL));
		FillVolume();

		m_material = MaterialLibrary::UseShader("shaders/GPUParticle.glsl");
		m_material->SetRenderingMode(RenderingMode::MODE_TRANSPARENT);
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
		if (Application::GetMode() == EDITOR) return;
		if (b_animated) {
			// Emit all at once
			if (!b_emitOneByOne && !_volumeFilled) {
				m_particleCount = m_particleCutoff;
				FillVolume();
				_volumeFilled = true;
			}
			// One-by-one
			else if (b_emitOneByOne && _volumeFilled) {
				m_particleCount = 0;
				_volumeFilled = false;
			}
			// Check EmitOnce() end condition
			if (_killExpiredParticle && _deathCount == m_particleCutoff) {
				_killExpiredParticle = false;
				b_animated = false;
				ResetParticleCount();
			}
			// Calculate new particle position
			switch (m_shape)
			{
			case EmitterShape::BOX:
				BoxEmitter(deltaTime); break;
			case EmitterShape::CONE:
				ConeEmitter(deltaTime); break;
			case EmitterShape::SPHERE:
				SphereEmitter(deltaTime); break;
			default:
				break;
			}
			// Set vertex data
			m_vertexBuffer->SetData(m_particlePositions, sizeof(m_particlePositions));
		}

		// Update shader uniforms
		int cutoff = std::min(m_particleCutoff, m_particleCount);
		m_material->SetRawUniform("EmissionShape", (void*)&m_shape);
		m_material->SetRawUniform("ColorStartTransition", (void*)glm::value_ptr(m_colorStartTransition));
		m_material->SetRawUniform("ColorEndTransition", (void*)glm::value_ptr(m_colorEndTransition));
		m_material->SetRawUniform("DeadParticleCount", &_deathCount);
		m_material->SetRawUniform("ParticleCutoff", &cutoff);
		m_material->SetRawUniform("ParticleSize", &m_particleSize);
		m_material->SetRawUniform("ParticleOrientation", (void*)glm::value_ptr(glm::rotate(m_particleOrientation, glm::vec3(0, 0, 1))));
		m_material->SetRawTexture2D(0, m_particleTexture ? m_particleTexture : TextureLibrary::Use("textures/ui/sphere.png"));

		// Submit render command
		RenderCommand command;
		command.UseMaterial = m_material;
		command.UseVertexArray = m_vertexArray;
		command.UseWorldTransform = transform->GetMatrix();
		Renderer::Submit(command);
	}

	void ParticleComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("Particle System", &m_show, ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Checkbox("Simulate On Begin", &b_animated)) {
				UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &b_animated, !b_animated, b_animated, std::string(b_animated ? "Enabled" : "Disabled") + " animation of particle system for " + GetOwner()->GetName()));
			}
			EmitterShape prevShape = m_shape;

			ImGui::Combo("Shape", (int*)&m_shape, shapes, IM_ARRAYSIZE(shapes));
			if (prevShape != m_shape) {
				FillVolume();
				UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_shape, prevShape, m_shape, "Set particle shape mode to " + std::string(shapes[(int)m_shape]) + " for " + GetOwner()->GetName(), &ParticleComponent::FillVolume));
			}
			ImGui::Spacing();

			if(ImGui::TreeNode("Emission")) {
				if (ImGui::Checkbox("Emit One By One?", &b_emitOneByOne)) {
					ResetParticleCount();
					UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &b_emitOneByOne, !b_emitOneByOne, b_emitOneByOne, std::string(b_emitOneByOne ? "Enable" : "Disable") + " particle one-to-one emission mode for " + GetOwner()->GetName(), &ParticleComponent::ResetParticleCount));
				}

				if (ImGui::DragFloat("Emission Speed", &m_emissionSpeed, 0.05f, 0.0, FLT_MAX / INT_MAX)) {
					m_isChanging = 5;
				}
				if (m_isChanging != 5) {
					m_prevProp[5] = m_emissionSpeed;
				}
				else if (ImGui::IsItemActive() == false) {
					if (m_prevProp[0] != m_emissionSpeed) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_emissionSpeed, m_prevProp[5], m_emissionSpeed, "Set particle emission speed to " + StringOps::ToString(m_emissionSpeed) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::DragFloat("Emission Rate (per second)", &m_emissionRate, 0.1f)) {
					m_isChanging = 0;
				}
				m_emissionRate = m_emissionRate < 0.0f ? 0.0f : m_emissionRate;
				if (m_isChanging != 0) {
					m_prevProp[0] = m_emissionRate;
				} else if (ImGui::IsItemActive() == false) {
					if (m_prevProp[0] != m_emissionRate) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_emissionRate, m_prevProp[0], m_emissionRate, "Set particle emission rate to " + StringOps::ToString(m_emissionRate) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderAngle("Emission Angle", &m_emissionAngle, 0.f, 89.f)) {
					m_isChanging = 1;
				}
				if (m_isChanging != 1) {
					m_prevProp[1] = m_emissionAngle;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[1] != m_emissionAngle) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_emissionAngle, m_prevProp[1], m_emissionAngle, "Set particle emission angle to " + StringOps::ToDegreeString(m_emissionAngle) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Transition")) {
				ImGui::ColorEdit4("Start Color", glm::value_ptr(m_colorStartTransition));
				if (ImGui::IsItemActive()) {
					m_isChangingColor = 0;
				}
				if (m_isChangingColor != 0) {
					m_prevColor[0] = m_colorStartTransition;
				} else if (m_isChangingColor == 0 && ImGui::IsItemActive() == false) {
					if (m_prevColor[0] != m_colorStartTransition) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_colorStartTransition, m_prevColor[0], m_colorStartTransition, "Set transition start color to " + StringOps::ToColorString(m_colorStartTransition) + " for " + GetOwner()->GetName()));
					}
					m_isChangingColor = -1;
				}

				ImGui::ColorEdit4("End Color", glm::value_ptr(m_colorEndTransition));
				if (ImGui::IsItemActive()) {
					m_isChangingColor = 1;
				}
				if (m_isChangingColor != 1) {
					m_prevColor[1] = m_colorEndTransition;
				} else if (m_isChangingColor == 1 && ImGui::IsItemActive() == false) {
					if (m_prevColor[1] != m_colorEndTransition) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_colorEndTransition, m_prevColor[1], m_colorEndTransition, "Set transition end color to " + StringOps::ToColorString(m_colorEndTransition) + " for " + GetOwner()->GetName()));
					}
					m_isChangingColor = -1;
				}

				ImGui::TreePop();
			}
			
			if (ImGui::TreeNode("Particle")) {
				if (ImGui::SliderInt("Cutoff", &m_particleCutoff, 0, MAX_PARTICLES)) {
					m_isChanging = 2;
				}
				if (m_isChanging != 2) {
					m_prevProp[2] = m_particleCutoff;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[2] != m_particleCutoff) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_particleCutoff, (int) round(m_prevProp[2]), m_particleCutoff, "Set particle cutoff to " + std::to_string(m_particleCutoff) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderFloat("Size", &m_particleSize, 0.f, 1.f)) {
					m_isChanging = 3;
				}
				if (m_isChanging != 3) {
					m_prevProp[3] = m_particleSize;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[3] != m_particleSize) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_particleSize, m_prevProp[3], m_particleSize, "Set particle size to " + StringOps::ToString(m_particleSize) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}

				if (ImGui::SliderAngle("Orientation", &m_particleOrientation, 0.f, 360.f)) {
					m_isChanging = 4;
				}
				if (m_isChanging != 4) {
					m_prevProp[4] = m_particleOrientation;
				} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
					if (m_prevProp[1] != m_particleOrientation) {
						UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_particleOrientation, m_prevProp[4], m_particleOrientation, "Set particle orientation angle to " + StringOps::ToDegreeString(m_particleOrientation) + " for " + GetOwner()->GetName()));
					}
					m_isChanging = -1;
				}
				ImVec2 previewSize(24, 24);
				Texture2D* notFound = TextureLibrary::Placeholder();
				if (ImGui::ImageButton(m_particleTexture ? m_particleTexture->Get() : notFound->Get(), previewSize)) {
					std::string path = FileSystem::Path(FileSystem::OpenFileDialog());
					if (!path.empty()) {
						Texture2D* texture = TextureLibrary::Use(path.c_str());
						if (m_particleTexture != texture) {
							UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_particleTexture, m_particleTexture, texture, "Set particle texture for " + GetOwner()->GetName()));
							m_particleTexture = texture;
						}
					}
				}
				ImGui::SameLine();
				ImGui::Text("Particle Texture");
				ImGui::TreePop();
			}
		}
	}

	void ParticleComponent::ResetParticleCount() {
		m_particleCount = 0;
		_simulateElapsedTime = 0.f;
	}

	void ParticleComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		oarchive(*this);
	}

	void ParticleComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		try {
			iarchive(*this);
		}
		catch (std::exception e) {
			LOG("Deserializing ParticleComponent failed. Reason: {}", e.what());
		}
	}

	void ParticleComponent::EmitOnce()
	{
		b_animated = true;
		b_emitOneByOne = true;
		_killExpiredParticle = true;
		_deathCount = 0;
		ResetParticleCount();
	}

	void ParticleComponent::FillVolume()
	{
		//Timer timer;
		switch (m_shape)
		{
		case EmitterShape::BOX:
			for (int i = 0; i < MAX_PARTICLES; ++i) {
				float x = RandomNumber() * 2.f - 1.f;
				float y = RandomNumber() * 2.f - 1.f;
				float z = RandomNumber() * 2.f - 1.f;
				m_particlePositions[i] = { x, y, z };
			}
			break;
		case EmitterShape::CONE:
			for (int i = 0; i < MAX_PARTICLES; ++i) {
				float h = 1.f * std::cbrt(RandomNumber());
				float r = tan(m_emissionAngle) * h * std::sqrt(RandomNumber());
				float t = 2.f * M_PI *RandomNumber();
				m_particlePositions[i] = { r*cos(t), h, r*sin(t) };
			}
			break;
		case EmitterShape::SPHERE:
			for (int i = 0; i < MAX_PARTICLES; ++i) {
				float u = RandomNumber() * 2.f - 1.f;
				float x1 = RandomNumber() * 2.f - 1.f;
				float x2 = RandomNumber() * 2.f - 1.f;
				float x3 = RandomNumber() * 2.f - 1.f;
				float mag = std::sqrt(x1*x1 + x2*x2 + x3*x3);
				x1 /= mag; 
				x2 /= mag; 
				x3 /= mag;
				float c = std::cbrt(u);
				m_particlePositions[i] = { x1*c, x2*c, x3*c };
			}
			break;
		default:
			break;
		}
		//LOG("Particle position generation: {}s", timer.GetElapsedTime());
	}

	void ParticleComponent::BoxEmitter(double deltaTime)
	{
		// emit new particle one by one
		if (b_emitOneByOne) {
			_simulateElapsedTime += deltaTime * m_emissionSpeed / 1000.0; // in seconds
			if (_simulateElapsedTime > m_emissionRate) {
				if (m_particleCount + 1 < MAX_PARTICLES) {
					float x = RandomNumber() * 2.f - 1.f;
					float y = -1.f;
					float z = RandomNumber() * 2.f - 1.f;
					m_particlePositions[m_particleCount++] = { x, y, z };
				}
				_simulateElapsedTime = 0.0f;
			}
		}

		// animate particles
		for (int i = 0; i < m_particleCount; ++i) {
			m_particlePositions[i].y += deltaTime * m_emissionSpeed / 1000.f;
			if (m_particlePositions[i].y > 1.f) {
				m_particlePositions[i].y = std::fmod(m_particlePositions[i].y, 2.f) - 1.99f;
				if (_killExpiredParticle) {
					_deathCount++;
				}
			}
		}
	}

	void ParticleComponent::ConeEmitter(double deltaTime)
	{
		// emit new particle one by one
		if (b_emitOneByOne) {
			_simulateElapsedTime += deltaTime * m_emissionSpeed / 1000.0; // in seconds
			if (_simulateElapsedTime > m_emissionRate) {
				if (m_particleCount + 1 < MAX_PARTICLES) {
					float h = 1.f * std::cbrt(RandomNumber());
					float r = tan(m_emissionAngle) * h * std::sqrt(RandomNumber());
					float t = 2.f * M_PI *RandomNumber();
					m_particlePositions[m_particleCount++] = 0.001f * glm::vec3(r*cos(t), h, r*sin(t));
				}
				_simulateElapsedTime = 0.0f;
			}
		}

		// animate particles
		for (int i = 0; i < m_particleCount; ++i) {
			glm::vec3 direction = glm::normalize(m_particlePositions[i]);
			m_particlePositions[i] += direction * float(deltaTime * m_emissionSpeed / 1000.f);
			float dist = glm::length(m_particlePositions[i]);
			if (dist > 1.f) {
				dist = std::fmod(dist, 1.f);
				float h = 1.f * std::cbrt(RandomNumber());
				float r = tan(m_emissionAngle) * h * std::sqrt(RandomNumber());
				float t = 2.f * M_PI *RandomNumber();
				glm::vec3 position = { r*cos(t), h, r*sin(t) };
				m_particlePositions[i] = glm::normalize(position) * dist;
				if (_killExpiredParticle) {
					_deathCount++;
				}
			}
		}
	}

	void ParticleComponent::SphereEmitter(double deltaTime)
	{
		// emit new particle one by one
		if (b_emitOneByOne) {
			_simulateElapsedTime += deltaTime * m_emissionSpeed / 1000.0; // in seconds
			if (_simulateElapsedTime > m_emissionRate) {
				if (m_particleCount + 1 < MAX_PARTICLES) {
					float u = RandomNumber() * 2.f - 1.f;
					float x1 = RandomNumber() * 2.f - 1.f;
					float x2 = RandomNumber() * 2.f - 1.f;
					float x3 = RandomNumber() * 2.f - 1.f;
					float mag = std::sqrt(x1*x1 + x2 * x2 + x3 * x3);
					x1 /= mag;
					x2 /= mag;
					x3 /= mag;
					float c = std::cbrt(u);
					m_particlePositions[m_particleCount++] = glm::normalize(glm::vec3(x1*c, x2*c, x3*c)) * 0.001f;
				}
				_simulateElapsedTime = 0.0f;
			}
		}

		// animate particle
		for (int i = 0; i < m_particleCount; ++i) {
			glm::vec3 direction = glm::normalize(m_particlePositions[i]);
			m_particlePositions[i] += direction * float(deltaTime * m_emissionSpeed / 1000.f);
			float dist = glm::length(m_particlePositions[i]);
			if (dist > 1.f) {
				dist = std::fmod(dist, 1.f);
				m_particlePositions[i] = direction * dist;
				if (_killExpiredParticle) {
					_deathCount++;
				}
			}
		}
	}

	inline float ParticleComponent::RandomNumber() const
	{
		float number = (float(rand()) / float(RAND_MAX));
		return number;
	}

}