#pragma once
#include <string>

namespace Lobster
{
	
	class Layer
	{
	private:
		std::string m_name;
	public:
		Layer(const char* name) : m_name(name) {}
		virtual ~Layer() {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(double deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
	};

}
