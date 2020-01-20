#pragma once

#include "components/Component.h"
#include "objects/GameObject.h"
#include "objects/Transform.h"

namespace Lobster
{

	// A helper class to bind functions to Lua through LuaBridge
	struct FunctionBinder {
		static void DisableCursor();
		static void EnableCursor();
		// glm vector/matrix utilities
		static glm::vec3 Normalize(glm::vec3 vec);
		// component getters
		static AudioSource* GetAudioSource(GameObject* gameObject);
		static CameraComponent* GetCameraComponent(GameObject* gameObject);		
		static LightComponent* GetLightComponent(GameObject* gameObject);
		static MeshComponent* GetMeshComponent(GameObject* gameObject);
		static ParticleComponent* GetParticleComponent(GameObject* gameObject);
		static PhysicsComponent* GetPhysicsComponent(GameObject* gameObject);				
	};
    
	//	This class is a component for user to define custom scripts with Lua.
    class Script : public Component {
	private:
		lua_State* L;
		std::string filename;
		std::string errmsg;	// error message of the script (if any)

		// Load or reload a Lua script in relative path into the object.
		void loadScript(const char* file);
    public:
		Script();
		~Script();	
		// pass class/object information to Lua via LuaBridge
		void Bind(); 
		virtual void OnBegin() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override {}
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override {}
    };
    
}
