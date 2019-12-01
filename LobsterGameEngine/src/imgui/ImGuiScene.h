#pragma once
#include "ImGuiComponent.h"
#include "ImGuiProperties.h"
#include "graphics/Material.h"
#include "graphics/Scene.h"
#include "graphics/Renderer.h"
#include "graphics/FrameBuffer.h"
#include "graphics/meshes/MeshFactory.h"
#include "system/Input.h"
#include "system/Profiler.h"
#include "system/UndoSystem.h"
#include <ImGuizmo.h>

namespace Lobster
{

	struct GizmosCommand {
		std::string texture;
		glm::vec3 position;
		ImVec2 size = ImVec2(32, 32);
	};

	class ImGuiScene : public ImGuiComponent
	{
	private:
		// window-related variable
		ImVec2 window_pos;
		ImVec2 window_size;
		// editor camera
		GameObject* m_editorCamera;
		// ImGuizmo
		ImGuizmo::OPERATION m_operation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_mode = ImGuizmo::LOCAL;
		glm::vec3 m_originalScale;
		// Custom gizmos
		static std::list<GizmosCommand> m_gizmosQueue;
		// not the owner of these objects, don't delete
		Scene* m_scene;
		Renderer* m_renderer;
		// grid line
		bool b_showGrid;
		glm::vec4 m_gridColor;
		Material* m_gridMaterial;
		VertexArray* m_gridVertexArray;
		// profiler
		bool b_showProfiler;
		// boolean to indicate whether we are moving object in previous frame
		bool b_isMoving = false;
		// transform object storing previous state of the game object prior to move
		Transform m_transform;
	public:
		explicit ImGuiScene(Scene* scene, Renderer* renderer) :
			m_editorCamera(nullptr),
			m_scene(scene),
			m_renderer(renderer),
			b_showGrid(true),
			m_gridColor(glm::vec4(1.0)),
			m_gridMaterial(nullptr),
			m_gridVertexArray(nullptr),
			b_showProfiler(true)
		{
			m_editorCamera = (new GameObject("EditorCamera"))->AddComponent<CameraComponent>(ProjectionType::PERSPECTIVE);
			m_editorCamera->transform.Translate(10, 8, 10);
			m_editorCamera->transform.LookAt(glm::vec3(0, 0, 0));

			m_gridMaterial = MaterialLibrary::UseShader("shaders/SolidColor.glsl");
			m_gridMaterial->SetRawUniform("color", glm::value_ptr(m_gridColor));
			m_gridVertexArray = MeshFactory::Grid(20, 20);

			// listen events
			EventDispatcher::AddCallback(EVENT_KEY_PRESSED, new EventCallback<KeyPressedEvent>([this](KeyPressedEvent* e) {
				if (e->Key == GLFW_KEY_P)
				{
					b_showProfiler = !b_showProfiler;
				}
			}));
		}
		
		inline CameraComponent* GetCamera() { return m_editorCamera->GetComponent<CameraComponent>(); }
		inline static void SubmitGizmos(GizmosCommand command) { m_gizmosQueue.push_back(command); }

		// Check if moues is inside the "scene" window 
		bool insideWindow(const ImVec2& mouse, const ImVec2& pos, const ImVec2& size) {
			if (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y)
				return true;
			return false;
		}

		~ImGuiScene()
		{
			if (m_editorCamera) delete m_editorCamera;
			if (m_scene) delete m_scene;
			if (m_renderer) delete m_renderer;
			if (m_gridMaterial) delete m_gridMaterial;
			if (m_gridVertexArray) delete m_gridVertexArray;
			m_editorCamera = nullptr;
			m_scene = nullptr;
			m_renderer = nullptr;
			m_gridMaterial = nullptr;
			m_gridVertexArray = nullptr;
		}

		void OnUpdate(double deltaTime)
		{
			// grid line rendering
			if (b_showGrid) 
			{
				RenderCommand command;
				command.UseMaterial = m_gridMaterial;
				command.UseVertexArray = m_gridVertexArray;
				command.UseWorldTransform = glm::mat4(1.0f);
				Renderer::Submit(command);
			}
		}

		virtual void Show(bool* p_open) override
		{			
			// ====================================================
			// Scene window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Scene", p_open, ImGuiWindowFlags_None);
			window_pos = ImGui::GetWindowPos();
			window_size = ImGui::GetWindowSize();

			// right-click settings pop up
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
			if (ImGui::BeginPopupContextItem()) {
				ImGui::Checkbox("Show Reference Grid", &b_showGrid);
				ImGui::Separator();
				ImGui::Text("Grid Color");
				ImGui::ColorEdit3("", glm::value_ptr(m_gridColor), 0);
				m_gridMaterial->SetRawUniform("color", glm::value_ptr(m_gridColor));
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			// draw scene
			CameraComponent* camera = m_editorCamera->GetComponent<CameraComponent>();
			camera->ResizeProjection(window_size.x / window_size.y);
			void* image = camera->GetFrameBuffer()->Get();
			ImGui::GetWindowDrawList()->AddImage(image, ImVec2(window_pos.x, window_pos.y), ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImVec2(0, 1), ImVec2(1, 0));
			
			DrawCustomGizmos();

			// Control the camera ONLY IF window is focused and mouse on the window
			{
				float deltaTime = ImGui::GetIO().DeltaTime;
				if (ImGui::IsWindowFocused() && insideWindow(ImGui::GetIO().MousePos, ImGui::GetWindowPos(), ImGui::GetWindowSize()))
				{
					glm::vec2 lastScroll = Input::GetLastScroll();
					glm::vec2 mouseDelta = Input::GetMouseDelta();
					// Free Look
					if (Input::IsKeyDown(GLFW_KEY_SPACE) && Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
					{
						m_editorCamera->transform.RotateEuler(-mouseDelta.y * 10.0f * deltaTime, m_editorCamera->transform.Right());
						m_editorCamera->transform.RotateEuler(-mouseDelta.x * 10.0f * deltaTime, glm::vec3(0, 1, 0));
					}
					// Rotate and Look-At
					else if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
						float dx = -mouseDelta.x * deltaTime * 10.f;
						float dy = -mouseDelta.y * deltaTime * 10.f;
						m_editorCamera->transform.RotateAround(dy, m_editorCamera->transform.Right(), glm::vec3(0, 0, 0));
						m_editorCamera->transform.RotateAround(dx, glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));
					}
					// Pan
					else if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
						glm::vec3 pan = -mouseDelta.x * m_editorCamera->transform.Right() + mouseDelta.y * m_editorCamera->transform.Up();
						m_editorCamera->transform.WorldPosition += pan * deltaTime * 2.0f;
					}
					// Zoom
					else if (lastScroll.y != 0 && insideWindow(ImGui::GetIO().MousePos, ImGui::GetWindowPos(), ImGui::GetWindowSize())) {
						glm::vec3 zoom = -lastScroll.y * m_editorCamera->transform.Forward();
						m_editorCamera->transform.WorldPosition += zoom * 50.0f * deltaTime;					
					}
				}
			}

			// ImGuizmo
			{
				GameObject* gameObject = ImGuiProperties::selectedObj;
				if (gameObject)
				{
					glm::mat4 transformMatrix = gameObject->transform.GetMatrix();
					ImGuizmo::SetDrawlist();
					ImGuiIO& io = ImGui::GetIO();
					ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);
					if (Input::IsKeyDown(GLFW_KEY_W))
						m_operation = ImGuizmo::TRANSLATE;
					else if (Input::IsKeyDown(GLFW_KEY_E))
						m_operation = ImGuizmo::ROTATE;
					else if (Input::IsKeyDown(GLFW_KEY_R))
						m_operation = ImGuizmo::SCALE;
					else if (Input::IsKeyDown(GLFW_KEY_Q))
						m_operation = ImGuizmo::BOUNDS;
					if (Input::IsKeyDown(GLFW_KEY_MINUS))
						m_mode = ImGuizmo::LOCAL;
					else if (Input::IsKeyDown(GLFW_KEY_EQUAL))
						m_mode = ImGuizmo::WORLD;
					glm::mat4 deltaMatrix;
					glm::vec3 deltaTranslation;
					glm::vec3 deltaRotation;
					glm::vec3 deltaScale;
					ImGuizmo::Manipulate(glm::value_ptr(camera->GetViewMatrix()), glm::value_ptr(camera->GetProjectionMatrix()), m_operation, m_mode, glm::value_ptr(transformMatrix), glm::value_ptr(deltaMatrix));
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(deltaMatrix), glm::value_ptr(deltaTranslation), glm::value_ptr(deltaRotation), glm::value_ptr(deltaScale));
					switch (m_operation)
					{
					case ImGuizmo::TRANSLATE:
						gameObject->transform.WorldPosition += deltaTranslation; break;
					case ImGuizmo::ROTATE:
						gameObject->transform.RotateEuler(deltaRotation.x, glm::vec3(1, 0, 0));
						gameObject->transform.RotateEuler(deltaRotation.y, glm::vec3(0, 1, 0));
						gameObject->transform.RotateEuler(deltaRotation.z, glm::vec3(0, 0, 1)); break;
					case ImGuizmo::SCALE:
						if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) m_originalScale = gameObject->transform.LocalScale;
						else gameObject->transform.LocalScale = m_originalScale + deltaScale - glm::vec3(1); break;
					}

					// check with IsUsing if we are trying to edit an item.
					bool isMoving = ImGuizmo::IsUsing();

					// store the item transform details if we plan to move (ie: mouse over) but hadn't move yet.
					if (!b_isMoving && !isMoving && ImGuizmo::IsOver()) {
						m_transform = gameObject->transform;
					}
					
					// keep track of whether we are moving, and when we stopped moving, send an undo event.
					if (!b_isMoving && isMoving) {
						b_isMoving = true;
					} else if (b_isMoving && !isMoving) {
						b_isMoving = false;

						// only send event if transform is updated.
						if (m_transform.GetMatrix() != gameObject->transform.GetMatrix()) {
							UndoSystem::GetInstance()->Push(new TransformCommand(gameObject, m_transform, gameObject->transform));
						}
					}
				}
				
				ImGui::End();
				ImGui::PopStyleVar();
			}

			// ====================================================
			// Profiling overlay
			if(b_showProfiler)
			{
				float margin = 10.f;
				ImGui::SetNextWindowPos(ImVec2(window_pos.x + margin, window_pos.y + window_size.y - margin), 0, ImVec2(0, 1));
				ImGui::SetNextWindowSize(ImVec2(280, 120));
				ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				if (ImGui::Begin("Performance Profiler", p_open,
					ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking))
				{
					ImGui::Text("Performance Profiler ([P] Show/Hide)");
					ImGui::Separator();
					for (auto it = Profiler::s_instance->m_profilerData.begin(); it != Profiler::s_instance->m_profilerData.end(); ++it)
					{
						std::string label = it->first + ": %.1f ms";
						ImGui::Text(label.c_str(), it->second);
					}
				}
				ImGui::End();
				ImGui::PopStyleVar();
			}

		}

		private:
			// draw camera gizmo
			void DrawCustomGizmos()
			{
				// get view and projection matrix
				CameraComponent* editorCamera = m_editorCamera->GetComponent<CameraComponent>();
				glm::mat4 viewProjectionMatrix = editorCamera->GetProjectionMatrix() * editorCamera->GetViewMatrix();
				for (std::list<GizmosCommand>::iterator it = m_gizmosQueue.begin(); it != m_gizmosQueue.end(); ++it)
				{
					GizmosCommand& command = *it;
					// select texture and specify world position
					void* customGizmo = TextureLibrary::Use(command.texture.c_str())->Get();
					glm::vec4 pos = viewProjectionMatrix * glm::vec4(command.position, 1);
					ImVec2 size = command.size;
					m_gizmosQueue.pop_front(); // already extract all data, remove from queue
					if (pos.w <= 0.0) continue;
					constexpr auto remap = [](float value, float start1, float stop1, float start2, float stop2) {
						return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
					};
					float screenX = remap(pos.x / pos.w, -1.f, 1.f, window_pos.x, window_pos.x + window_size.x);
					float screenY = remap(-pos.y / pos.w, -1.f, 1.f, window_pos.y, window_pos.y + window_size.y);
					ImVec2 startPos = { screenX - size.x / 2.f, screenY - size.y / 2.f };
					ImGui::GetWindowDrawList()->AddImage(customGizmo, startPos, ImVec2(startPos.x + size.x, startPos.y + size.y));
				}
			}

	};

}
