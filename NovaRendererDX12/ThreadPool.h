#pragma once

#include <functional>
#include <thread>
#include <deque>
#include <condition_variable>

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();

	void JobStealerLoop();
	void AddJob(std::function<void()> newJob);
private:
	struct Task {
		std::function<void()> m_Job;
		std::vector<Task*> m_ChildTasks;
	};

	bool m_Exiting;
	int m_NumThreads;
	int m_ActiveThreads;
	
	std::vector<std::thread> m_Pool;
	std::deque<Task> m_Queue;
	std::condition_variable m_Condition;
	std::mutex m_QueueMutex;
};

ThreadPool* GetThreadPool();