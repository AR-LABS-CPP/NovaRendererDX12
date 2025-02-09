#include "stdafx.h"
#include "ThreadPool.h"
#include "AsyncPool.h"

Async::Async(std::function<void()> job, Sync* sync) :
	m_Job{ job }, m_Sync{ sync } {
	if (m_Sync) {
		m_Sync->Increment();
	}

	{
		std::unique_lock<std::mutex> lock(s_Mutex);

		while (s_ActiveThreads >= s_MaxThreads) {
			s_Condition.wait(lock);
		}

		s_ActiveThreads++;
	}

	m_Thread = new std::thread([this]() {
		m_Job();

		{
			std::lock_guard<std::mutex> lock(s_Mutex);
			s_ActiveThreads--;
		}

		s_Condition.notify_one();

		if (m_Sync) {
			m_Sync->Decrement();
		}
	});
}

Async::~Async() {
	m_Thread->join();
	delete m_Thread;
}

void Async::Wait(Sync* sync) {
	if (sync->Get() == 0) {
		return;
	}

	{
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_ActiveThreads--;
	}

	s_Condition.notify_one();
	sync->Wait();

	{
		std::unique_lock<std::mutex> lock(s_Mutex);
		s_Condition.wait(lock, [] {
			return s_Exiting || (s_ActiveThreads < s_MaxThreads);
		});

		s_ActiveThreads++;
	}
}

AsyncPool::~AsyncPool() {
	Flush();
}

void AsyncPool::Flush() {
	for(int idx = 0; idx < m_Pool.size(); idx++) {
		delete m_Pool[idx];
	}

	m_Pool.clear();
}

void AsyncPool::AddTask(std::function<void()> newJob, Sync* sync) {
	m_Pool.push_back(new Async(newJob, sync));
}

void ExecuteAsyncIfPool(AsyncPool* asyncPool, std::function<void()> newJob) {
	if (asyncPool != NULL) {
		asyncPool->AddTask(newJob);
	}
}

int Async::s_ActiveThreads = 0;
int Async::s_MaxThreads = std::thread::hardware_concurrency();
bool Async::s_Exiting = false;

std::mutex Async::s_Mutex;
std::condition_variable Async::s_Condition;