#include"EventLoopThread.h"
#include"EventLoop.h"
#include<mutex>
#include<condition_variable>
EventLoopThread::EventLoopThread()
:m_loop(nullptr),
m_exiting(false),
m_mutex(),
m_con() 
{}
EventLoopThread::~EventLoopThread()
{
  	m_exiting = true;
  	if (m_loop != nullptr) {
    		m_loop->quit();
    		m_thread.join();
  	}
}

EventLoop* EventLoopThread::startloop() 
{
  	m_thread=std::thread([this](){this->threadFunc();});
  	{

		std::unique_lock<std::mutex> uLock(m_mutex);
    		while (m_loop == nullptr){
		 	 m_con.wait(uLock);
		}

  	}
  	return m_loop;
}

void EventLoopThread::threadFunc()
{
  	EventLoop loop;
	{
		std::unique_lock<std::mutex> uLock(m_mutex);
    		m_loop = &loop;
    		m_con.notify_one();
 	}
  	loop.loop();
	m_loop = nullptr;
}
