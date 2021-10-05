#include"Epoll.h"
#include"EventLoop.h"
#include"Channel.h"
#include"../log/Log.h"

#include<sys/epoll.h>
#include<memory>
#include<utility>
#include<unistd.h>
Epoll::Epoll(EventLoop* loop)
:m_ownerLoop(loop),
m_epollFd(epoll_create(5)),
m_timerQueue(loop)
{
	m_events.resize(1000);
}
void Epoll::epollAdd(SP_Channel request,int timeout=0)
{
	int fd = request->fd();
	if(timeout>0){
		add_timer(request, timeout);
      		m_httpDatas[fd] = request->getHolder();
	}
 	epoll_event event;
  	event.data.fd = fd;
  	event.events = request->events();
	m_channels[fd]=request;
  	if(epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &event)<0){
		LOG_ERROR("epoll add error");
		m_channels[fd].reset();
	}
}
void Epoll::epollMod(SP_Channel request)
{
	int fd = request->fd();
  	epoll_event event;
  	event.data.fd = fd;
  	event.events = request->events();
  	if(epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &event)<0){
		LOG_ERROR("epoll mod error");
		m_channels[fd].reset();
	}
}

void Epoll::epollDel(SP_Channel request)
{
        int fd = request->fd();
        epoll_event event;
        event.data.fd = fd;
        event.events = request->events();
        if(epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &event)<0){
		LOG_ERROR("epoll del error");
	}
	m_httpDatas[fd].reset();
	m_channels[fd].reset();
	close(fd);
}
void Epoll::poll(ChannelList* activeChannels)
{
	int eventNum=epoll_wait(m_epollFd,&*m_events.begin(),static_cast<int>(m_events.size()),0);
	if(eventNum>0){
		fillActiveChannels(eventNum,activeChannels);
		if(static_cast<size_t>(eventNum)==m_events.size())
			m_events.resize(m_events.size()*2);
	}
}
void Epoll::fillActiveChannels(int eventNum,ChannelList* activeChannels)const 
{
	for(int i=0;i<eventNum;i++){
		int fd=m_events[i].data.fd;
		SP_Channel channel=m_channels[fd];
		channel->setRevents(m_events[i].events);
		activeChannels->push_back(channel);
	}
}
void Epoll::assertInLoopThread()
{
	m_ownerLoop->assertInLoopThread();
}
void Epoll::add_timer(std::shared_ptr<Channel> requestData, int timeout)
{
	std::shared_ptr<HttpData> httpData= requestData->getHolder();
  	if (httpData)
    		m_timerQueue.addTimer(httpData, timeout);
}
