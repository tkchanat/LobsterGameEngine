#include "pch.h"
#include "EventDispatcher.h"
#include "EventQueue.h"

namespace Lobster
{

	EventDispatcher* EventDispatcher::s_instance = nullptr;

	void EventDispatcher::AddCallback(EventType type, IEventCallback* callback)
	{
		CallbackList& callbacks = s_instance->m_callbackMap[type];
		CallbackList::iterator position = find(callbacks.begin(), callbacks.end(), callback);

		if (position != callbacks.end())
		{
			WARN("Action existed in delegate list.");
			return;
		}

		callbacks.push_back(callback);
	}

	void EventDispatcher::RemoveCallback(EventType type, IEventCallback* callback)
	{
		CallbackList& callbacks = s_instance->m_callbackMap[type];
		CallbackList::iterator position = find(callbacks.begin(), callbacks.end(), callback);

		if (position == callbacks.end())
		{
			return;
		}

		callbacks.erase(position);
	}

	void EventDispatcher::Dispatch(Event* event)
	{
		EventType type = event->GetType();
		bool noListeners = s_instance->m_callbackMap.find(type) == s_instance->m_callbackMap.end();
		if (noListeners)
		{
			EventQueue::GetInstance()->Pop();
			return; //  No one is listening to you, stupid event! Go away!
		}

		CallbackList& callbacks = s_instance->m_callbackMap[type];
		for (IEventCallback* callback : callbacks)
		{
			(*callback)(event);
		}
		EventQueue::GetInstance()->Pop();	//  No one is listening to you, stupid event! Go away!
	}



} // namespace SSF
