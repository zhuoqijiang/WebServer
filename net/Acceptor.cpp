#include"Acceptor.h"
#include"Server.h"
#include"HttpData.h"
#include"Channel.h"
#include"EventLoopThreadPool.h"
#include"../log/Log.h"

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<cstring>
#include<fcntl.h>
#define ACCEPTOR_EVENTS EPOLLIN|EPOLLET|EPOLLRDHUP
Acceptor::Acceptor(EventLoop* loop,int port)
:m_loop(loop),
m_port(port),
m_listenFd(Acceptor::socketBindListen(port)),
m_server(nullptr),
m_channel(new Channel(loop,m_listenFd))
{}
void Acceptor::start(Server* server)
{
	Acceptor::setSocketNonBlocking(m_listenFd);
  	m_server=server;
  	m_channel->setEvents(ACCEPTOR_EVENTS);
  	m_channel->setReadCallback([this](){this->handNewConn();});
  	m_loop->addToPoller(m_channel);
}
void Acceptor::handNewConn()
{
  	struct sockaddr_in clientAddr;
  	memset(&clientAddr, 0, sizeof(struct sockaddr_in));
  	socklen_t clientAddr_len = sizeof(clientAddr);
  	int acceptFd = 0;
  	while ((acceptFd = accept(m_listenFd, (struct sockaddr *)&clientAddr,&clientAddr_len)) > 0) {
    		EventLoop *loop = m_server->getEventLoopThreadPool()->getnextloop();
		if(Acceptor::setSocketNonBlocking(acceptFd)<0){
			LOG_ERROR("setBlocking fault");
		}
		LOG_INFO("New connection:<IP:%s  PORT:%d>",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
    		std::shared_ptr<HttpData> reqInfo(new HttpData(loop, acceptFd));
    		reqInfo->setCallback();
    		loop->queueInLoop([reqInfo](){reqInfo->newEvent();});
  	}
  	m_channel->setEvents(ACCEPTOR_EVENTS);
}
void Acceptor::handThisConn()
{
	m_loop->updatePoller(m_channel); 
}
int Acceptor::socketBindListen(int port)
{
	int listenFd = 0;
  	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenFd<0){
		LOG_ERROR("socket fault");
	}
  	struct sockaddr_in server_addr;
  	bzero((char *)&server_addr, sizeof(server_addr));
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  	server_addr.sin_port = htons(port);
  	bind(listenFd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  	if(listen(listenFd, 2048)<0){
		LOG_ERROR("listen fault");
	}
  	return listenFd;
}
int Acceptor::setSocketNonBlocking(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
  	if (flag == -1)
	       	return -1;
	flag |= O_NONBLOCK;
  	if (fcntl(fd, F_SETFL, flag) == -1) 
		return -1;
  	return 0;
}




