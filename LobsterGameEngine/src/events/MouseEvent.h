#pragma once
#include "Event.h"

namespace Lobster
{
    
    class MousePressedEvent : public Event
    {
    public:
        int Button;
        bool Hold;
    public:
        MousePressedEvent(int button, bool hold) : Event(EVENT_MOUSE_PRESSED), Button(button), Hold(hold) {}
        ~MousePressedEvent() = default;
    };
    
    class MouseReleasedEvent : public Event
    {
    public:
        int Button;
    public:
        MouseReleasedEvent(int button) : Event(EVENT_MOUSE_RELEASED), Button(button) {}
        ~MouseReleasedEvent() = default;
    };
    
    struct MouseMovedEvent : public Event
    {
    public:
        float x, y;
    public:
        MouseMovedEvent(float x, float y) : Event(EVENT_MOUSE_MOVED), x(x), y(y) {}
        ~MouseMovedEvent() = default;
    };
    
    struct MouseScrolledEvent : public Event
    {
    public:
        float dx, dy;
    public:
        MouseScrolledEvent(float dx, float dy) : Event(EVENT_MOUSE_SCROLLED), dx(dx), dy(dy) {}
        ~MouseScrolledEvent() = default;
    };
    
} // namespace SSF
