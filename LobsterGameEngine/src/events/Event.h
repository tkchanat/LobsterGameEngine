#pragma once

namespace Lobster
{
    
    enum EventType : uint
    {
        EVENT_NONE = 0,
		EVENT_KEY_PRESSED, EVENT_KEY_RELEASED,
		EVENT_MOUSE_PRESSED, EVENT_MOUSE_RELEASED, EVENT_MOUSE_MOVED, EVENT_MOUSE_SCROLLED,
		EVENT_WINDOW_CLOSED, EVENT_WINDOW_FOCUSED, EVENT_WINDOW_RESIZED, EVENT_WINDOW_FOCUS, EVENT_WINDOW_MOVED, EVENT_WINDOW_MINIMIZED,
		EVENT_VIEWPORT_RESIZED, EVENT_IMGUI_ITEM_SELECTED
    };    
    
	//	This struct defines what an event looks like.
	//	Every events has an event type and a boolean value for determining whether it has been processed.
	class Event
    {
    protected:
        EventType m_type;
    public:
		virtual ~Event() {}
        inline EventType GetType() const { return m_type; }
	protected:
		Event(EventType type) : m_type(type) {}
    };

}
