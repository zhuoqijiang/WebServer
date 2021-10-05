#include"TimerQueue.h"
#include"HttpData.h"
#include"EventLoop.h"
#include"Channel.h"
#include"../log/Log.h"
#include<sys/timerfd.h>
#include<time.h>
#include<sys/epoll.h>
#include<sys/syscall.h>
#include<iostream>
#define TIMER_EVENTS EPOLLIN|EPOLLRDHUP
int TimerQueue::createTimerFd()
{
	int timerFd=timerfd_create(CLOCK_REALTIME,0);
	return timerFd;
}

TimerQueue::TimerQueue(EventLoop* loop)
:m_loop(loop),
m_one(0),
m_timerFd(TimerQueue::createTimerFd()),
m_channel(new Channel(loop,m_timerFd))
{
}
void TimerQueue::start()
{
        struct itimerspec newValue;
        newValue.it_value.tv_sec =  5;
        newValue.it_value.tv_nsec = 0;
        newValue.it_interval.tv_sec = 5;
        newValue.it_interval.tv_nsec = 0;
        timerfd_settime(m_timerFd, 0, &newValue, nullptr);
	m_channel->setEvents(TIMER_EVENTS);
	if(getpid()!=syscall(SYS_gettid)){
        	m_channel->setReadCallback([this](){this->handleExpiredEvent();});
	}
	else{
		m_channel->setReadCallback([this](){this->handleLogQueue();});
	}
        m_loop->addToPoller(m_channel);	
}
void TimerQueue::addTimer(SP_HttpData HttpData, int timeout)
{
        SP_TimerNode new_node(new TimerNode(HttpData, timeout));
        m_timerNodeQueue.push(new_node);
        HttpData->linkTimer(new_node);
}
void TimerQueue::handleExpiredEvent()
{
	read(m_timerFd,&m_one,sizeof(uint64_t));
 	while (!m_timerNodeQueue.empty()) {
    		SP_TimerNode timerNode = m_timerNodeQueue.top();
    		if (timerNode->isDeleted()){
            		SP_HttpData httpData=timerNode->getHttpData();
            		if((httpData->getTimer()).lock()==timerNode){
				if(httpData->connectionState()!=H_DISCONNECTED){
                    			httpData->handleClose();
				}
			}
      			m_timerNodeQueue.pop();
    		}
    		else if (timerNode->isValid() == false){
           		 SP_HttpData httpData=timerNode->getHttpData();
            		if((httpData->getTimer()).lock()==timerNode){
                    		if(httpData->connectionState()!=H_DISCONNECTED){
                    			httpData->handleClose();
				}
			}
        		m_timerNodeQueue.pop();
    		}
    		else
      			break;
  	}
}
void TimerQueue::handleLogQueue()
{
	read(m_timerFd,&m_one,sizeof(uint64_t));
	Logger::handleOldLog();
}
