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

		m_spriteShader = ShaderLibrary::Use("shaders/Sprite.glsl");
		m_spriteMesh = MeshFactory::Sprite();

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
		Material* boundedMaterial = nullptr;
		for (std::list<RenderCommand>::iterator it = queue.begin(); it != queue.end(); ++it)
		{
			RenderCommand& command = *it;
			Material* useMaterial = command.UseMaterial;
			Shader* useShader = command.UseMaterial->GetShader();
			useShader = (useShader && useShader->CompileSuccess()) ? useShader : ShaderLibrary::Use("shaders/SolidColor.glsl");
			useShader->Bind();
			// Vertex shader uniforms
			useShader->SetUniform("sys_world", command.UseWorldTransform);
			useShader->SetUniform("sys_view", camera->GetViewMatrix());
			useShader->SetUniform("sys_projection", camera->GetProjectionMatrix());
			if (command.UseBoneTransforms) {
				useShader->SetUniform("sys_bones[0]", MAX_BONES, command.UseBoneTransforms);
				useShader->SetUniform("sys_animate", true);
			}
			else {
				useShader->SetUniform("sys_animate", false);
			}
			// Fragment shader uniforms
			useShader->SetUniform("sys_cameraPosition", camera->GetPosition());
			useShader->SetTextureCube(8, m_activeSceneEnvironment.Skybox->GetIrradiance());
			useShader->SetTextureCube(9, m_activeSceneEnvironment.Skybox->GetPrefilter());
			useShader->SetTexture2D(10, m_activeSceneEnvironment.Skybox->GetBRDF());
			useShader->SetUniform("sys_irradianceMap", 8);
			useShader->SetUniform("sys_prefilterMap", 9);
			useShader->SetUniform("sys_brdfLUTMap", 10);
			
			if (boundedMaterial != useMaterial) {
				useMaterial->SetUniforms();
				boundedMaterial = useMaterial;
			}

			command.UseVertexArray->Draw();
			//queue.pop_front();
		}
	}

	void Renderer::Render(CameraComponent* camera)
	{
		if (!camera) return;

		// =====================================================
		// [First Pass] Render the scene to frame buffer
		// Render order: Background -> Opaque -> Transparent(sorted) -> Overlay

		// Set renderer configurations
		FrameBuffer* renderTarget = camera->GetFrameBuffer();
		renderTarget->Bind();
		Renderer::Clear(0.2f, 0.3f, 0.3f);
		Renderer::SetFaceCulling(true);
		
		// Update lightings
		LightLibrary::SetUniforms();

		// Background
		if (m_activeSceneEnvironment.Skybox)
		{
			Renderer::SetFaceCulling(true, CULL_FRONT);
			Renderer::SetDepthTest(true, DEPTH_LEQUAL);
			m_skyboxShader->Bind();
			m_skyboxShader->SetUniform("sys_world", glm::translate(camera->GetPosition())); // camera position
			m_skyboxShader->SetUniform("sys_view", camera->GetViewMatrix());
			m_skyboxShader->SetUniform("sys_projection", camera->GetProjectionMatrix());
			m_skyboxShader->SetTextureCube(0, m_activeSceneEnvironment.Skybox->Get());
			m_skyboxShader->SetUniform("skybox", 0);
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
		m_spriteShader->Bind();
		for (std::list<RenderOverlayCommand>::iterator it = m_overlayQueue.begin(); it != m_overlayQueue.end(); ++it)
		{
			RenderOverlayCommand& command = *it;
			m_spriteShader->SetTexture2D(0, command.UseTexture->Get());
			glm::mat4 world = glm::mat4(1.0);
			world = glm::translate(world, glm::vec3(command.x, command.y, command.z));
			world = glm::scale(world, glm::vec3(command.w, command.h, 1.0f));

			m_spriteShader->SetUniform("sys_world", world);
			m_spriteShader->SetUniform("sys_projection", camera->GetOrthoMatrix());
			m_spriteMesh->Draw();
		}

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

	void Renderer::Submit(RenderOverlayCommand ocommand)
	{
		s_instance->m_overlayQueue.push_back(ocommand);
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
		s_instance->m_overlayQueue.clear();
	}

}
