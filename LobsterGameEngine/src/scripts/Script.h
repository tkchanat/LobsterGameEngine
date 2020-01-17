#pragma once

#include "components/Component.h"
#include "objects/GameObject.h"
#include "objects/Transform.h"

namespace Lobster
{

	class EventHandler {

	};
    
	//	This class is a component for user to define custom scripts with Lua.
    class Script : public Component {
	private:
		lua_State* L;
		std::string filename;
		std::string errmsg;	// error message of the script (if any)
		bool m_open = true;

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
		virtual void OnSimulationBegin() override;
		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override {}
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override {}
    };
    
}
