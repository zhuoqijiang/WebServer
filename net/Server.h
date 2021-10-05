#pragma once
#include"EventLoop.h"
#include"Acceptor.h"
#include <memory>
#include<string>
class Channel;
class EventLoopThreadPool;
class Server {
public:
	 typedef std::shared_ptr<Channel> SP_Channel;
 	 Server(EventLoop *loop, int threadNum, int port);
	 void start();
 	 EventLoop *getloop() const 
	 {
		 return m_loop;
	 }
	 std::shared_ptr<EventLoopThreadPool> getEventLoopThreadPool()
	 {
		return m_eventLoopThreadPool;
	 }
	 ~Server();

private:
	int m_threadNum;
  	EventLoop *m_loop;
  	std::shared_ptr<EventLoopThreadPool> m_eventLoopThreadPool;
  	Acceptor m_acceptor;
	bool m_started;
};
