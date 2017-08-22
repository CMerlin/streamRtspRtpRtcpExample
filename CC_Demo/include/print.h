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
