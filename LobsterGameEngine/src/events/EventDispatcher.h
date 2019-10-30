#pragma once
#include "EventCallback.h"
#include "EventCollection.h"
#include <unordered_map>

namespace Lobster
{

	class EventDispatcher
	{
	private:
		static EventDispatcher* s_instance;
		std::unordered_map<EventType, CallbackList> m_callbackMap;
	public:
		static void Initialize() { if (!s_instance) s_instance = new EventDispatcher; }
		static void Dispatch(Event* event);
		static void AddCallback(EventType type, IEventCallback* callback);
		static void RemoveCallback(EventType type, IEventCallback* callback);
	};

} // namespace SSF
