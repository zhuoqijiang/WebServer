#pragma once
#include<memory>
#include<string>
#include<unistd.h>
#include<map>



class TimerNode;
class EventLoop;
class Channel;
enum ProcessState {
  	STATE_PARSE_URI = 1,
  	STATE_PARSE_HEADERS,
  	STATE_RECV_BODY,
  	STATE_ANALYSIS,
  	STATE_FINISH
};
enum URIState {
  	PARSE_URI_AGAIN = 1,
  	PARSE_URI_ERROR,
  	PARSE_URI_SUCCESS,
};
enum HeaderState {
  	PARSE_HEADER_SUCCESS = 1,
  	PARSE_HEADER_AGAIN,
  	PARSE_HEADER_ERROR
};
enum AnalysisState { 
	ANALYSIS_SUCCESS = 1, 
	ANALYSIS_ERROR
};
enum ParseState {
	H_START = 0,
 	H_KEY,
  	H_COLON,
  	H_SPACES_AFTER_COLON,
  	H_VALUE,
  	H_CR,
  	H_LF,
  	H_END_CR,
  	H_END_LF
};
enum ConnectionState { 
	H_CONNECTED = 0,
       	H_DISCONNECTING,
       	H_DISCONNECTED
};

enum HttpMethod { 
	METHOD_POST = 1,
       	METHOD_GET,
       	METHOD_HEAD
};

enum HttpVersion { 
	HTTP_10 = 1, 
	HTTP_11
};
struct ErrorText{
	int m_errorNum;
	std::string m_errBuffer;
};
class HttpData:public std::enable_shared_from_this<HttpData>{
public:
	typedef std::shared_ptr<TimerNode> SP_TimerNode;
	typedef std::shared_ptr<Channel> SP_Channel;
	typedef std::weak_ptr<TimerNode> WP_TimerNode;
        HttpData(EventLoop* loop,int connfd);
	void setCallback();
        void newEvent();
	void reset();
	void handleClose();
	static void initErrorTexts();
	std::shared_ptr<Channel> getChannel() 
	{
	       	return m_channel;
       	}
        EventLoop* getLoop() 
	{
	       	return m_loop;
       	}
	WP_TimerNode getTimer()
	{
		return m_timer;
	}
	void linkTimer(SP_TimerNode mtimer)
       	{
		m_timer = mtimer;
	}
	ConnectionState connectionState()
	{
		return m_connectionState;
	}
private:
	static void readn(int fd,std::string &inbuffer);
	static void writen(int fd,std::string &outbuffer);
	void handleError(int fd, int err_num);
        void handleRead();
        void handleWrite();
	URIState parseURI();
  	HeaderState parseHeaders();
  	AnalysisState analysisRequest();
private:
        EventLoop* m_loop;
	int m_fd;
        std::string m_inBuffer;
        std::string m_outBuffer;
        SP_Channel m_channel;
	WP_TimerNode m_timer;
private:
	static ErrorText m_errorTexts[20];
	ConnectionState m_connectionState;
	HttpMethod m_method;
	HttpVersion m_HttpVersion;
  	std::string m_fileName;
  	std::string m_path;
  	int m_nowReadPos;
  	ProcessState m_state;
  	ParseState m_hState;
  	std::map<std::string, std::string> m_headers;
	bool m_error;
	bool m_keepAlive;
};

