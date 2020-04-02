#include "pch.h"
#include "LightComponent.h"
#include "imgui/ImGuiScene.h"
#include "graphics/FrameBuffer.h"
#include "graphics/Renderer.h"
#include "objects/Transform.h"
#include "physics/Rigidbody.h"
#include "system/Input.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	LightComponent::LightComponent(LightType type) :
		Component(LIGHT_COMPONENT),
		m_type(type),
		m_color(glm::vec3(1)),
		m_intensity(1),
		b_dirty(false),
		m_depthBuffer(nullptr),
		m_lightSpaceMatrix(glm::mat4(1.0))
	{
		std::vector<RenderTargetDesc> renderTargetsDesc;
		RenderTargetDesc depthDesc;
		depthDesc.depthOnly = true;
		depthDesc.format = Formats::DepthComponentFormat;
		depthDesc.type = Types::FloatType;
		renderTargetsDesc.push_back(depthDesc);
		m_depthBuffer = new FrameBuffer(1024, 1024, renderTargetsDesc);
	}

	LightComponent::~LightComponent()
	{
		if (b_isVirtuallyDeleted == false) LightLibrary::RemoveLight(this, m_type);
		if (m_depthBuffer) delete m_depthBuffer;
		m_depthBuffer = nullptr;
	}

	void LightComponent::VirtualCreate() {
		b_isVirtuallyDeleted = false;
		LightLibrary::AddLight(this, m_type);
	}

	void LightComponent::VirtualDelete() {
		b_isVirtuallyDeleted = true;
		LightLibrary::RemoveLight(this, m_type);
	}

	void LightComponent::OnAttach()
	{
		LightLibrary::AddLight(this, GetType());

		PhysicsComponent* physics = new Rigidbody();
		physics->SetEnabled(false);
		gameObject->AddComponent(physics);
	}

	void LightComponent::OnUpdate(double deltaTime)
	{
#ifdef LOBSTER_BUILD_EDITOR
		GizmosCommand command;
		command.texture = "textures/ui/light.png";
		command.position = transform->WorldPosition;
		command.source = gameObject;
		ImGuiScene::SubmitGizmos(command);
#endif
	}

	void LightComponent::OnImGuiRender()
	{
		std::string headerLabel = fmt::format("Light {}", b_dirty ? "*" : "");
		if (ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Light type
			const char* types[] = { "Directional", "Point" };
			m_prevType = m_type;
			ImGui::Combo("Type", (int*)&m_type, types, IM_ARRAYSIZE(types));
			if (m_prevType != m_type) {
				UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_type, m_prevType, m_type, "Set light type to " + std::string(types[m_type]) + " for " + GetOwner()->GetName(), &LightComponent::ChangeLightType));
				ChangeLightType();
			}

			// Light color
			ImGui::ColorEdit3("Color", glm::value_ptr(m_color), 0);
			if (ImGui::IsItemActive()) {
				m_isChanging = 0;
			}
			if (m_isChanging != 0) {
				m_prevColor = m_color;
			} else if (m_isChanging == 0 && ImGui::IsItemActive() == false) {
				if (m_prevColor != m_color) {
					UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_color, m_prevColor, m_color, "Set light color to " + StringOps::ToColorString(m_color) + " for " + GetOwner()->GetName()));
				}
				m_isChanging = -1;
			}

			// Light intensity/attenuation
			const char* label = (m_type == POINT_LIGHT) ? "Attenuation" : "Intensity";
			if (ImGui::SliderFloat(label, &m_intensity, 0.f, 1.f)) {
				m_isChanging = 1;
			}
			if (m_isChanging != 1) {
				m_prevIntensity = m_intensity;
			} else if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
				if (m_prevIntensity != m_intensity) {
					UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_intensity, m_prevIntensity, m_intensity, "Set light intensity to " + StringOps::ToString(m_intensity) + " for " + GetOwner()->GetName()));
				}
				m_isChanging = -1;
			}
		}
	}

	void LightComponent::ChangeLightType() {
		LightLibrary::RemoveLight(this, m_prevType);
		LightLibrary::AddLight(this, m_type);
		m_prevType = m_type;
		b_dirty = true;
	}

	void LightComponent::RenderDepthMap(const std::list<RenderCommand>& queue)
	{
		Shader* shader = ShaderLibrary::Use("shaders/ShadowMapping.glsl");
		shader->Bind();
		glm::mat4 lightProjection, lightView;
		float near_plane = 0.1f, far_plane = 20.0f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(transform->WorldPosition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		m_lightSpaceMatrix = lightProjection * lightView;
		shader->SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);
		// render scene from light's point of view
		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		glViewport(0, 0, 1024, 1024);
		m_depthBuffer->BindAndClear(ClearFlag::DEPTH);
		for (auto& command : queue) {
			shader->SetUniform("model", command.UseWorldTransform);
			if (command.UseVertexArray)
				command.UseVertexArray->Draw();
		}
		shader->Unbind();
		m_depthBuffer->Unbind();
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
	}


    void LightComponent::Serialize(cereal::JSONOutputArchive& oarchive)
    {
        //LOG("Serializing LightComponent");
        oarchive(*this);
    }

    void LightComponent::Deserialize(cereal::JSONInputArchive& iarchive)
    {
        //LOG("Deserializing LightComponent");
        try {
            iarchive(*this);
        }
        catch (std::exception e) {
            LOG("Deserializing LightComponent failed. Reason: {}", e.what());
        }
    }

	// =======================================================
	// LightLibrary
	// =======================================================
	LightLibrary* LightLibrary::s_instance = nullptr;

	void LightLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("LightLibrary already existed!");
		}
		s_instance = new LightLibrary();

		// directional lights
		glGenBuffers(1, &s_instance->m_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, s_instance->m_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo_Lights), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, s_instance->m_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void LightLibrary::AddLight(LightComponent * light, LightType type)
	{
		switch (type)
		{
		case Lobster::DIRECTIONAL_LIGHT:
			if (s_instance->m_directionalLights.size() + 1 > MAX_DIRECTIONAL_LIGHTS) {
				WARN("MAX_DIRECTIONAL_LIGHTS exceeded, this new light will be ignored!");
				return;
			}
			s_instance->m_directionalLights.push_back(light);
			break;
		case Lobster::POINT_LIGHT:
			if (s_instance->m_pointLights.size() + 1 > MAX_POINT_LIGHTS) {
				WARN("MAX_POINT_LIGHTS exceeded, this new light will be ignored!");
				return;
			}
			s_instance->m_pointLights.push_back(light);
			break;
		default:
			break;
		}
	}

	void LightLibrary::RemoveLight(LightComponent * light, LightType type)
	{
		switch (type)
		{
		case Lobster::DIRECTIONAL_LIGHT:
			s_instance->m_directionalLights.remove(light);
			break;
		case Lobster::POINT_LIGHT:
			s_instance->m_pointLights.remove(light);
			break;
		default:
			break;
		}
	}

	void LightLibrary::Update(const std::list<RenderCommand>& queue)
	{
		int i = 0; 
		for (auto dirLight : s_instance->m_directionalLights) {
			if (i >= MAX_DIRECTIONAL_SHADOW) break;
			dirLight->RenderDepthMap(queue);
			i++;
		}
		s_instance->SetUniforms();
	}

	void * LightLibrary::GetDirectionalShadowMap(int index)
	{
		if (index < s_instance->m_directionalLights.size()) {
			int i = 0;
			for (auto dirShadowMap : s_instance->m_directionalLights) {
				if (i == index) {
					return dirShadowMap->m_depthBuffer->Get(0);
				}
				i++;
			}
		}
		return nullptr;
	}

	void LightLibrary::SetUniforms()
	{
		// update light information
		ubo_DirectionalLight directionalLightsData[MAX_DIRECTIONAL_LIGHTS];
		int directionalLightCount = m_directionalLights.size();
		int i = 0; 
		for (auto dirLight : m_directionalLights) {
			directionalLightsData[i].color = dirLight->m_color;
			directionalLightsData[i].direction = dirLight->transform->WorldPosition;
			directionalLightsData[i].intensity = dirLight->m_intensity;
			i++;
		}
		ubo_PointLight pointLightsData[MAX_POINT_LIGHTS];
		int pointLightCount = m_pointLights.size();
		i = 0;
		for (auto pointLight : m_pointLights) {
			pointLightsData[i].color = pointLight->m_color;
			pointLightsData[i].position = pointLight->transform->WorldPosition;
			pointLightsData[i].attenuation = pointLight->m_intensity;
			i++;
		}

		// update all lightSpaceMatrix
		glm::mat4 lightSpaceMatrix[MAX_DIRECTIONAL_SHADOW];
		i = 0;
		for (auto dirLight : m_directionalLights) {
			if (i >= MAX_DIRECTIONAL_SHADOW) break;
			lightSpaceMatrix[i] = dirLight->m_lightSpaceMatrix;
			i++;
		}

		size_t offset = 0;
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(ubo_DirectionalLight) * MAX_DIRECTIONAL_LIGHTS, directionalLightsData);
		offset += sizeof(ubo_DirectionalLight) * MAX_DIRECTIONAL_LIGHTS;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(ubo_PointLight) * MAX_POINT_LIGHTS, pointLightsData);
		offset += sizeof(ubo_PointLight) * MAX_POINT_LIGHTS;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::mat4) * MAX_DIRECTIONAL_SHADOW, lightSpaceMatrix);
		offset += sizeof(glm::mat4) * MAX_DIRECTIONAL_SHADOW;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &directionalLightCount);
		offset += sizeof(int);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &pointLightCount);
		offset += sizeof(int);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

}
