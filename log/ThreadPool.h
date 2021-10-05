#pragma once
#include<mutex>
#include<thread>
#include<condition_variable>
#include<queue>
#include<vector>
#include<memory>
#include<deque>
#define THREADNUM 1
class Appender;
class ThreadPool:public std::enable_shared_from_this<ThreadPool> {
public:
	typedef std::shared_ptr<ThreadPool> ptr;
	typedef std::shared_ptr<Appender> SP_Appender;
	void start();
	void add(SP_Appender appender);
	void handleOldLog();
	uint32_t size();
	void stop();
	ThreadPool();
private:
	std::queue<SP_Appender> take();
	static ThreadPool::ptr threadfun(ThreadPool::ptr arg);
private:
	std::mutex m_mutex;
	std::mutex m_queueMutex;
	uint32_t m_threadnum;
	std::condition_variable m_con;
	std::vector<std::thread> m_threads;
	std::queue<SP_Appender> m_appenders;
	std::queue<std::queue<SP_Appender>> m_appendersQueue;
	bool m_isRunning;
};
