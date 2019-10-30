#pragma once
#include <vector>

namespace Lobster
{

	class Layer;

	class LayerStack
	{
	private:
		std::vector<Layer*> m_layers;
		std::vector<Layer*>::iterator m_layerInsert;
	public:
		LayerStack();
		~LayerStack();
		void Push(Layer* layer);
		void PushOverlay(Layer* layer);
		void Pop(Layer* layer);
		void PopOverlay(Layer* layer);
		void OnUpdate(double deltaTime);
		
		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_layers.end(); }
	};

}
