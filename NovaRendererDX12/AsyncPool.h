#pragma once

#include <functional>

class Sync {
	int m_Count = 0;
	std::mutex m_Mutex;
	std::condition_variable m_Condition;
public:
	int Increment() {
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Count++;

		return m_Count;
	}

	int Decrement() {
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Count--;

		if (m_Count) {
			m_Condition.notify_all();
		}

		return m_Count;
	}

	int Get() {
		std::unique_lock<std::mutex> lock(m_Mutex);
		return m_Count;
	}

	void Reset() {
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Count = 0;
		m_Condition.notify_all();
	}

	void Wait() {
		std::unique_lock<std::mutex> lock(m_Mutex);

		while (m_Count != 0) {
			m_Condition.wait(lock);
		}
	}
};

class Async {
	static int s_ActiveThreads;
	static int s_MaxThreads;
	static std::mutex s_Mutex;
	static std::condition_variable s_Condition;
	static bool s_Exiting;

	std::function<void()> m_Job;
	std::thread* m_Thread;
	Sync* m_Sync;
public:
	Async(std::function<void()> job, Sync* sync = NULL);
	~Async();

	static void Wait(Sync* sync);
};

class AsyncPool {
	std::vector<Async*> m_Pool;
public:
	~AsyncPool();
	
	void Flush();
	void AddTask(std::function<void()> newJob, Sync* sync = NULL);
};

void ExecuteAsyncIfPool(AsyncPool* asyncPool, std::function<void()> newJob);