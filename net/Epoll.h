#pragma once
#include"TimerQueue.h"
#include<sys/epoll.h>
#include<map>
#include<vector>
#include<memory>

class EventLoop;
class HttpData;
class Channel;

class Epoll{
public:
	typedef std::shared_ptr<Channel> SP_Channel; 
	typedef std::shared_ptr<HttpData> SP_HttpData;
	typedef std::vector<epoll_event> Eventlist;
	typedef std::vector<SP_Channel> ChannelList;
	Epoll()=delete;
	Epoll(EventLoop* loop);
	void poll(ChannelList* activeChannels);
	void add_timer(SP_Channel request_data, int timeout);
	void epollAdd(SP_Channel,int timeout);
	void epollDel(SP_Channel);
	void epollMod(SP_Channel);
	void assertInLoopThread();
	void startQueue()
	{
		m_timerQueue.start();
	}
private:
	void fillActiveChannels(int numEvents,ChannelList* activeChannels)const;
private:
	static const int MAX=65536;
	EventLoop* m_ownerLoop;
	int m_epollFd;
	Eventlist m_events;
	TimerQueue m_timerQueue;
	SP_HttpData m_httpDatas[MAX];
	SP_Channel m_channels[MAX];
};
