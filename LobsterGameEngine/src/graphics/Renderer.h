#pragma once
#include "graphics/Material.h"

namespace Lobster
{

	class CameraComponent;
    class FrameBuffer;
	class Scene;
	class VertexArray;

	struct RenderCommand
	{
		Material* UseMaterial = nullptr;
		VertexArray* UseVertexArray = nullptr;
		glm::mat4 UseWorldTransform = glm::mat4(1.0);
		glm::mat4* UseBoneTransforms = nullptr;
	};

	struct RenderOverlayCommand
	{
		enum OverlayType { Image, Text };
		Texture2D* UseTexture;
		OverlayType type;
		float x, y;
		float w, h;
		float alpha;
		int z;
		RenderOverlayCommand() { memset(this, 0, sizeof(RenderOverlayCommand)); }
	};

	struct SceneEnvironment
	{
		TextureCube* Skybox;
	};

	enum BlendFactor : uint 
	{
		BLEND_ZERO = GL_ZERO,
		BLEND_ONE = GL_ONE,
		BLEND_SRC_COLOR = GL_SRC_COLOR,
		BLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
		BLEND_DST_COLOR = GL_DST_COLOR,
		BLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
		BLEND_SRC_ALPHA = GL_SRC_ALPHA,
		BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
		BLEND_DST_ALPHA = GL_DST_ALPHA,
		BLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
		BLEND_CONSTANT_COLOR = GL_CONSTANT_COLOR,
		BLEND_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
		BLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
		BLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA
	};

	enum CullMode : uint
	{
		CULL_FRONT = GL_FRONT,
		CULL_BACK = GL_BACK,
		CULL_FRONT_AND_BACK = GL_FRONT_AND_BACK
	};

	enum DepthFunc : uint
	{
		DEPTH_ALWAYS = GL_ALWAYS,
		DEPTH_NEVER = GL_NEVER,
		DEPTH_LESS = GL_LESS,
		DEPTH_EQUAL = GL_EQUAL,
		DEPTH_LEQUAL = GL_LEQUAL,
		DEPTH_GREATER = GL_GREATER,
		DEPTH_NOTEQUAL = GL_NOTEQUAL,
		DEPTH_GEQUAL = GL_GEQUAL
	};
    
	//	This class is specifically for rendering scene.
	//	The notion of using a renderer class for scene drawing is because we want to customize the drawing order or other techniques.
	//	For example, forward rendering vs. deferred rendering.
	//	Each rendering pipeline would have different configuration or even resources like additional geometric buffers.
	//	Therefore, renderer should definitely deserve a separate class for an application / game.
    class Renderer
    {
		friend class Application; // placeholder
	private:
		// post processing resources
		VertexArray* m_postProcessMesh;
		Shader* m_postProcessShader;
		// skybox resources
		VertexArray* m_skyboxMesh;
		Shader* m_skyboxShader;
		// sprite resources
		VertexArray* m_spriteMesh;
		Shader* m_spriteShader;

		// renderer resources
		static Renderer* s_instance;
		SceneEnvironment m_activeSceneEnvironment;
		std::list<RenderCommand> m_opaqueQueue;
		std::list<RenderCommand> m_transparentQueue;
		std::list<RenderCommand> m_debugQueue;
		std::list<RenderOverlayCommand> m_overlayQueue;
    public:
        Renderer();
        ~Renderer();
		// Renderer & Rasterization configurations
		static void SetDepthTest(bool enabled, DepthFunc func = DEPTH_LESS);
		static void SetAlphaBlend(bool enabled, BlendFactor factor = BLEND_ONE_MINUS_SRC_ALPHA);
		static void SetFaceCulling(bool enabled, CullMode mode = CULL_BACK);
		// RenderCommand & RenderQueue
		static void BeginScene(TextureCube* skybox);
		static void Submit(RenderCommand command);
		static void Submit(RenderOverlayCommand ocommand);
		static void SubmitDebug(RenderCommand dcommand);
		static void EndScene();
		static void ClearOverlayQueue();
		static void ClearAllQueues();
	private:
		void DrawQueue(CameraComponent* camera, std::list<RenderCommand>& queue);
        void Render(CameraComponent* camera, bool debug = false);
    };
    
}
