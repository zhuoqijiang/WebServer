#include"HttpData.h"
#include"Channel.h"
#include"EventLoop.h"
#include<sys/epoll.h>
#include<iostream>
#include<memory>
#include<string>
#include<cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEFAULT_EVENTS EPOLLIN|EPOLLET|EPOLLRDHUP
#define DEFAULT_TIME 5
#define DEFAULT_KEEP_ALIVE_TIME 10
#define MAX_READ_SIZE 4096
ErrorText HttpData::m_errorTexts[20];
HttpData::HttpData(EventLoop* loop,int connfd):
m_loop(loop),
m_fd(connfd),
m_channel(new Channel(m_loop,connfd)),
m_error(false),
m_connectionState(H_CONNECTED),
m_method(METHOD_GET),
m_HttpVersion(HTTP_11),
m_nowReadPos(0),
 m_state(STATE_PARSE_URI),
m_hState(H_START),
m_keepAlive(false)
{
	m_channel->setEvents(DEFAULT_EVENTS);
}
void HttpData::initErrorTexts()
{
	m_errorTexts[0].m_errorNum=404;
	m_errorTexts[1].m_errorNum=400;
	struct stat sbuf;
	std::string fileName,bodyBuffer,headerBuffer;
	int count=2,findFlag=1;
	for(int i=0;i<count;i++){
		findFlag=1;
		bodyBuffer.clear();
		headerBuffer.clear();
		fileName="html/"+std::to_string(m_errorTexts[i].m_errorNum)+".html";
		headerBuffer+= "HTTP/1.1 " + std::to_string(m_errorTexts[i].m_errorNum) +"\r\n";
  		headerBuffer+= "Content-Type: text/html\r\n";
  		headerBuffer+= "Connection: Close\r\n";
		headerBuffer+= "Server: Jiang's Web Server\r\n";
		int src_fd= open(fileName.c_str(), O_RDONLY, 0);
        	if(stat(fileName.c_str(), &sbuf) < 0||src_fd < 0) {
				close(src_fd);
				findFlag=0;
                		bodyBuffer+= "<html><title>出错了</title>";
        			bodyBuffer+= "<body bgcolor=\"ffffff\">";
        			bodyBuffer+= "<hr><em> Jiang's Web Server</em>\n</body></html>";
        	}
		if(findFlag==0){
        		headerBuffer+="Content-Length: " + std::to_string(bodyBuffer.size()) + "\r\n";
			headerBuffer+="\r\n";
			m_errorTexts[i].m_errBuffer=headerBuffer+bodyBuffer;
			continue;
		}
        	void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        	close(src_fd);
        	if (mmapRet == (void *)-1) {
				bodyBuffer+= "<html><title>出错了</title>";
        			bodyBuffer+= "<body bgcolor=\"ffffff\">";
        			bodyBuffer+= "<hr><em> Jiang's Web Server</em>\n</body></html>";
                		headerBuffer+="Content-Length: " + std::to_string(bodyBuffer.size()) + "\r\n";
				headerBuffer+="\r\n";
				m_errorTexts[i].m_errBuffer=headerBuffer+bodyBuffer;
				munmap(mmapRet, sbuf.st_size);
				continue;
                	
        	}
		headerBuffer+="Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
    		headerBuffer+= "Server: Jiang's Web Server\r\n";
    		headerBuffer+= "\r\n";
        	char *src_addr = static_cast<char *>(mmapRet);
       		m_errorTexts[i].m_errBuffer=headerBuffer+std::string(src_addr, src_addr + sbuf.st_size);
        	munmap(mmapRet, sbuf.st_size);
	}
}
void HttpData::readn(int fd,std::string &inBuffer)
{
	char buf[MAX_READ_SIZE];
	int nread=0;
	int readNum=0;
	while(1){
		if((nread=read(fd,buf,MAX_READ_SIZE))<0){
			if(errno==EINTR)
				continue;
			else if(errno==EAGAIN)
				return;
			else
				return;
		}
		else if(nread==0){
			return;
		}
		readNum+=nread;
		inBuffer+=std::string(buf,buf+nread);
	}
}
void HttpData::writen(int fd,std::string &outBuffer)
{
        int nwrite=0;
        int writeNum=0;
	const char* buf=outBuffer.c_str();
	int len=outBuffer.size();
        while(1){
                if((nwrite=write(fd,buf,len))<0){
                        if(errno==EINTR)
                                continue;
                        else if(errno==EAGAIN)
                                break;
                }
                else if(nwrite==0){
                        break;
		}
               buf+=nwrite;
	       len-=nwrite;
	       writeNum+=nwrite;
        }
	outBuffer=outBuffer.substr(writeNum);
}
void HttpData::setCallback()
{
	auto self=shared_from_this();
	m_channel->setHolder(self);
	m_channel->setReadCallback([self](){self->handleRead();});
        m_channel->setWriteCallback([self](){self->handleWrite();});
	m_channel->setCloseCallback([self](){self->handleClose();});
}
void HttpData::reset() 
{
 	m_fileName.clear();
  	m_path.clear();
  	m_nowReadPos = 0;
  	m_state = STATE_PARSE_URI;
  	m_hState = H_START;
  	m_headers.clear();
  	m_keepAlive = false;
}
void HttpData::handleRead()
{
	do{
    		HttpData::readn(m_fd, m_inBuffer);
    		if (m_connectionState == H_DISCONNECTING) {
      			m_inBuffer.clear();
      			break;
    		}
		if (m_state == STATE_PARSE_URI) {
      			URIState flag = this->parseURI();
      			if (flag == PARSE_URI_AGAIN)
        			break;
      			else if (flag == PARSE_URI_ERROR) {
        			m_inBuffer.clear();
        			m_error = true;
        			handleError(m_fd, 400);
        			break;
      			}
		       	else
        			m_state = STATE_PARSE_HEADERS;
    		}
    	        if (m_state == STATE_PARSE_HEADERS) {
      			HeaderState flag = this->parseHeaders();
      			if (flag == PARSE_HEADER_AGAIN)
       				 break;
      			else if (flag == PARSE_HEADER_ERROR) {
				m_inBuffer.clear();
        			m_error = true;
        			handleError(m_fd, 400);
        			break;
      			}
      
       		 	m_state = STATE_ANALYSIS;
    		}
    		if (m_state == STATE_ANALYSIS) {
     			 AnalysisState flag = this->analysisRequest();
      			if (flag == ANALYSIS_SUCCESS) {
        			m_state = STATE_FINISH;
       				break;
      			}
		       	else {
        			m_error = true;
       				 break;
      			}
    		}
	} while (false);
	if(m_error||m_connectionState == H_DISCONNECTED){
		reset();
		return;		
	}
	else{
		reset();
		if(m_inBuffer.size()>0){
			handleRead();
		}
		if(m_outBuffer.size()>0){
			handleWrite();
		}
	}
}	
void HttpData::handleWrite()
{
	if (!m_error && m_connectionState != H_DISCONNECTED) {
    		HttpData::writen(m_fd, m_outBuffer);
  	}
	uint32_t events=m_channel->events();
	if(m_outBuffer.size()>0){
		if(!(events|EPOLLOUT)){
			m_channel->enableWriting();
			m_loop->updatePoller(m_channel);
		}
	}
	else{
		if(events|EPOLLOUT){
			m_channel->disableWriting();
			m_loop->updatePoller(m_channel);
		}
	}
	if(!m_keepAlive)
		handleClose();
}
void HttpData::handleClose()
{
	m_connectionState = H_DISCONNECTED;
	m_loop->removeFromPoller(m_channel);
}
void HttpData::newEvent()
{
  	m_channel->setEvents(DEFAULT_EVENTS);
  	m_loop->addToPoller(m_channel,DEFAULT_TIME);
}
URIState HttpData::parseURI()
{
	std::string &str=m_inBuffer;
	int pos=str.find('\r',m_nowReadPos);
	if(pos<0){
		return PARSE_URI_AGAIN;
	}
	std::string request_line=str.substr(0,pos);
	if(str.size()>pos+1)
		str=str.substr(pos+1);
	else 
		str.clear();
	int posGet=request_line.find("GET");
	int posHead=request_line.find("HEAD");
	if(posGet>=0){
		pos=posGet;
		m_method=METHOD_GET;
	}
	else if(posHead>=0){
		pos=posHead;
		m_method=METHOD_HEAD;
	}
	else 
		return PARSE_URI_ERROR;
	pos=request_line.find("/",pos);
	if(pos<0){
		m_fileName="html/index.html";
		m_HttpVersion=HTTP_11;
		return PARSE_URI_SUCCESS;
	}
	else{
		int _pos=request_line.find(' ',pos);
		if(_pos<0)
			return PARSE_URI_ERROR;
		else{
			if(_pos-pos>1){
				m_fileName="html/"+request_line.substr(pos+1,_pos - pos - 1);
				 int __pos = m_fileName.find('?');
       				 if (__pos >= 0)
          				m_fileName ="html/"+m_fileName.substr(0, __pos);
			}
			else
				m_fileName = "html/index.html";
		}
		pos = _pos;
	}
	pos = request_line.find("/", pos);
  	if (pos < 0)
    		return PARSE_URI_ERROR;
  	else {
    		if (request_line.size() - pos <= 3)
      			return PARSE_URI_ERROR;
    		else {
			std::string ver = request_line.substr(pos + 1, 3);
      			if (ver == "1.0")
        			m_HttpVersion = HTTP_10;
      			else if (ver == "1.1")
       				 m_HttpVersion = HTTP_11;
      			else
        			return PARSE_URI_ERROR;
    		}
  	}
  	return PARSE_URI_SUCCESS;
}
HeaderState HttpData::parseHeaders()
{
	std::string &str = m_inBuffer;
  	int keyStart = -1, keyEnd = -1, valueStart = -1, valueEnd = -1;
  	int readBegin = 0;
  	bool notFinish = true;
	int i=0;
  	for (;i < str.size() && notFinish; i++) {
    		switch (m_hState) {
      		case H_START: {
        		if (str[i] == '\n' || str[i] == '\r')
			       	break;
       			m_hState = H_KEY;
        		keyStart = i;
        		readBegin = i;
        		break;
      		}
      		case H_KEY: {
        		if (str[i] == ':') {
          			keyEnd = i;
          			if (keyEnd - keyStart <= 0){
					return PARSE_HEADER_ERROR;
				}
          			m_hState = H_COLON;
        		}
		       	else if (str[i] == '\n' || str[i] == '\r')
          			return PARSE_HEADER_ERROR;
        		break;
      		}
      		case H_COLON: {
        		if (str[i] == ' ') {
          			m_hState = H_SPACES_AFTER_COLON;
        		}
		       	else
          			return PARSE_HEADER_ERROR;
        		break;
      		}
      		case H_SPACES_AFTER_COLON: {
        		m_hState = H_VALUE;
        		valueStart = i;
        		break;
      		}
      		case H_VALUE: {
        		if (str[i] == '\r') {
         			m_hState = H_CR;
          			valueEnd = i;
        			if (valueEnd - valueStart <= 0) 
					  return PARSE_HEADER_ERROR;
        		}
		       	else if(i - valueStart > 255)
          			return PARSE_HEADER_ERROR;
        		break;
      		}
      		case H_CR: {
        		if (str[i] == '\n') {
          			m_hState = H_LF;
				std::string key(str.begin() + keyStart, str.begin() + keyEnd);
				std::string value(str.begin() + valueStart, str.begin() + valueEnd);
          			m_headers[key] = value;
          			readBegin = i;
        		}
		       	else
          			return PARSE_HEADER_ERROR;
        		break;
      		}
      		case H_LF: {
       			 if (str[i] == '\r') {
          			m_hState = H_END_CR;
        		 }
			 else {
          			keyStart = i;
 		        	m_hState = H_KEY;
        		}
       			 break;
      		}
      		case H_END_CR: {
        		if (str[i] == '\n') {
          			m_hState = H_END_LF;
       			 }
		       	else
          			return PARSE_HEADER_ERROR;
       			 break;
      		}
      		case H_END_LF: {
        		notFinish = false;
        		keyStart = i;
        		readBegin = i;
        	break;
      		}
			       
    		}
	  }
  	if (m_hState == H_END_LF) {
    		str = str.substr(i);
    		return PARSE_HEADER_SUCCESS;
 	}
  	str = str.substr(readBegin);
  	return PARSE_HEADER_AGAIN;
}
AnalysisState HttpData::analysisRequest()
{
	std::string header;
	header += "HTTP/1.1 200 OK\r\n";
    	if (m_headers.find("Connection") != m_headers.end() &&
        (m_headers["Connection"] == "Keep-Alive" || m_headers["Connection"] == "keep-alive")) {
      		m_keepAlive = true;
      		header+=std::string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    	}
	struct stat sbuf;
    	if (stat(m_fileName.c_str(), &sbuf) < 0) {
      		header.clear();
      		handleError(m_fd, 404);
      		return ANALYSIS_ERROR;
    	}
    	header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
    	header += "Server: Jiang's Web Server\r\n";
    	header += "\r\n";
    	m_outBuffer += header;
    	if (m_method == METHOD_HEAD) 
		return ANALYSIS_SUCCESS;
    	int src_fd = open(m_fileName.c_str(), O_RDONLY, 0);
    	if (src_fd < 0) {
      		m_outBuffer.clear();
      		handleError(m_fd, 404);
      		return ANALYSIS_ERROR;
    	}
    	void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    	close(src_fd);
    	if (mmapRet == (void *)-1) {
      		munmap(mmapRet, sbuf.st_size);
      		m_outBuffer.clear();
      		handleError(m_fd, 404);
      		return ANALYSIS_ERROR;
    	}
    	char *src_addr = static_cast<char *>(mmapRet);
    	m_outBuffer += std::string(src_addr, src_addr + sbuf.st_size);
    	munmap(mmapRet, sbuf.st_size);
    	return ANALYSIS_SUCCESS;
}
void HttpData::handleError(int fd, int errorNum)
{
	int srcIndex=0;
	for(;srcIndex<20;srcIndex++){
		if(m_errorTexts[srcIndex].m_errorNum==errorNum)
			break;
	}
	std::string errBuffer=m_errorTexts[srcIndex].m_errBuffer;
	HttpData::writen(fd,errBuffer);
}
