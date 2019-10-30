#include "pch.h"
#include "Profiler.h"

namespace Lobster
{

	Profiler* Profiler::s_instance = nullptr;

	Profiler::Profiler() :
		m_interval(50.0)
	{
	}

	void Profiler::Initialize()
	{
		if (s_instance)
		{
			throw std::runtime_error("Profiler already initialized, please don't create another!");
		}
		s_instance = new Profiler;
	}

	void Profiler::SubmitData(const std::string & name, double data)
	{
		s_instance->m_cumulativeTime += s_instance->m_timer.GetDeltaTime();
		if (s_instance->m_cumulativeTime > s_instance->m_interval)
		{
			s_instance->m_profilerData[name] = data;
			s_instance->m_cumulativeTime = 0.0;
		}
	}

}
