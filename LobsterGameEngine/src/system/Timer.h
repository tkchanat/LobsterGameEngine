#pragma once

#include <chrono>

namespace Lobster
{

	typedef std::chrono::high_resolution_clock HighResolutionClock;
	typedef std::chrono::duration<double, std::milli> milliseconds_type;
    
    //  This class is for integrating precision timing to your code.
	//	The Timer object is recommended to statically allocate in the stack, 
	//	since we want the timer destroy itself when it goes out of scope. (i.e. new operator is not recommended.)
    //  This timer is implemented with std::chrono right now. For a better performance, please implement it platform dependently.
	class Timer
	{
	private:
		std::chrono::time_point<HighResolutionClock> m_startTime;
		std::chrono::time_point<HighResolutionClock> m_endTime;
		std::chrono::time_point<HighResolutionClock> m_lastRequestTime;
		bool m_running;
	public:
		Timer()
		{
			m_startTime = HighResolutionClock::now();
			m_lastRequestTime = m_startTime;
			m_running = true;
		}

		~Timer()
		{
		}

		void Stop()
		{
			m_endTime = HighResolutionClock::now();
			m_running = false;
		}

		void Restart()
		{
			m_startTime = HighResolutionClock::now();
			m_running = true;
		}

		double GetElapsedTime()
		{
			std::chrono::time_point<HighResolutionClock> endTime;

			if (m_running)
			{
				endTime = HighResolutionClock::now();
			}
			else
			{
				endTime = m_endTime;
			}

			return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count();
		}

		double GetDeltaTime()
		{
			std::chrono::time_point<HighResolutionClock> now = HighResolutionClock::now();
			double deltaTime = std::chrono::duration_cast<milliseconds_type>(now - m_lastRequestTime).count();
			m_lastRequestTime = now;
			return deltaTime;
		}
	};
    
}
