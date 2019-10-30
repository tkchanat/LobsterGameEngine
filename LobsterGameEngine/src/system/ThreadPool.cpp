#include "pch.h"
#include "ThreadPool.h"

namespace Lobster
{

	ThreadPool* ThreadPool::s_instance = nullptr;


	ThreadPool::ThreadPool()
	{
	}

	ThreadPool::~ThreadPool()
	{
		// halt and destruct all threads
		{
			std::unique_lock<std::mutex> lock{ m_eventMutex };
			b_isStopped = true;
		}

		m_eventVariable.notify_all();

		for (auto &thread : m_threads)
			thread.join();
	}

	void ThreadPool::Initialize(int numThreads)
	{
		if (s_instance)
		{
			std::runtime_error("ThreadPool already initialized!");
		}
		s_instance = new ThreadPool;

		// create thread pools
		for (auto i = 0u; i < numThreads; ++i)
		{
			s_instance->m_threads.emplace_back([=] {
				while (true)
				{
					Job task;

					{
						std::unique_lock<std::mutex> lock{ s_instance->m_eventMutex };

						s_instance->m_eventVariable.wait(lock, [=] { return s_instance->b_isStopped || !s_instance->m_jobs.empty(); });

						if (s_instance->b_isStopped && s_instance->m_jobs.empty())
							break;

						task = std::move(s_instance->m_jobs.front());
						s_instance->m_jobs.pop();
					}

					task();
				}
			});
		}
	}

}
