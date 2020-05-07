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
    
    Renderer::Renderer() :
		b_deferredRendering(true),
		m_gBuffer(nullptr),
		b_ppBlur(false),
		b_ppSSR(false)
    {
		// Check if renderer already exists
		if (s_instance) {
			throw std::runtime_error("Renderer already exists!");
			return;
		}
		// Create frame buffer for second pass
        glm::ivec2 size = Application::GetInstance()->GetWindow()->GetSize();
		std::vector<RenderTargetDesc> desc(3);
		desc[0].internalFormat = Formats::RGBA16FFormat;
		desc[0].format = Formats::RGBAFormat;
		desc[0].type = Types::FloatType;
		desc[1].internalFormat = Formats::RGB16FFormat;
		desc[1].format = Formats::RGBFormat;
		desc[1].type = Types::FloatType;
		desc[2].format = Formats::RGBAFormat;
		desc[2].type = Types::UnsignedByteType;
		m_gBuffer = new FrameBuffer(size.x, size.y, desc);

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
		if (m_postProcessMesh) delete m_postProcessMesh;
		if (m_skyboxMesh) delete m_skyboxMesh;
		if (m_spriteMesh) delete m_spriteMesh;
		m_postProcessMesh = nullptr;
		m_skyboxMesh = nullptr;
		m_spriteMesh = nullptr;
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
			for (int i = 0; i < MAX_DIRECTIONAL_SHADOW; ++i) {
				useShader->SetTexture2D(11 + i, LightLibrary::GetDirectionalShadowMap(i));
				useShader->SetUniform(("sys_shadowMap["+std::to_string(i)+"]").c_str(), 11 + i);
			}
			useShader->SetUniform("sys_irradianceMap", 8);
			useShader->SetUniform("sys_prefilterMap", 9);
			useShader->SetUniform("sys_brdfLUTMap", 10);
			
			if (boundedMaterial != useMaterial) {
				useMaterial->SetUniforms(useShader);
				boundedMaterial = useMaterial;
			}

			command.UseVertexArray->Draw();
			//queue.pop_front();
		}
	}

	void Renderer::DrawDeferredQueue(CameraComponent * camera, std::list<RenderCommand>& queue)
	{
		// Geometry pass
		m_gBuffer->BindAndClear(ClearFlag::COLOR | ClearFlag::DEPTH);
		Shader* useShader = ShaderLibrary::Use("shaders/GBuffer.glsl");
		Material* boundedMaterial = nullptr;
		useShader->Bind();
		useShader->SetUniform("sys_view", camera->GetViewMatrix());
		useShader->SetUniform("sys_projection", camera->GetProjectionMatrix());
		useShader->SetUniform("sys_cameraPosition", camera->GetPosition());
		for (std::list<RenderCommand>::iterator it = queue.begin(); it != queue.end(); ++it) {
			RenderCommand& command = *it;
			Material* useMaterial = command.UseMaterial;
			// Vertex shader uniforms
			useShader->SetUniform("sys_world", command.UseWorldTransform);
			if (command.UseBoneTransforms) {
				useShader->SetUniform("sys_bones[0]", MAX_BONES, command.UseBoneTransforms);
				useShader->SetUniform("sys_animate", true);
			}
			else {
				useShader->SetUniform("sys_animate", false);
			}
			// Fragment shader uniforms
			if (boundedMaterial != useMaterial) {
 				useShader->SetTexture2D(2, nullptr); // placeholder
 				useShader->SetTexture2D(3, nullptr); // placeholder
 				useShader->SetTexture2D(4, nullptr); // placeholder
				useShader->SetUniform("MetallicMap", 2); // placeholder
				useShader->SetUniform("RoughnessMap", 3); // placeholder
				useShader->SetUniform("AmbientOcclusionMap", 4); // placeholder
				useMaterial->SetUniforms(useShader);
				boundedMaterial = useMaterial;
			}
			command.UseVertexArray->Draw();
		}
		m_gBuffer->Unbind();

		// Copy content of geometry's depth buffer to default frame buffer's depth buffer
		FrameBuffer* frameBuffer = camera->GetFrameBuffer();
		m_gBuffer->BindRead();
		frameBuffer->BindDraw();
		glm::ivec2 src_size = m_gBuffer->GetSize();
		glm::ivec2 dst_size = frameBuffer->GetSize();
		glBlitFramebuffer(0, 0, src_size.x, src_size.y, 0, 0, dst_size.x, dst_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		// Lighting Pass
		frameBuffer->Bind();
		useShader = ShaderLibrary::Use("shaders/LightingPass.glsl");
		useShader->Bind();
		useShader->SetTexture2D(0, m_gBuffer->Get(0));
		useShader->SetTexture2D(1, m_gBuffer->Get(1));
		useShader->SetTexture2D(2, m_gBuffer->Get(2));
		useShader->SetUniform("sys_gNormalDepth", 0);
		useShader->SetUniform("sys_gMetalRoughAO", 1);
		useShader->SetUniform("sys_gAlbedo", 2);
		useShader->SetUniform("sys_view", camera->GetViewMatrix());
		useShader->SetUniform("sys_projection", camera->GetProjectionMatrix());
		useShader->SetUniform("sys_cameraPosition", camera->GetPosition());
		for (int i = 0; i < MAX_DIRECTIONAL_SHADOW; ++i) {
			useShader->SetTexture2D(11 + i, LightLibrary::GetDirectionalShadowMap(i));
			useShader->SetUniform(("sys_shadowMap[" + std::to_string(i) + "]").c_str(), 11 + i);
		}
		useShader->SetTextureCube(8, m_activeSceneEnvironment.Skybox->GetIrradiance());
		useShader->SetTextureCube(9, m_activeSceneEnvironment.Skybox->GetPrefilter());
		useShader->SetTexture2D(10, m_activeSceneEnvironment.Skybox->GetBRDF());
		useShader->SetUniform("sys_irradianceMap", 8);
		useShader->SetUniform("sys_prefilterMap", 9);
		useShader->SetUniform("sys_brdfLUTMap", 10);

		Renderer::SetDepthTest(false);
		m_postProcessMesh->Draw();
		Renderer::SetDepthTest(true, DEPTH_LESS);
	}

	void Renderer::Render(CameraComponent* camera, bool debug)
	{
		if (!camera) return;

		// =====================================================
		// Render the scene to frame buffer
		// Render order: Background -> Opaque -> Transparent(sorted) -> Overlay

		// Set renderer configurations
		FrameBuffer* renderTarget = camera->GetFrameBuffer();
		renderTarget->BindAndClear(ClearFlag::COLOR | ClearFlag::DEPTH);
		Renderer::SetFaceCulling(true);

		// Opaque
		if (!b_deferredRendering) {
			Renderer::DrawQueue(camera, m_opaqueQueue);
		}
		else {
			Renderer::DrawDeferredQueue(camera, m_opaqueQueue);
		}
		// Background
		if (m_activeSceneEnvironment.Skybox) {
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
		// Transparent(sorted)		
		Renderer::SetAlphaBlend(true);
		glDepthMask(GL_FALSE);
		Renderer::DrawQueue(camera, m_transparentQueue);
		glDepthMask(GL_TRUE);
		Renderer::SetAlphaBlend(false);
		
		// Overlay-original
		// Debug
#ifdef LOBSTER_BUILD_EDITOR
		if (debug) {
			Renderer::DrawQueue(camera, m_debugQueue);
		}
#endif

		// Unset renderer configurations
		Renderer::SetFaceCulling(false);
		renderTarget->Unbind();

		// =====================================================
		// Render the stored frame buffer in rect
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Renderer::SetDepthTest(false);
		m_postProcessShader->Bind();
		m_postProcessShader->SetTexture2D(0, renderTarget->Get(0));
		m_postProcessShader->SetTexture2D(1, m_gBuffer->Get(0));
		m_postProcessShader->SetUniform("sys_cameraPosition", camera->GetPosition());
		m_postProcessShader->SetUniform("sys_view", camera->GetViewMatrix());
		m_postProcessShader->SetUniform("sys_projection", camera->GetProjectionMatrix());
		m_postProcessShader->SetUniform("screenTexture", 0);
		m_postProcessShader->SetUniform("sys_gNormalDepth", 1);
		m_postProcessShader->SetUniform("sys_ppBlur", b_ppBlur);
		m_postProcessShader->SetUniform("sys_ppSSR", b_ppSSR);
		m_postProcessShader->SetUniform("sys_ppUseKernel", b_ppUseKernel);			
		m_postProcessShader->SetUniform("sys_ppKernel", m_ppKernel);
		m_postProcessShader->SetUniform("sys_ppBlend", b_ppBlend);
		m_postProcessShader->SetUniform("sys_ppBlendColor", m_ppBlendColor);
		m_postProcessShader->SetUniform("sys_ppSobel", b_ppSobel);
		m_postProcessShader->SetUniform("sys_ppSobelThreshold", m_ppSobelThreshold);
		m_postProcessMesh->Draw();
		Renderer::SetDepthTest(true);

		// from front to back order
		// Overlay
		m_spriteShader->Bind();
		for (auto it = m_overlayQueue.rbegin(); it != m_overlayQueue.rend(); ++it)
		{
			RenderOverlayCommand& command = *it;
			glm::mat4 world = glm::mat4(1.0);
			world = glm::translate(world, glm::vec3(command.x, command.y, command.z));
			world = glm::scale(world, glm::vec3(command.w, command.h, 1.0f));
			m_spriteShader->SetTexture2D(0, renderTarget->Get(0));
			m_spriteShader->SetTexture2D(1, command.UseTexture->Get());
			m_spriteShader->SetUniform("sys_world", world);
			m_spriteShader->SetUniform("sys_projection", camera->GetOrthoMatrix());
			m_spriteShader->SetUniform("alpha", command.alpha);
			m_spriteShader->SetUniform("sys_background", 0);
			m_spriteShader->SetUniform("sys_spriteTexture", 1);
			m_spriteShader->SetUniform("blend", glm::vec4(command.blendR, command.blendG, command.blendB, command.blendA));
			m_spriteMesh->Draw();
		}

		glm::ivec2 buf_size = renderTarget->GetSize();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		renderTarget->BindDraw();
		glBlitFramebuffer(0, 0, buf_size.x, buf_size.y, 0, 0, buf_size.x, buf_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

	void Renderer::BeginScene(TextureCube * skybox)
	{
		// set global environment
		s_instance->m_activeSceneEnvironment.Skybox = skybox;
	}

	void Renderer::SetApplySobel(bool apply, float threshold) {
		s_instance->b_ppSobel = apply;
		s_instance->m_ppSobelThreshold = threshold;
	}

	void Renderer::SetApplyKernel(bool apply, glm::mat3 kernel) {
		s_instance->b_ppUseKernel = apply;
		s_instance->m_ppKernel = kernel;
	}

	void Renderer::SetBlur(bool blur) {
		s_instance->b_ppBlur = blur;
	}

	void Renderer::SetSSR(bool ssr) {
		s_instance->b_ppSSR = ssr;
	}

	void Renderer::SetBlend(bool blend, glm::vec3 color, float alpha) {
		s_instance->b_ppBlend = blend;
		if (blend)
			s_instance->m_ppBlendColor = glm::vec4(color, alpha);
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

	void Renderer::SubmitDebug(RenderCommand dcommand)
	{
		s_instance->m_debugQueue.push_back(dcommand);
	}

	void Renderer::EndScene()
	{
		// do all batching and sorting job here
	}

	void Renderer::ClearOverlayQueue() {
		s_instance->m_overlayQueue.clear();
	}

	void Renderer::ClearAllQueues()
	{
		// remove all previous render commands
		s_instance->m_opaqueQueue.clear();
		s_instance->m_transparentQueue.clear();
		s_instance->m_overlayQueue.clear();
		s_instance->m_debugQueue.clear();
	}

	void Renderer::OnImGuiRender()
	{
		ImGui::Text("Render Pipeline");
		ImGui::Checkbox("Deferred Rendering", &s_instance->b_deferredRendering);
		ImGui::Separator();
		ImGui::Text("Post Processing");
		ImGui::Checkbox("Blur", &s_instance->b_ppBlur);
		ImGui::Checkbox("Screen Space Reflection", &s_instance->b_ppSSR);
		ImGui::Checkbox("Sobel Edge Detection", &s_instance->b_ppSobel);
		ImGui::Checkbox("Color Blend", &s_instance->b_ppBlend);
		if (ImGui::Button("Clear Applied Filters")) {
			s_instance->b_ppUseKernel = false;
		}
	}

}
