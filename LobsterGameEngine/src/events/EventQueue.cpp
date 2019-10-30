#include "pch.h"
#include "EventQueue.h"

namespace Lobster
{
    
    EventQueue* EventQueue::s_instance = nullptr;
    
	EventQueue::EventQueue()
    {
    }
    
    EventQueue::~EventQueue()
    {
    }

	void EventQueue::Initialize()
	{
		if (!s_instance)
		{
			s_instance = new EventQueue();
		}
	}
    
	Event* EventQueue::Next()
	{
		if (!m_queue.empty())
		{
			return m_queue.front();
		}
		return nullptr;
	}

	void EventQueue::Pop()
	{
		Event* front = m_queue.front();
		if(front) delete m_queue.front();
		front = nullptr;
		m_queue.pop();
	}

    void EventQueue::Clear()
    {
		while (!m_queue.empty())
		{
			m_queue.pop();
		}
    }
    
}
