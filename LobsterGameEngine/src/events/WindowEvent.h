#pragma once
#include "Event.h"

namespace Lobster
{
    
    class WindowClosedEvent : public Event
    {
    public:
        WindowClosedEvent() : Event(EVENT_WINDOW_CLOSED) {}
    };

	class WindowFocusedEvent : public Event
	{
	public:
		bool Focused;
	public:
		WindowFocusedEvent(bool focused) : Event(EVENT_WINDOW_FOCUSED), Focused(focused) {}
	};
    
    class WindowResizedEvent : public Event
    {
    public:
        uint Width, Height;
    public:
        WindowResizedEvent(uint width, uint height) : Event(EVENT_WINDOW_RESIZED), Width(width), Height(height) {}
    };
    
    class WindowMovedEvent : public Event
    {
    public:
        uint X, Y;
    public:
        WindowMovedEvent(uint x, uint y) : Event(EVENT_WINDOW_MOVED), X(x), Y(y) {}
    };

	class WindowMinimizedEvent : public Event
	{
	public:
		bool Minimized;
	public:
		WindowMinimizedEvent(bool minimized) : Event(EVENT_WINDOW_MINIMIZED), Minimized(false) {}
	};

} // namespace SSF
