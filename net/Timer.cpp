#include "Timer.h"
#include"HttpData.h"
#include"../log/Log.h"
#include<time.h>
#include <unistd.h>
#include <queue>
#include<iostream>
TimerNode::TimerNode(SP_HttpData requestData, int timeout)
:m_deleted(false),
m_httpData(requestData)
{
  	uint64_t nowTime=time(0);
  	m_expiredTime=nowTime+timeout;
}
TimerNode::TimerNode(TimerNode &timerNode)
:m_httpData(timerNode.m_httpData),
m_expiredTime(0)
{}
void TimerNode::update(int timeout) 
{
  	uint64_t nowTime=time(0);
  	m_expiredTime=nowTime+timeout;
}
bool TimerNode::isValid() 
{
  	uint64_t nowTime=time(0);
  	if(nowTime < m_expiredTime)
    		return true;
  	else {
    		this->setDeleted();
    		return false;
  	}
}
void TimerNode::clearReq()
{
  	m_httpData.reset();
  	this->setDeleted();
}
