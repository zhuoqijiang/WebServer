#include"Log.h"
#include"ThreadPool.h"
#include<unistd.h>
#include<sys/types.h>
#include <fcntl.h>
#include<vector>
#include<iostream>
Logger::ptr Logger::m_logger=nullptr;
std::mutex Logger::m_mutex{};
Logger::SP_ThreadPool Logger::m_threadPool=nullptr;
LogLevel::Level Logger::m_level=LogLevel::DEBUG;
LogTarget::Target Logger::m_target=LogTarget::CONSOLE;
std::string Logger::m_fileName="";
int FileAppender::m_fileFd{};
std::string FileAppender::m_name="";
std::string LogLevel::ToString(LogLevel::Level level)
{
	switch(level){

	case LogLevel::DEBUG :return "DEBUG";
	case LogLevel::INFO :return "INFO";
	case LogLevel::WARN :return "WARN";
	case LogLevel::ERROR :return "ERROR";
	default:return "NOFOUND";

	}
	return "NOFOUND";
}
std::string LogEvent::handleEvent()
{
	char str[MAXSIZE],timeString[64];
	std::string level=LogLevel::ToString(m_level);
	struct tm* timeStruct=gmtime(&m_time);
	timeStruct->tm_hour+=8;
	strftime(timeString,64,"[%Y-%m-%d] [%H:%M:%S]",timeStruct);
	snprintf(str,MAXSIZE,"[%lu] %s [%s] [%u] [%s] [%u] [%s]\r\n",m_time,timeString,level.data(),m_threadId,m_funName.data(),m_line,m_content.data());
	return std::string(str);
}

LogEvent::LogEvent(uint32_t line,uint32_t threadId,time_t time,LogLevel::Level level,std::string funName,std::string content):
m_line(line),
m_threadId(threadId),
m_time(time),
m_level(level),
m_funName(funName),
m_content(content)
{
}
void Logger::init(LogLevel::Level level,LogTarget::Target target,std::string fileName)
{
	m_level=level;
	m_target=target;
	m_fileName=fileName;
	FileAppender::m_name=fileName;
	FileAppender::m_fileFd=open(fileName.c_str(),O_WRONLY|O_CREAT|O_APPEND,0);
	m_threadPool=SP_ThreadPool(new ThreadPool);
	m_threadPool->start();
}
void Logger::log(LogLevel::Level level,uint32_t line,std::string funName,std::string format,...)
{
	if(level<m_level)
		return;
	uint32_t threadId=pthread_self();
	uint32_t nowTime=time(0);
	std::string content;
	int fmt_status=0;
	int formatLength=format.length();
	va_list valist;
	va_start(valist,format);
	std::string parmString;
	for(int i=0;i<formatLength;i++){
	 	if(i+1<formatLength&&format[i]=='%'){
			switch(format[++i]){
			case 'd':
				parmString=std::to_string(va_arg(valist,int));
				content.append(parmString);
				break;
			case 's':
				parmString=std::string(va_arg(valist,char*));
				content.append(parmString);
				break;
			case 'c':
				content.push_back(static_cast<char>(va_arg(valist,int)));
				break;
			case 'f':
				parmString=std::to_string(va_arg(valist,double));
				content.append(parmString);
				break;
			case 'l':
				if(i+1<formatLength){
					if(format[i+1]=='f'){
						i++;
						parmString=std::to_string(va_arg(valist,double));
						content.append(parmString);
					}
					else if(format[i+1]=='d'){
						i++;
						parmString=std::to_string(va_arg(valist,long long));
						content.append(parmString);
					}
					else
						return;
				}
				break;
			case '%':
				content.push_back('%');
				break;
			}
		}
		else{
			content.push_back(format[i]);
		}
	}
	LogEvent::ptr event(new LogEvent(line,threadId,nowTime,level,funName,content));
	Appender::ptr appender=nullptr;
	if(m_target==LogTarget::FILE){
		appender=Appender::ptr(new FileAppender(event));
	}
	else{
		appender=Appender::ptr(new ConsoleAppender(event));
	}
	m_threadPool->add(appender);
}
void Logger::handleOldLog()
{
	m_threadPool->handleOldLog();
}
void Logger::stop()
{
	m_threadPool->stop();
}
Appender::Appender(LogEvent::ptr event)
:m_event(event)
{
}
FileAppender::FileAppender(LogEvent::ptr event)
:Appender(event)
{
}
ConsoleAppender::ConsoleAppender(LogEvent::ptr event)
:Appender(event)
{
}
void FileAppender::append()
{
	
	std::string outString=m_event->handleEvent();
	write(m_fileFd,outString.c_str(),outString.size());
}
void ConsoleAppender::append()
{
	std::string outString=m_event->handleEvent();
	std::cout<<outString;
}
