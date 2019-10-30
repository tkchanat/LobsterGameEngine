#pragma once
#include <functional>

namespace Lobster
{

	class Event;

	class IEventCallback
	{
	public:
		virtual void operator() (Event* event) = 0;
	};
	typedef std::vector<IEventCallback*> CallbackList;

	// Bind any functors to a callback function
	// Usage:
	//  bool Class::foo(KeyPressedEvent* e) { return false; }  /* 1. Member function */
	//
	//  EventCallback* callback = new EventCallback(&foo);     /* Create callback from function */
	//  EventDispatcher::GetInstance()->AddCallback(callback); /* Register callback to dispatcher */
	template<typename T>
	class EventCallback : public IEventCallback
	{
		typedef std::function<void(T*)> Functor;
	private:
		Functor functor;
	public:
		EventCallback(Functor f) : functor(f) {}
		virtual void operator() (Event* event) override { functor((T*)event); }
	};

} // namespace SSF
