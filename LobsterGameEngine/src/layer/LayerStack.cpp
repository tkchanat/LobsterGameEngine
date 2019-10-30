#include "pch.h"
#include "LayerStack.h"
#include "Layer.h"

namespace Lobster
{

	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
	}

	void LayerStack::Push(Layer * layer)
	{
		m_layerInsert = m_layers.emplace(m_layerInsert, layer);
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer * layer)
	{
		m_layers.emplace_back(layer);
	}

	void LayerStack::Pop(Layer* layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
			m_layerInsert--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
		}
	}

	void LayerStack::OnUpdate(double deltaTime)
	{
		for (Layer* layer : m_layers)
		{
			layer->OnUpdate(deltaTime);
		}
	}

}
