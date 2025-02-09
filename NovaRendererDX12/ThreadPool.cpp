#include "stdafx.h"
#include "ThreadPool.h"

#define ENABLE_MULTI_THREADING

static ThreadPool g_ThreadPool;

ThreadPool* GetThreadPool() {
	return &g_ThreadPool;
}

ThreadPool::ThreadPool() {
	m_ActiveThreads = 0;
	m_NumThreads = 0;

#ifdef ENABLE_MULTI_THREADING
	m_NumThreads = std::thread::hardware_concurrency();
	m_Exiting = false;

	for (int dummy = 0; dummy < m_NumThreads; dummy++) {
		m_Pool.push_back(std::thread(&ThreadPool::JobStealerLoop, GetThreadPool()));
	}
#endif
}

ThreadPool::~ThreadPool() {
#ifdef ENABLE_MULTI_THREADING
	m_Exiting = true;
	m_Condition.notify_all();

	for (int idx = 0; idx < m_NumThreads; idx++) {
		m_Pool[idx].join();
	}
#endif
}

void ThreadPool::JobStealerLoop() {
#ifdef ENABLE_MULTI_THREADING
	while (true) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);

			m_Condition.wait(
				lock,
				[this] { return m_Exiting ||
				(!m_Queue.empty() && (m_ActiveThreads < m_NumThreads)); }
			);

			if (m_Exiting) {
				return;
			}

			m_ActiveThreads++;

			task = m_Queue.front();
			m_Queue.pop_front();
		}

		task.m_Job();

		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_ActiveThreads--;
		}
	}
#endif
}

void ThreadPool::AddJob(std::function<void()> newJob) {
#ifdef ENABLE_MULTI_THREADING
	if (!m_Exiting) {
		std::unique_lock<std::mutex> lock(m_QueueMutex);

		Task newTask;
		newTask.m_Job = newJob;
		m_Queue.push_back(newTask);

		if (m_ActiveThreads < m_NumThreads) {
			m_Condition.notify_one();
		}
	}
#else
	// do something else here
#endif
}