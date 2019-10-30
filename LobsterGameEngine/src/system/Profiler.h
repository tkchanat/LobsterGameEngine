#pragma once
#include "Timer.h"

namespace Lobster
{

	class Profiler
	{
		friend class ImGuiScene;
	private:
		double m_interval;
		double m_cumulativeTime;
		Timer m_timer;
		std::map<std::string, double> m_profilerData;
		static Profiler* s_instance;
	public:
		Profiler();
		static void Initialize();
		static void SubmitData(const std::string& name, double data);
	};

}
