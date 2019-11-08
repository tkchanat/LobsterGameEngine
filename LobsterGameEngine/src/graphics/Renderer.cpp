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

	std::list<RenderCommand> Renderer::s_renderQueue;
	SceneEnvironment Renderer::s_activeSceneEnvironment;
    
    Renderer::Renderer()
    {
		// Create frame buffer for second pass
        glm::ivec2 size = Application::GetInstance()->GetWindow()->GetSize();

		m_postProcessFrameBuffer = new FrameBuffer(size.x, size.y);
		m_postProcessShader = ShaderLibrary::Use("shaders/PostProcessing.glsl");
		m_postProcessMesh = MeshFactory::Plane();

		m_skyboxShader = ShaderLibrary::Use("shaders/Skybox.glsl");
		m_skyboxMesh = MeshFactory::Cube();
    }
    
    Renderer::~Renderer()
    {
        
    }
    
    void Renderer::Render(Scene* scene)
    {
		// =====================================================
		// [First Pass] Render the scene to frame buffer
		m_postProcessFrameBuffer->Bind();
		Renderer::Clear(0.2f, 0.3f, 0.3f);
        
        // Draw all objects with mesh component accordingly
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
        for(int i = 0; i < s_renderQueue.size(); ++i)
        {
			RenderCommand& command = s_renderQueue.front();
			Material* useMaterial = command.UseMaterial;
			Shader* useShader = command.UseMaterial->GetShader();
			useShader = (useShader && useShader->CompileSuccess()) ? useShader : ShaderLibrary::Use("shaders/SolidColor.glsl");
			useShader->Bind();
			useShader->SetUniform("world", command.UseWorldTransform);
			useShader->SetUniform("view", s_activeSceneEnvironment.ViewMatrix);
			useShader->SetUniform("projection", s_activeSceneEnvironment.ProjectionMatrix);
			useShader->SetUniform("cameraPosition", s_activeSceneEnvironment.CameraPosition);
            useShader->SetUniform("lightPosition", glm::vec3(0.0, 2.0, 3.0));
			useShader->SetUniform("lightDirection", glm::normalize(glm::vec3(0.0, -2.0, -3.0)));
			useShader->SetUniform("lightColor", glm::vec3(1.0, 1.0, 1.0));

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
			
			s_renderQueue.pop_front();
        }
		glDisable(GL_CULL_FACE);

		// Fill the rest spaces with skybox
		if (s_activeSceneEnvironment.Skybox)
		{
			glDepthFunc(GL_LEQUAL);
			m_skyboxShader->Bind();
			m_skyboxShader->SetUniform("world", glm::translate(s_activeSceneEnvironment.CameraPosition)); // camera position
			m_skyboxShader->SetUniform("view", s_activeSceneEnvironment.ViewMatrix);
			m_skyboxShader->SetUniform("projection", s_activeSceneEnvironment.ProjectionMatrix);
			m_skyboxShader->SetTextureCube(0, s_activeSceneEnvironment.Skybox->Get());
			m_skyboxMesh->Draw();
			glDepthFunc(GL_LESS);
		}

		m_postProcessFrameBuffer->Unbind();

		// =====================================================
		// [Second Pass] Render the stored frame buffer in rect
		Renderer::Clear(0.1f, 0.2f, 0.3f);
		glDisable(GL_DEPTH_TEST);

		m_postProcessShader->Bind();
		m_postProcessShader->SetTexture2D(0, m_postProcessFrameBuffer->Get());
		m_postProcessMesh->Draw();

		glEnable(GL_DEPTH_TEST);
    }

	void Renderer::Clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::BeginScene(CameraComponent * camera, TextureCube * skybox)
	{
		if (!camera) return;
		s_activeSceneEnvironment.CameraPosition = camera->GetPosition();
		s_activeSceneEnvironment.ViewMatrix = camera->GetViewMatrix();
		s_activeSceneEnvironment.ProjectionMatrix = camera->GetProjectionMatrix();
		s_activeSceneEnvironment.Skybox = skybox;
	}

	void Renderer::Submit(RenderCommand command)
	{
		if (command.UseMaterial == nullptr)
		{
			throw std::runtime_error("What happened? Why there's a command without material?");
		}
		s_renderQueue.push_back(command);
	}

	void Renderer::EndScene()
	{
		// do all batching and sorting job here
	}

}
