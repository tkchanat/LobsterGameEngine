#pragma once
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/UniformBuffer.h"

namespace Lobster
{

	class CameraComponent;
    class FrameBuffer;
	class Material;
	class Scene;
	class VertexArray;

	struct RenderCommand
	{
		Material* UseMaterial;
		VertexArray* UseVertexArray;
		glm::mat4 UseWorldTransform;

		RenderCommand() { memset(this, 0, sizeof(RenderCommand)); }
	};

	struct SceneEnvironment
	{
		glm::vec3 CameraPosition;
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		TextureCube* Skybox;
	};
    
	//	This class is specifically for rendering scene.
	//	The notion of using a renderer class for scene drawing is because we want to customize the drawing order or other techniques.
	//	For example, forward rendering vs. deferred rendering.
	//	Each rendering pipeline would have different configuration or even resources like additional geometric buffers.
	//	Therefore, renderer should definitely deserve a separate class for an application / game.
    class Renderer
    {
		friend class Application; // placeholder
		friend class ImGuiScene;
	private:
		// post processing resources
		FrameBuffer* m_postProcessFrameBuffer;
		VertexArray* m_postProcessMesh;
		Shader* m_postProcessShader;
		// skybox resources
		VertexArray* m_skyboxMesh;
		Shader* m_skyboxShader;

		static SceneEnvironment s_activeSceneEnvironment;
		static std::list<RenderCommand> s_renderQueue;
    public:
        Renderer();
        ~Renderer();
		static void BeginScene(CameraComponent* camera, TextureCube* skybox);
		static void Submit(RenderCommand command);
		static void EndScene();
	private:
        void Render(Scene* scene);
		void Clear(float r, float g, float b);
    };
    
}
