#pragma once
#include<thread>
#include<condition_variable>
#include<mutex>
class EventLoop;
class EventLoopThread{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startloop();
private:
	void threadFunc();
	bool m_exiting;
	EventLoop* m_loop;
	std::thread m_thread;
	std::mutex m_mutex;
	std::condition_variable m_con;
};
