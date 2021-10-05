#pragma once
#include<memory>
class Server;
class Channel;
class EventLoop;
class Acceptor{
public:
	typedef std::shared_ptr<Channel> SP_Channel;
	Acceptor()=delete;
	Acceptor(EventLoop* loop,int port);
	void start(Server* server);
private:
	static int socketBindListen(int port);
        static int setSocketNonBlocking(int fd);

	void handNewConn();
	void handThisConn();
private:
	EventLoop* m_loop;
	int m_port;
	int m_listenFd;
	SP_Channel m_channel;
	Server* m_server;
};
