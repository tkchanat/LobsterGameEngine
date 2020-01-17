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

	void Script::OnSimulationBegin() {
		// TODO reload script?
		// This is required for resetting all global variables.
		// If the program does not reset the values, simulation will start from
		// where it finished last time
	}

	void Script::OnImGuiRender() {
		// combo box of scripts
		if (ImGui::CollapsingHeader("Script", &m_open, ImGuiTreeNodeFlags_DefaultOpen)) {
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

		// Remove the component upon the cross button click
		if (!m_open) {
			gameObject->RemoveComponent(this);
		}
	}

	void Script::OnBegin() {		
		if (Application::GetMode() == EDITOR) return;
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
		if (Application::GetMode() == EDITOR) return;
		if (filename.size() == 0 || errmsg.size() > 0) return;
		LuaRef lua_OnUpdate = getGlobal(L, "OnUpdate");
		try {
			lua_OnUpdate(deltaTime);
		}
		catch (LuaException const& e) {
			errmsg += "OnUpdate(): " + std::string(e.what()) + "\n";
		}
	}

	void Script::Bind() {
		// Class/function binding
		getGlobalNamespace(L)
			.beginNamespace("Lobster")
			// Input events
			.addFunction("IsKeyDown", Input::IsKeyDown)
			.addFunction("GetMousePosX", Input::GetMousePosX)
			.addFunction("GetMousePosY", Input::GetMousePosY)
			.addFunction("IsMouseDown", Input::IsMouseDown)
			.addFunction("IsMouseHold", Input::IsMouseHold)
			// glm::vec3
			.beginClass<glm::vec3>("Vec3")
			.addProperty("x", &glm::vec3::x)
			.addProperty("y", &glm::vec3::y)
			.addProperty("z", &glm::vec3::z)
			.endClass()
			// Transform
			.beginClass<Transform>("Transform")
			.addProperty("WorldPosition", &Transform::WorldPosition)
			.addProperty("LocalPosition", &Transform::LocalPosition)
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
			// GameObject
			.beginClass<GameObject>("GameObject")
			.addConstructor<void (*)(const char*)>()
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