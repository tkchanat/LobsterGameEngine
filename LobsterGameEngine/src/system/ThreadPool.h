#pragma once

namespace Lobster
{

	using Job = std::function<void()>;

	class ThreadPool
	{
	private:
		std::vector<std::thread> m_threads;
		std::condition_variable m_eventVariable;
		std::mutex m_eventMutex;
		bool b_isStopped = false;
		std::queue<Job> m_jobs;
		static ThreadPool* s_instance;
	public:
		ThreadPool();
		~ThreadPool();
		static void Initialize(int numThreads);
		template<class T>
		static auto Enqueue(T task)->std::future<decltype(task())>
		{
			auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));

			{
				std::unique_lock<std::mutex> lock{ s_instance->m_eventMutex };
				s_instance->m_jobs.emplace([=] {
					(*wrapper)();
				});
			}

			s_instance->m_eventVariable.notify_one();
			return wrapper->get_future();
		}
	};

}
