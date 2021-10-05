#include"EventLoop.h"
#include"Server.h"
#include"../log/Log.h"
#include<unistd.h>
int main(int argc,char **argv)
{
	int threadNum = 1;
  	int port = 80;
  	std::string logPath = "log.txt";
  	int opt;
  	const char *str = "t:l:p:";
  	while ((opt = getopt(argc, argv, str)) != -1) {
    	switch (opt) {
      	case 't':
         	 threadNum = atoi(optarg);
       		 break;
     	case 'l':
        	logPath = optarg;
        	if (logPath.size() < 2 || optarg[0] != '/') {
          		abort();
        	}
        	break;
      	case 'p': 
        	port = atoi(optarg);
        	break;
      	default:
        	break;
    	}

  	}
	Logger::init(LogLevel::DEBUG,LogTarget::FILE,logPath);
	EventLoop loop;
	Server server(&loop,threadNum,port);
	server.start();
	loop.loop();
}
