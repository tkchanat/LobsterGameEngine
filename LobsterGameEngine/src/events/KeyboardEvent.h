#pragma once
#include "Event.h"

namespace Lobster {
    
	//	This struct defines all kinds of keyboard input events, 
	//	associated with the event key and action (pressed? released? hold?), etc.
    class KeyPressedEvent : public Event
    {
    public:
        int Key;
        bool Hold;
    public:
        KeyPressedEvent(int key, bool hold) : Event(EVENT_KEY_PRESSED), Key(key), Hold(hold) {}
        ~KeyPressedEvent() = default;
    };

    class KeyReleasedEvent : public Event
    {
    public:
        int Key;
    public:
        KeyReleasedEvent(int key) : Event(EVENT_KEY_RELEASED), Key(key) {}
        ~KeyReleasedEvent() = default;
    };
    
} // namespace SSF
