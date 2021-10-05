#pragma once
#include"Epoll.h"

#include<thread>
#include<memory>
#include<vector>
#include<functional>
#include<mutex>
class Channel;
class EventLoop{
public:
	typedef std::function<void()> Functor;
	typedef std::shared_ptr<Channel> SP_Channel;
	typedef std::shared_ptr<Epoll> SP_Epoll;
	typedef std::vector<SP_Channel> ChannelList;
	EventLoop();
	~EventLoop();
	void loop();
	void runInLoop(const Functor& cb);
	void queueInLoop(const Functor& cb);
	void quit()
	{
		m_quit=true;
	}
	void assertInLoopThread()
	{
		if(!isInLoopThread())
			abortNotInLoopThread();
	}
	EventLoop* getEventLoopOfCurrentThread()
	{
		return t_loopInThisThread;
	}
	bool isInLoopThread() const
	{
		return m_threadId==std::this_thread::get_id();
	}
	void removeFromPoller(SP_Channel channel)
       	{
   		 m_poller->epollDel(channel);
  	}
  	void updatePoller(SP_Channel channel)
       	{
    		m_poller->epollMod(channel);
  	}
	 void addToPoller(SP_Channel channel,int timeout=0) {
   		 m_poller->epollAdd(channel,timeout);
  	}
private:
	 static int createEventFd();
	 void abortNotInLoopThread();
	 void handleRead();
	 void doPendingFunctors();
	 void wakeup();
private:
	EventLoop* t_loopInThisThread;
	int m_wakeupFd;
	SP_Channel m_wakeupChannel;
	std::vector<Functor> m_pendingFunctors;
	std::mutex m_mutex;
	ChannelList m_activeChannels;
	SP_Epoll m_poller;
	std::thread::id m_threadId;
	bool m_callingpendingFunctors;
	bool m_looping;
	bool m_quit;
};
