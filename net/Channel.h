#pragma once
#include<sys/epoll.h>
#include<functional>
#include<memory>

class HttpData;
class EventLoop;

class Channel{
public:
	typedef std::function<void()> EventCallback;
	typedef std::shared_ptr<HttpData> SP_HttpData;
	typedef std::weak_ptr<HttpData> WP_HttpData;
	Channel(EventLoop* loop,int fd);
	void handleEvent();
	void setReadCallback(const EventCallback& cb)
	{
		m_readCallback=cb;
	}
	void setWriteCallback(const EventCallback& cb)
	{
		m_writeCallback=cb;
	}
	void setCloseCallback(const EventCallback& cb)
	{
		m_closeCallback=cb;
	}
	int fd() 
	{
		return m_fd;
	}
	uint32_t events()
       	{
		return m_events;
	}
	void setEvents(uint32_t events)
	{
		m_events=events;
	}
	void setRevents(uint32_t revents)
	{
		m_revents=revents;
	}
	void enableReading()
	{
		m_events|=EPOLLIN;
	}
	void enableWriting()
	{
		m_events|=EPOLLOUT;
	}
	void disableWriting()
	{
		m_events&=~EPOLLOUT;
	}
	EventLoop* ownerloop()
	{
		return m_loop;
	}
	void setHolder(SP_HttpData holder)
       	{
		m_holder = holder;
       	}
  	SP_HttpData getHolder()
	{
		SP_HttpData ret(m_holder.lock());
    		return ret;
  	}
private:
	int m_fd;
	EventLoop* m_loop;
	uint32_t m_events;
	uint32_t m_revents;
	WP_HttpData m_holder;
	EventCallback m_readCallback;
	EventCallback m_writeCallback;
	EventCallback m_closeCallback;
};

