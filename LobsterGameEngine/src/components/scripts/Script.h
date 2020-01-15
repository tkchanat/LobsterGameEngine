#pragma once

#include "components/Component.h"
#include "objects/GameObject.h"
#include "objects/Transform.h"

namespace Lobster
{
    
	//	This class is an abstract class isolated all user custom scripts from Component class.
	//	All user scripts must inherit from this class.
	//	Although user scripts are kind of a component, I tend to separate them just for the sake of readability.
    class Script : public Component
    {
    public:
		Script() : Component(SCRIPT_COMPONENT) {}
        ~Script() = default;
        virtual void OnUpdate(double deltaTime);
		virtual void OnImGuiRender();
    };
    
}
