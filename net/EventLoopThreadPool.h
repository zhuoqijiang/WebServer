#pragma once
#include<memory>
#include<vector>
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool{
public:
	typedef std::shared_ptr<EventLoopThread> SP_EventLoopThread;
	EventLoopThreadPool(EventLoop* baseloop,int numThreads);
	void start();
	EventLoop* getnextloop();
private:
	EventLoop* m_baseloop;
	int m_threadNum;
	int m_next;
	std::vector<SP_EventLoopThread> m_threads;
	std::vector<EventLoop*> m_loops;
	bool m_started;
};
