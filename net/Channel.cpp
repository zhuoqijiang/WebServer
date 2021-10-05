#include"Channel.h"
Channel::Channel(EventLoop* loop,int fd)
:m_loop(loop),
m_fd(fd),
m_events(0),
m_revents(0)
{}
void Channel::handleEvent()
{
	if(m_revents&EPOLLIN){
		if(m_revents&EPOLLRDHUP){
			if(m_closeCallback)
				m_closeCallback();
		}
		else{
			if(m_readCallback)
				m_readCallback();
		}
	}
	if(m_revents&EPOLLOUT){
                if(m_writeCallback)
                        m_writeCallback();
        }
	
}
