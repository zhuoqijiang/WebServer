#pragma once
#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include"HttpData.h"
class Channel;
class TimerNode {
public:
	typedef std::shared_ptr<HttpData> SP_HttpData;
  	TimerNode(SP_HttpData requestData, int timeout);
  	TimerNode(TimerNode &tn);
  	void update(int timeout);
  	bool isValid();
  	void clearReq();
  	void setDeleted() 
	{ 
		m_deleted = true;
      	}
  	bool isDeleted() const
       	{
	       	return m_deleted;
       	}
  	uint64_t getExpTime() const
       	{
	       	return m_expiredTime;
      	}
  	SP_HttpData getHttpData()
	{
		return m_httpData;
	}
private:
  	bool m_deleted;
  	uint64_t m_expiredTime;
  	SP_HttpData m_httpData;
};

