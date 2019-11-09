#include "pch.h"
#include "Application.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Renderer.h"
#include "Scene.h"
#include "components/ComponentCollection.h"
#include "graphics/meshes/MeshFactory.h"
#include "objects/GameObject.h"

namespace Lobster
{

	Renderer* Renderer::s_instance = nullptr;
    
    Renderer::Renderer()
    {
		// Check if renderer already exists
		if (s_instance)
		{
			throw std::runtime_error("Renderer already exists!");
			return;
		}
		// Create frame buffer for second pass
        glm::ivec2 size = Application::GetInstance()->GetWindow()->GetSize();

		m_postProcessShader = ShaderLibrary::Use("shaders/PostProcessing.glsl");
		m_postProcessMesh = MeshFactory::Plane();

		m_skyboxShader = ShaderLibrary::Use("shaders/Skybox.glsl");
		m_skyboxMesh = MeshFactory::Cube();

		s_instance = this;
    }
    
    Renderer::~Renderer()
    {
        
    }

	void Renderer::SetDepthTest(bool enabled, DepthFunc func)
	{
		if (enabled) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(func);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
	}

	void Renderer::SetAlphaBlend(bool enabled, BlendFactor factor)
	{
		if (enabled) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, factor);
		}
		else {
			glDisable(GL_BLEND);
		}
	}

	void Renderer::SetFaceCulling(bool enabled, CullMode mode)
	{
		if (enabled) {
			glEnable(GL_CULL_FACE);
			glCullFace(mode);
		}
		else {
			glDisable(GL_CULL_FACE);
		}
	}
    
	void Renderer::DrawQueue(CameraComponent* camera, std::list<RenderCommand>& queue)
	{
		for (std::list<RenderCommand>::iterator it = queue.begin(); it != queue.end(); ++it)
		{
			RenderCommand& command = *it;
			Material* useMaterial = command.UseMaterial;
			Shader* useShader = command.UseMaterial->GetShader();
			useShader = (useShader && useShader->CompileSuccess()) ? useShader : ShaderLibrary::Use("shaders/SolidColor.glsl");
			useShader->Bind();
			useShader->SetUniform("world", command.UseWorldTransform);
			useShader->SetUniform("view", camera->GetViewMatrix());
			useShader->SetUniform("projection", camera->GetProjectionMatrix());
			useShader->SetUniform("cameraPosition", camera->GetPosition());
			useShader->SetUniform("lightPosition", glm::vec3(0.0, 2.0, 3.0));
			useShader->SetUniform("lightDirection", glm::normalize(glm::vec3(0.0, -2.0, -3.0)));
			useShader->SetUniform("lightColor", glm::vec4(1.0, 1.0, 1.0, 1.0));

			for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
			{
				void* textureID = (useMaterial->GetTextureUnit(i) == nullptr) ? 0 : useMaterial->GetTextureUnit(i)->Get();
				useShader->SetTexture2D(i, textureID);
			}
			for (int i = 0; i < MAX_UNIFORM_BUFFER; ++i)
			{
				if (useMaterial->GetUniformBufferData(i) == nullptr) continue;
				useMaterial->GetUniformBufferData(i)->BindData();
			}

			command.UseVertexArray->Draw();

			//queue.pop_front();
		}
	}

	void Renderer::Render(CameraComponent* camera)
	{
		// =====================================================
		// [First Pass] Render the scene to frame buffer
		// Render order: Background -> Opaque -> Transparent(sorted) -> Overlay

		// Set renderer configurations
		FrameBuffer* renderTarget = camera->GetFrameBuffer();
		renderTarget->Bind();
		Renderer::Clear(0.2f, 0.3f, 0.3f);
		Renderer::SetFaceCulling(true);

		// Background
		if (m_activeSceneEnvironment.Skybox)
		{
			Renderer::SetFaceCulling(true, CULL_FRONT);
			Renderer::SetDepthTest(true, DEPTH_LEQUAL);
			m_skyboxShader->Bind();
			m_skyboxShader->SetUniform("world", glm::translate(camera->GetPosition())); // camera position
			m_skyboxShader->SetUniform("view", camera->GetViewMatrix());
			m_skyboxShader->SetUniform("projection", camera->GetProjectionMatrix());
			m_skyboxShader->SetTextureCube(0, m_activeSceneEnvironment.Skybox->Get());
			m_skyboxMesh->Draw();
			Renderer::SetDepthTest(true, DEPTH_LESS);
			Renderer::SetFaceCulling(true, CULL_BACK);
		}

		// Opaque
		Renderer::DrawQueue(camera, m_opaqueQueue);
		// Transparent(sorted)
		Renderer::SetAlphaBlend(true);
		Renderer::DrawQueue(camera, m_transparentQueue);
		Renderer::SetAlphaBlend(false);
		// Overlay

		// Unset renderer configurations
		Renderer::SetFaceCulling(false);

		renderTarget->Unbind();

		// =====================================================
		// [Second Pass] Render the stored frame buffer in rect
		Renderer::Clear(0.1f, 0.2f, 0.3f);
		Renderer::SetDepthTest(false);
		m_postProcessShader->Bind();
		m_postProcessShader->SetTexture2D(0, renderTarget->Get());
		m_postProcessMesh->Draw();
		Renderer::SetDepthTest(true);
    }

	void Renderer::Clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::BeginScene(TextureCube * skybox)
	{
		// set global environment
		s_instance->m_activeSceneEnvironment.Skybox = skybox;
	}

	void Renderer::Submit(RenderCommand command)
	{
		Material* material = command.UseMaterial;
		if (material == nullptr) {
			throw std::runtime_error("What happened? Why there's a command without material?");
		}
		switch (material->GetRenderingMode())
		{
		case RenderingMode::MODE_OPAQUE:
			s_instance->m_opaqueQueue.push_back(command);	break;
		case RenderingMode::MODE_TRANSPARENT:
			s_instance->m_transparentQueue.push_back(command);	break;
		}
	}

	void Renderer::EndScene()
	{
		// do all batching and sorting job here
	}

	void Renderer::ClearAllQueues()
	{
		// remove all previous render commands
		s_instance->m_opaqueQueue.clear();
		s_instance->m_transparentQueue.clear();
	}

}
