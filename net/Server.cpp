#include"Server.h"
#include"EventLoopThreadPool.h"
#include"Channel.h"
#include"EventLoop.h"
#include"HttpData.h"
#include"../log/Log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<cstring>
Server::Server(EventLoop *loop, int threadNum,int port)
:m_loop(loop),
m_threadNum(threadNum),
m_eventLoopThreadPool(new EventLoopThreadPool(m_loop, threadNum)),
m_started(false),
m_acceptor(loop,port)
{}
void Server::start() 
{
	HttpData::initErrorTexts();
  	m_eventLoopThreadPool->start();
  	m_acceptor.start(this);
  	m_started = true;
}
Server::~Server()
{
	LOG_ERROR("Server Over!");
	Logger::stop();
}
                                                                                               
