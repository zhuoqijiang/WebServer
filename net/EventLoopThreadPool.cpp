#include"EventLoop.h"
#include"EventLoopThread.h"
#include"EventLoopThreadPool.h"
#include"../log/Log.h"
#include<memory>
EventLoopThreadPool::EventLoopThreadPool (EventLoop* baseloop, int threadNum)
:m_baseloop(baseloop), 
m_started(false),
m_threadNum(threadNum),
m_next(0) 
{}
void EventLoopThreadPool::start() 
{
  	m_baseloop->assertInLoopThread();
  	m_started = true;
  	for (int i = 0; i < m_threadNum; ++i) {
    		std::shared_ptr<EventLoopThread> it(new EventLoopThread());
    		m_threads.push_back(it);
    		m_loops.push_back(it->startloop());
 	}
}
EventLoop* EventLoopThreadPool::getnextloop()
{
  	m_baseloop->assertInLoopThread();
  	EventLoop  *loop = m_baseloop;
  	if (!m_loops.empty()) {
    		loop = m_loops[m_next];
    		m_next = (m_next + 1) % m_threadNum;
  	}
  	return loop;
}

