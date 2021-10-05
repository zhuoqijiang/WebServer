#include"EventLoop.h"
#include"Channel.h"
#include"../log/Log.h"

#include<functional>
#include<mutex>
#include <stdint.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/eventfd.h>
#define DEFAULT_EVENTS EPOLLIN|EPOLLET|EPOLLRDHUP
EventLoop::EventLoop():
m_looping(false),
m_threadId(std::this_thread::get_id()),
t_loopInThisThread(nullptr),
m_quit(true),
m_poller(new Epoll(this)),
m_callingpendingFunctors(false),
m_wakeupFd(EventLoop::createEventFd()), 
m_wakeupChannel(new Channel(this, m_wakeupFd))
{
	m_poller->startQueue();
	if(t_loopInThisThread)
		LOG_ERROR("Thread ERROR!!!");
	else
		t_loopInThisThread=this;
	m_wakeupChannel->setReadCallback([this](){this->handleRead();});
	m_wakeupChannel->setEvents(DEFAULT_EVENTS);
	addToPoller(m_wakeupChannel);
}
EventLoop::~EventLoop()
{
	t_loopInThisThread=nullptr;
}
int EventLoop::createEventFd()
{
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  	return evtfd;	
}
void EventLoop::loop()
{
	assertInLoopThread();
	m_looping=true;
	m_quit=false;
	while(m_looping){
		m_activeChannels.clear();
		m_poller->poll(&m_activeChannels);
		for(auto& channel:m_activeChannels){
			channel->handleEvent();
		}
		doPendingFunctors();
	}
	m_looping=false;
}
void EventLoop::wakeup()
{
	uint64_t one = 1;
  	int n = write(m_wakeupFd, (char*)(&one), sizeof one);
}
void EventLoop::handleRead()
{
	uint64_t one = 1;
  	int n = read(m_wakeupFd, &one, sizeof one);
  	m_wakeupChannel->setEvents(DEFAULT_EVENTS);
}
void EventLoop::runInLoop(const Functor& cb)
{
	if(isInLoopThread())
		cb();
	else
		queueInLoop(cb);
}
void EventLoop::queueInLoop(const Functor& cb)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pendingFunctors.push_back(cb);
	}
	if(!isInLoopThread()||m_callingpendingFunctors)
		wakeup();
}
void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	m_callingpendingFunctors=true;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		functors.swap(m_pendingFunctors);
	}
	for(auto &fun:functors)
		fun();
	m_callingpendingFunctors=false;

}
void EventLoop::abortNotInLoopThread()
{
	LOG_ERROR("Thread ERROR!!!");
}
