#pragma once
#include "Event.h"

namespace Lobster
{

	class ViewportResizedEvent : public Event
	{
	public:
        uint TopLeftX, TopLeftY;
		uint Width, Height;
	public:
		ViewportResizedEvent(uint topLeftX, uint topLeftY, uint width, uint height) :
			Event(EVENT_VIEWPORT_RESIZED),
            TopLeftX(topLeftX),
            TopLeftY(topLeftY),
			Width(width),
			Height(height)
		{
		}
		~ViewportResizedEvent() {}
	};
    
    class ImGuiItemSelectedEvent : public Event
    {
    public:
        std::string ItemName;
    public:
        ImGuiItemSelectedEvent(std::string itemName) :
            Event(EVENT_IMGUI_ITEM_SELECTED),
            ItemName(itemName)
        {
        }
        ~ImGuiItemSelectedEvent() {}
    };


} // namespace SSF
