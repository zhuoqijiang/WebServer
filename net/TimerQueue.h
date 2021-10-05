#pragma once
#include"Timer.h"
class Channel;
class HttpData;
class EventLoop;
struct TimerCmp {
	typedef std::shared_ptr<TimerNode> SP_TimerNode;
  	bool operator()(SP_TimerNode &a,SP_TimerNode &b) const
       	{
    		return a->getExpTime() > b->getExpTime();
  	}
};
class TimerQueue {
public:
	typedef std::shared_ptr<HttpData> SP_HttpData;
	typedef std::shared_ptr<TimerNode> SP_TimerNode;
	typedef std::shared_ptr<Channel> SP_Channel;
	TimerQueue(EventLoop* loop);
	TimerQueue()=delete;
  	void addTimer(SP_HttpData HttpData, int timeout);
	void start();
private:
	void handleLogQueue();
	void handleExpiredEvent();
	static int createTimerFd();
	EventLoop* m_loop;
	uint64_t  m_one;
	int m_timerFd;
	SP_Channel m_channel;
private:
  	std::priority_queue<SP_TimerNode, std::deque<SP_TimerNode>, TimerCmp> m_timerNodeQueue;
};
