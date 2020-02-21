#include "pch.h"
#include "Script.h"
#include "graphics/Scene.h"
#include "system/Input.h"
#include <LuaBridge/Vector.h>

using namespace luabridge;

namespace Lobster {

	Script::Script() : Component(SCRIPT_COMPONENT) {
		L = luaL_newstate();
		luaL_openlibs(L);		
	}

	Script::Script(const char* file) : Component(SCRIPT_COMPONENT) {
		L = luaL_newstate();
		luaL_openlibs(L);
		loadScript(file);
	}

	Script::~Script() {
		lua_close(L);
		L = nullptr;
	}

	void Script::loadScript(const char* file) {
		filename = file;
		errmsg.clear();
		int r = luaL_dofile(L, FileSystem::Path(FileSystem::Join(PATH_SCRIPTS, filename)).c_str());
		if (r != LUA_OK) {
			WARN(lua_tostring(L, -1));
			errmsg += lua_tostring(L, -1);
			errmsg += "\n";
		}
		Bind();
	}

	void Script::OnImGuiRender() {
		// combo box of scripts
		if (ImGui::CollapsingHeader("Script", &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginCombo("Source", filename.c_str())) {
				// None
				if (ImGui::Selectable("None", filename.size() == 0)) {
					filename.clear();
				}
				// load scripts in resources
				fs::path subdir = FileSystem::Path(PATH_SCRIPTS);
				for (const auto& dirEntry : fs::directory_iterator(subdir)) {
					if (dirEntry.is_directory()) continue; // not to display folder
					std::string displayName = dirEntry.path().filename().string();
					if (ImGui::Selectable(displayName.c_str(), filename == displayName)) {
						loadScript(displayName.c_str());
					}
				}
				ImGui::EndCombo();
			}
			// script status
			ImGui::Text("Status");
			ImGui::BeginChild("Script Status: ", ImVec2(0, 100), true);
			if (filename.size() > 0) {
				if (errmsg.size() == 0) {
					ImGui::PushStyleColor(0, ImVec4(0.117f, 0.843f, 0.376f, 1.0f));
					ImGui::Text("OK");
					ImGui::PopStyleColor();
				}
				else {
					ImGui::PushStyleColor(0, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					ImGui::TextWrapped(errmsg.c_str());
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndChild();
		}
	}

	void Script::Execute(std::string funcName) {
		if (Application::GetMode() != GAME) return;
		if (filename.size() == 0 || errmsg.size() > 0) return;
		LuaRef lua_Func = getGlobal(L, funcName.c_str());
		try {
			lua_Func();
		}
		catch (LuaException const& e) {
			// not editing errmsg here (nowhere to display), use log instead
			static char lastWarn[256] = "";
			if (strcmp(lastWarn, e.what())) {
				strcpy(lastWarn, e.what());
				LOG(lastWarn);
			}
		}
	}

	LuaRef Script::GetVar(std::string varName) {
		if (Application::GetMode() != GAME) return LuaRef(L);
		if (varName.empty() || filename.empty() || errmsg.size() > 0) return LuaRef(L);
		LuaRef lua_Var = getGlobal(L, varName.c_str());
		if (lua_Var.isNil()) {
			// not editing errmsg here (nowhere to display), use log instead
			char buffer[256];			
			static char prevBuffer[256];
			sprintf(buffer, "%s: %s does not exist", filename.c_str(), varName.c_str());
			if (strcmp(buffer, prevBuffer) != 0) {				
				LOG(buffer);
				strcpy(prevBuffer, buffer);
			}
		}
		return lua_Var;
	}

	void Script::OnBegin() {		
		if (Application::GetMode() != GAME) return;
		if (filename.size() == 0 || errmsg.size() > 0) return;
		LuaRef lua_OnBegin = getGlobal(L, "OnBegin");
		try {
			lua_OnBegin();
		}
		catch (LuaException const& e) {
			errmsg += "OnBegin(): " + std::string(e.what()) + "\n";
		}
	}

	void Script::OnUpdate(double deltaTime) {
		if (Application::GetMode() != GAME) return;
		if (filename.size() == 0 || errmsg.size() > 0) return;
		LuaRef lua_OnUpdate = getGlobal(L, "OnUpdate");
		try {
			lua_OnUpdate(deltaTime);
		}
		catch (LuaException const& e) {
			errmsg += "OnUpdate(): " + std::string(e.what()) + "\n";
		}
	}
	
	void FunctionBinder::DisableCursor() {
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void FunctionBinder::EnableCursor() {
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	glm::vec3 FunctionBinder::Normalize(glm::vec3 vec) {
		return glm::normalize(vec);
	}

	AudioSource* FunctionBinder::GetAudioSource(GameObject* gameObject) {
		return gameObject->GetComponent<AudioSource>();
	}
	CameraComponent* FunctionBinder::GetCameraComponent(GameObject* gameObject) {
		return gameObject->GetComponent<CameraComponent>();
	}
	LightComponent* FunctionBinder::GetLightComponent(GameObject* gameObject) {
		return gameObject->GetComponent<LightComponent>();
	}
	MeshComponent* FunctionBinder::GetMeshComponent(GameObject* gameObject) {
		return gameObject->GetComponent<MeshComponent>();
	}
	ParticleComponent* FunctionBinder::GetParticleComponent(GameObject* gameObject) {
		return gameObject->GetComponent<ParticleComponent>();
	}
	PhysicsComponent* FunctionBinder::GetPhysicsComponent(GameObject* gameObject) {
		return gameObject->GetComponent<PhysicsComponent>();
	}

	void Script::Bind() {
		// Class/function binding
		getGlobalNamespace(L)
			.beginNamespace("Lobster")
			// Input events
			.addFunction("IsKeyDown", Input::IsKeyDown)
			.addFunction("IsKeyUp", Input::IsKeyUp)
			.addFunction("GetMousePosX", Input::GetMousePosX)
			.addFunction("GetMousePosY", Input::GetMousePosY)
			.addFunction("GetMouseDeltaX", Input::GetMouseDeltaX)
			.addFunction("GetMouseDeltaY", Input::GetMouseDeltaY)
			.addFunction("IsMouseDown", Input::IsMouseDown)
			.addFunction("IsMouseHold", Input::IsMouseHold)
			.addFunction("LockCursor", Input::LockCursor)
			.addFunction("UnlockCursor", Input::UnlockCursor)
			.addFunction("DisableCursor", FunctionBinder::DisableCursor)
			.addFunction("EnableCursor", FunctionBinder::EnableCursor)
			// utilities
			.addFunction("normalize", FunctionBinder::Normalize)
			.addFunction("GetAudioSource", FunctionBinder::GetAudioSource)
			.addFunction("GetCameraComponent", FunctionBinder::GetCameraComponent)
			.addFunction("GetLightComponent", FunctionBinder::GetLightComponent)
			.addFunction("GetMeshComponent", FunctionBinder::GetMeshComponent)
			.addFunction("GetPhysicsComponent", FunctionBinder::GetPhysicsComponent)
			.addFunction("GetParticleComponent", FunctionBinder::GetParticleComponent)
			// glm::vec3			
			.beginClass<glm::vec3>("Vec3")
			.addConstructor<void(*) (float, float, float)>()
			.addProperty("x", &glm::vec3::x)
			.addProperty("y", &glm::vec3::y)
			.addProperty("z", &glm::vec3::z)
			.endClass()
			// Transform
			.beginClass<Transform>("Transform")
			.addProperty("WorldPosition", &Transform::WorldPosition)
			.addFunction("Up", &Transform::Up)
			.addFunction("Right", &Transform::Right)
			.addFunction("Forward", &Transform::Forward)
			.addFunction("RotateEuler", &Transform::RotateEuler)
			.addFunction("RotateAround", &Transform::RotateAround)
			.addFunction("LookAt", &Transform::LookAt)
			.endClass()
			// Component
			.beginClass<Component>("Component")
			.addFunction("IsEnabled", &Component::IsEnabled)
			.addFunction("GetOwner", &Component::GetOwner)
			.addFunction("GetType", &Component::GetType) // LuaBridge does not support enum
			.endClass()
			// Physics Component
			.deriveClass<PhysicsComponent, Component>("PhysicsComponent")
			.addFunction("ApplyForce", &PhysicsComponent::ApplyForce)
			.endClass()
			// Camera Component
			.deriveClass<CameraComponent, Component>("CameraComponent")
			.addFunction("GetPosition", &CameraComponent::GetPosition)
			.addFunction("GetUI", &CameraComponent::GetUI)
			.endClass()
			// UI
			.beginClass<GameUI>("GameUI")
			.addFunction("GetSpriteList", &GameUI::GetSpriteList)
			.endClass()
			// Sprite
			.beginClass<Sprite2D>("Sprite2D")
			.addProperty("x", &Sprite2D::x)
			.addProperty("y", &Sprite2D::y)
			.addProperty("alpha", &Sprite2D::alpha)
			.addFunction("IsMouseOver", &Sprite2D::IsMouseOver)
			.endClass()
			.deriveClass<ImageSprite2D, Sprite2D>("ImageSprite2D")
			.endClass()
			.deriveClass<TextSprite2D, Sprite2D>("TextSprite2D")
			.addFunction("SetText", &TextSprite2D::SetText)
			.addFunction("SetFontSize", &TextSprite2D::SetFontSize)
			.addFunction("SetColor", &TextSprite2D::SetColor)
			.endClass()
			.deriveClass<DynamicTextSprite2D, TextSprite2D>("DynamicTextSprite2D")
			.endClass()
			// Audio Component
			.beginClass<AudioSource>("AudioSource")
			.addFunction("Play", &AudioSource::Play)
			.addFunction("Stop", &AudioSource::Stop)
			.addFunction("Mute", &AudioSource::Mute)
			.addFunction("Unmute", &AudioSource::Unmute)
			.addFunction("SetGain", &AudioSource::SetGain)
			.addFunction("SetPitch", &AudioSource::SetPitch)
			.endClass()
			// GameObject
			.beginClass<GameObject>("GameObject")
			.addConstructor<void(*)(const char*)>()
			.addProperty("transform", &GameObject::transform)			
			.addFunction("AddComponent", &GameObject::AddComponent)
			.addFunction("AddChild", &GameObject::AddChild)
			.addFunction("Intersects", &GameObject::Intersects)
			.addFunction("Destroy", &GameObject::Destroy)
			.endClass()
			// Scene
			.beginClass<Scene>("Scene")
			.addFunction("AddGameObject", &Scene::AddGameObject)
			//.addFunction("RemoveGameObject", ) <- overloaded function
			.addFunction("GetGameObjects", &Scene::GetGameObjects)
			.endClass()
			.endNamespace();
		getGlobalNamespace(L).beginNamespace("Lobster").endNamespace();
		// Object passing
		push(L, transform); // pointer to 'Transform', C++ lifetime
		lua_setglobal(L, "transform");
		push(L, gameObject); // pointer to 'GameObject', C++ lifetime
		lua_setglobal(L, "this");
	}

}