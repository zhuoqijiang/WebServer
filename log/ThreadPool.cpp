#include"ThreadPool.h"
#include"Log.h"
#include<iostream>
uint32_t ThreadPool::size()
{
	std::lock_guard<std::mutex> gLock(m_mutex);
	uint32_t appenderSize = m_appenders.size();
	return appenderSize;
}
ThreadPool::ptr ThreadPool::threadfun(ThreadPool::ptr arg)
{
	ThreadPool::ptr pool = static_cast<ThreadPool::ptr>(arg);
	SP_Appender appender=nullptr;
	while (pool->m_isRunning) {
		std::queue<SP_Appender> appenders= pool->take();
		while(!appenders.empty()) {
			appender=appenders.front();
			appenders.pop();
			appender->append();
		}
	}
	return pool;
}
ThreadPool::ThreadPool():m_threadnum(THREADNUM),m_isRunning(false)
{
	
}
void ThreadPool::start()
{
	m_isRunning=true;
	for (int i = 0; i < m_threadnum; i++) {	
		m_threads.push_back(std::thread(threadfun,shared_from_this()));
		
	}
}
void ThreadPool::add(SP_Appender appender)
{
	{
	std::lock_guard<std::mutex> gLock(m_mutex);
	m_appenders.push(appender);
	}
	if(size()>=100){
		std::lock_guard<std::mutex> gQueueLock(m_queueMutex);
		std::lock_guard<std::mutex> gLock(m_mutex);
		m_appendersQueue.emplace(move(m_appenders));
		m_con.notify_one();
	}
}
void ThreadPool::handleOldLog()
{
	if(size()<=0)
		return;
	std::lock_guard<std::mutex> gQueueLock(m_queueMutex);
        std::lock_guard<std::mutex> gLock(m_mutex);
        m_appendersQueue.emplace(move(m_appenders));
        m_con.notify_one();
}
std::queue<ThreadPool::SP_Appender> ThreadPool::take()
{
	std::queue<SP_Appender> appenders;
	while (appenders.empty()&&m_isRunning) {
		std::unique_lock<std::mutex> uQueueLock(m_queueMutex);
		while (m_appendersQueue.empty() && (m_isRunning)) {
			m_con.wait(uQueueLock);
		}
		if (!m_isRunning)
			break;
		else if (m_appendersQueue.empty())
			continue;
		appenders = m_appendersQueue.front();
		m_appendersQueue.pop();
	}
	return appenders;
}
void ThreadPool::stop()
{
	if (!m_isRunning)
		return;
	m_isRunning = false;
	m_con.notify_all();
	for (auto &thread:m_threads) {
		thread.join();
	}
	m_threads.clear();
}
