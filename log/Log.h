#pragma once
#include<stdarg.h>
#include<time.h>
#include<string>
#include<memory>
#include<mutex>
#include<fstream>
#define MAXSIZE 2048
#define LOG_DEBUG(...) Logger::log(LogLevel::DEBUG,__LINE__,__FUNCTION__,__VA_ARGS__)
#define LOG_INFO(...) Logger::log(LogLevel::INFO,__LINE__,__FUNCTION__,__VA_ARGS__)
#define LOG_WARN(...) Logger::log(LogLevel::WARN,__LINE__,__FUNCTION__,__VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::ERROR,__LINE__,__FUNCTION__,__VA_ARGS__)

class ThreadPool;
class FileAppender;
class LogLevel{
public:
	enum Level{
		DEBUG,
		INFO,
		WARN,
		ERROR
	};
	static std::string ToString(LogLevel::Level level);
};
class LogTarget{
public:
	enum Target{
		CONSOLE,
		FILE
	};
};
class LogEvent{
public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent(uint32_t line,uint32_t threadId,time_t time,LogLevel::Level level,std::string funName,std::string content);
	std::string handleEvent();
private:
	uint32_t m_line;
	uint32_t m_threadId;
        time_t m_time;
	LogLevel::Level m_level;
	std::string m_funName;
        std::string m_content;
};
class Logger{
public:
	typedef std::shared_ptr<Logger> ptr;
	typedef std::shared_ptr<ThreadPool> SP_ThreadPool;
	static void init(LogLevel::Level level,LogTarget::Target target,std::string fileName="");
	static void log(LogLevel::Level level,uint32_t line,std::string funName,std::string format,...);
	static void stop();
	static void handleOldLog();
private:
	 Logger(){};
private:
	static std::string m_fileName;
	static LogLevel::Level m_level;
	static LogTarget::Target m_target;
	static Logger::ptr m_logger;
	static std::mutex m_mutex;
	static SP_ThreadPool m_threadPool;
};
class Appender{
public:
	typedef std::shared_ptr<Appender> ptr;
	Appender(LogEvent::ptr event);
	virtual void append()=0;
	virtual ~Appender(){}
protected:
	LogEvent::ptr m_event; 
};
class FileAppender:public Appender{
public:
	friend Logger;
	FileAppender(LogEvent::ptr event);
	void append();
private:
	static std::string m_name;
	static int m_fileFd;
};
class ConsoleAppender:public Appender{
public:
	ConsoleAppender(LogEvent::ptr event);
	void append();
};
