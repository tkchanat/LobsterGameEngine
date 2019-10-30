#pragma once
#include "Event.h"

namespace Lobster 
{
    
    //  EventQueue is a global singleton class for polling and issuing event.
	//	Any unhandled events should stay in the queue until it is handled (To be discussed).
    class EventQueue
    {
    private:
        //  TODO:
        //  Don't use dynamic queue, use a fixed size stack array instead.
        //  This can restrict the number of event in a fraction of a second, to prevent spam.
		std::queue<Event*> m_queue;
		static EventQueue* s_instance;
    public:
        EventQueue();
        ~EventQueue();
		Event* Next();
		void Pop();
        void Clear();
		static void Initialize();
		inline static EventQueue* GetInstance() { return s_instance; }
        template<typename T, typename ...Args>
        inline void AddEvent(Args && ...args)
        {
            T* newEvent = new T(args...);
			m_queue.push(newEvent);
        }
		inline uint GetCount() const { return (uint)m_queue.size(); }
    };
    
}
