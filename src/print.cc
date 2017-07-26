//#include "print.h"
#ifndef _PRINT_H_
#define _PRINT_H_
#if 1
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
using namespace std;
//using std::vector;

#endif

const int LIBE = 6; //libErroe:LIBE
const int LIBR = 5; //libRelease:LIBT
const int LIBD = 4; //libDebug:LIBD

const int USERE  = 3; //userError:USERE
const int USERR =  2; //userRelease:USERR
const int USERD = 1; //userDebug:USERD
const long int fileSize  = (1024 * 1024 * 2); //日志文件的大小 2MB

/*此结构体用来描述log日志文件的相关信息*/
typedef struct _logFile {
	FILE * file;  //文件描述符
	char path[1024]; //文件存储的路径
	long int size; //日志文件限值（文件大小, 单位byte）
} LOGFILE, *P_LOGFILE;

class printInfo
{
	public:
		printInfo(int level);
		printInfo(int level, int time);
		printInfo(int level, const char * path);
		printInfo(int level, int time, const char * path);
		printInfo(char* moduleName, int level);
		printInfo(char *moduleName, int level, int time);
		printInfo(char *moduleName, int level, const char * path);
		printInfo(char *moduleName, int level, int time, const char * path);

		~printInfo();

		/**
		 * @synop:打印信息,使用方法和printf一样，但是第一个参数需要出入打印级别 
		 * @param level:程序的打印级别,可使用的参数为 USERD, USERR, USERD, LIBD, LIBR, LIBE 
		 * **/
		void print(int level, const char * format, ...);
		
		/**
		 * @synop:当调用此函数的时候，会将 errorInfo_ 中的值返回。注意 errorInfo_ 中存储着上一次的错误信息,
		 * 这个信息是是在调用 print的时候写入的,当print的第一个参数是 LIBE,或是 USERE的时候，format 中的信息
		 * 就会被存储到 errorInfo_ 中
		 * **/
		char* printError(); //用户可以将某些出错信息保存在此对象中	
		
		/**
		 * @synop: 此函数的作用是获取日志文件的大小
		 * @return : 日志文件的大小，单位是byte
		 * **/
		inline long int getFileSize();

		/**
		 * @synop: 创建一个日志文件， 日志文件的路径是 logFile_.path指定的路径
		 * **/
		inline void createNewFile();
		/**
		 * @synop:获取系统时间
		 * return:形如2015082414:35:43.103160的字符串，精确到毫秒
		 * **/
		inline string getSysTime();
        
		/**
		 * @synop:打印程序名(本进程模块的名字)和系统时间函数
		 * **/
		inline void printNameAndTime(const char * format, ...);
		//static char errorInfo_[2048];
		char errorInfo_[2048];

	private:
		int time_;
		int level_;
		char moduleName_[256];
		LOGFILE logFile_;
};
//char printInfo::errorInfo_[2048];
#endif


/* 获取指定文件的大小 */
inline long int  printInfo::getFileSize()
{
	long int filesize = -1;        
	struct stat statbuff;    
	if(stat(logFile_.path, &statbuff) < 0){    
		return filesize;    
	}else{    
		filesize = statbuff.st_size;    
	}    
	return filesize;    
}

/*创建新的日志文件，并将老的日志文件删除*/
inline void printInfo:: createNewFile()
{
	/* 创建日志文件 */
	logFile_.file = fopen(logFile_.path, "w");
	if(logFile_.file == NULL)
		printf("logFile create failure!\n");
}

/* 获取系统时间，返回的字符串形如：2015082116:35:09.30165 */
string printInfo::getSysTime()
{
	char buf[64] = {0};
	string ret;
	time_t  now;
	struct tm *ftime;
	struct timeval tv;
	struct timezone tz;

	now = time(NULL);
	ftime = localtime(&now);
	strftime(buf,32,"%Y%m%d%H:%M:%S.",ftime);

	gettimeofday(&tv, &tz);
	sprintf(buf, "%s%ld ", buf, tv.tv_usec);
	ret = buf;
	return ret;
}


/* 打印程序名和系统时间*/
inline void printInfo::printNameAndTime(const char * format, ...)
{
	va_list  ap;
	va_start(ap, format);
	if(time_ == 1) //需要打印时间
	{
		string time;
		time = getSysTime();	
		vprintf(time.c_str(), ap);
		if(logFile_.file != NULL) //将打印信息写入文件
			fprintf(logFile_.file, "%s", time.c_str());
	}

	/* 打印程序的名字*/
	if(0 != strlen(moduleName_))
	{
		char moduleName[256] = {0};
		sprintf(moduleName, "[%s]", moduleName_);
		vprintf(moduleName, ap);
		if(logFile_.file != NULL) //将打印信息写入文件
			fprintf(logFile_.file, "%s", moduleName); //将信息写入到日志文件中
	}

	va_end(ap);
	return ;
}

/* 打印级别分成两大部分第一部分的是库的跟踪；第二部分是应用程序的跟踪
 * 每一种信息有三个级别：debug > release > error
 * 6.libErroe:LIBE
 * 5.libRelease:LIBT
 * 4.libDebug:LIBD
 *
 * 3.userError:USERE
 * 2.userRelease:USERR
 * 1.userDebug:USERD
 * */
void printInfo::print(int level, const char *format, ...)
{
	va_list  ap;
	va_start(ap, format);

	if((level >= USERD) && (level <= USERE) && (level >= level_)) //用户级信息
	{
		printNameAndTime(format);
		vprintf(format, ap);
		if(logFile_.file!= NULL)
		{
			char buf[2048] = {0};
			vsnprintf(buf, 1024, format, ap); //解析用户传输过来的打印信息
			fprintf(logFile_.file, "%s", buf); //将信息写入到日志文件中
			if(logFile_.size <= getFileSize()) //检测日志文件的大小是否超过限定值
				createNewFile();
		}
	}else if((level >= LIBD) && (level_ >= LIBD) && (level >= level_)) //库级信息
	{	
		printNameAndTime(format);
		vprintf(format, ap);
		if(logFile_.file != NULL)
		{
			char buf[2048] = {0};
			vsnprintf(buf, 2048, format, ap); //解析用户传输过来的打印信息
			fprintf(logFile_.file, "%s", buf); //将信息写入到日志文件中
			if(logFile_.size <= getFileSize())//检测日志文件的大小是否超过限定值
				createNewFile();
		}
	}

	if((level == USERE)||(level == LIBE)) //将错误信息存储起来
	{
		memset(errorInfo_, '\0', sizeof(errorInfo_));
		if(strlen(format) <= sizeof(errorInfo_))
			vsnprintf(errorInfo_, 2048, format, ap); //解析用户传输过来的打印信息
	}
	va_end(ap);
}

char* printInfo::printError()
{
	return errorInfo_;
}

/* 打印级别为leve 相关的信息 */
printInfo::printInfo(int level):time_(0), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	memset(logFile_.path, '\0', sizeof(logFile_.path));
	logFile_.size = fileSize;
}


/* 打印级别为leve 相关的信息,并且打印信息的首部是程序名 */
printInfo::printInfo(char* moduleName, int level):time_(0), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	memset(logFile_.path, '\0', sizeof(logFile_.path));
	strcpy(moduleName_, moduleName);
	logFile_.size = fileSize;
}

/* 打印时间和级别相关的信息 */
printInfo::printInfo(int level, int time):time_(time), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	logFile_.size = fileSize;
}

/* 打印时间和级别相关的信息, 并且打印模块名 */
printInfo::printInfo(char *moduleName, int level, int time):time_(time), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	strcpy(moduleName_, moduleName);
	logFile_.size = fileSize;
}

/* 带日志文件的打印 */
printInfo::printInfo(int level, const char * path):time_(0), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	memset(logFile_.path, '\0', sizeof(logFile_.path));
	strcpy(logFile_.path, path);
	logFile_.size = fileSize;

	/* 创建日志文件 */
	logFile_.file = fopen(logFile_.path, "a");
	if(logFile_.file == NULL)
		printf("logFile create failure!\n");
}

/* 带日志文件的打印,并且有程序名 */
printInfo::printInfo(char *moduleName, int level, int time, const char * path):time_(time), level_(level)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	memset(logFile_.path, '\0', sizeof(logFile_.path));
	strcpy(logFile_.path, path);
	logFile_.size = fileSize;

	/* 创建日志文件 */
	logFile_.file = fopen(logFile_.path, "a");
	if(logFile_.file == NULL)
		printf("logFile create failure!\n");

	strcpy(moduleName_, moduleName);
}

/* 带日志文件的打印 */
printInfo::printInfo(char *moduleName, int level, const char * path)
{
	memset(&logFile_, '\0', sizeof(LOGFILE));
	memset(moduleName_, '\0', sizeof(moduleName_));
	memset(logFile_.path, '\0', sizeof(logFile_.path));
	strcpy(logFile_.path, path);
	logFile_.size = fileSize;

	/* 创建日志文件 */
	logFile_.file = fopen(logFile_.path, "a");
	if(logFile_.file == NULL)
		printf("logFile create failure!\n");

	strcpy(moduleName_, moduleName);
}

printInfo::~printInfo()
{
}


int mainPrint(int argc, const char *argv[])
{
#if 1
	/*输出的目标字符串前面会加上系统时间*/
	const int TIME = 1;
	printInfo print(USERD, TIME);
	print.print(USERD, "Hello!\n");
	//输出结果为：2015082415:08:35.789772 Hello!
#endif

	return 0;
}

