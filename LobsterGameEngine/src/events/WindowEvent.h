#pragma once
#include "Event.h"

namespace Lobster
{
    
    class WindowClosedEvent : public Event
    {
    public:
        WindowClosedEvent() : Event(EVENT_WINDOW_CLOSED) {}
        ~WindowClosedEvent() = default;
    };
    
    class WindowResizedEvent : public Event
    {
    public:
        uint Width, Height;
    public:
        WindowResizedEvent(uint width, uint height) : Event(EVENT_WINDOW_RESIZED), Width(width), Height(height) {}
        ~WindowResizedEvent() = default;
    };
    
    class WindowMovedEvent : public Event
    {
    public:
        uint X, Y;
    public:
        WindowMovedEvent(uint x, uint y) : Event(EVENT_WINDOW_MOVED), X(x), Y(y) {}
        ~WindowMovedEvent() = default;
    };

	class WindowMinimizedEvent : public Event
	{
	public:
		bool Minimized;
	public:
		WindowMinimizedEvent(bool minimized) : Event(EVENT_WINDOW_MINIMIZED), Minimized(false) {}
	};

} // namespace SSF
