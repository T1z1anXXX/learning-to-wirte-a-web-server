#include <stdarg.h>
#include <cassert>
#include <sys/stat.h>
#include <thread>
#include <sys/time.h>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "log.h"
#include "blockqueue.h"

Log::Log(){
	lineCount_ = 0;
	isAsync_ = false;
	writeThread_ = nullptr;
	deque_ = nullptr;
	toDay_ = 0;
	fp_ = nullptr;
}

void Log::flush(){
	if(isAsync_) deque_->flush();
	fflush(fp_);
}

Log::~Log(){
	if(writeThread_ && writeThread_->joinable()){
		while(!deque_->empty()){
			deque_->flush();
		}
		deque_->stop();
		writeThread_->join();
	}
	if(fp_){
		std::lock_guard<std::mutex> locker(mtx_);
		flush();
		fclose(fp_);
	}
}

int Log::GetLevel(){
	std::lock_guard<std::mutex> locker(mtx_);
	return level_;
}

void Log::SetLevel(int level){
	std::lock_guard<std::mutex> locker(mtx_);
	level_ = level;
}

void Log::AsyncWrite_(){
	std::string str = "";
	while(deque_->pop(str)){
		std::lock_guard<std::mutex> locker(mtx_);
		fputs(str.c_str(), fp_);
	}
}

Log* Log::Instance(){
	static Log ins;
	return &ins;
}

void Log::FlushLogThread(){
	Instance()->AsyncWrite_();
}

bool Log::IsOpen(){
	return isOpen_;
}

void Log::init(int level = 1, const char* path, const char* suffix, int maxQueueSize){
	isOpen_ = true;
	level_ = level;
	if(maxQueueSize > 0){
		isAsync_ = true;
		if(!deque_){
			std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
			deque_ = std::move(newDeque);
			std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
			writeThread_ = std::move(newThread);
		}
	}
	else{
		isAsync_ = false;
	}

	lineCount_ = 0;
	time_t timer =std::time(nullptr);
	struct std::tm *sysTime = std::localtime(&timer);
	struct std::tm t = *sysTime;
	
	path_ = path;
	suffix_ = suffix;
	char fileName[LOG_NAME_LEN] = {0};
	std::snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
				path_, t.tm_year+1900, t.tm_mon+1, t.tm_mday, suffix_);
	toDay_ = t.tm_mday;

	{
		std::lock_guard<std::mutex> locker(mtx_);
		buff_.RetrieveAll();
		if(fp_){
			flush();
			fclose(fp_);
		}

		fp_ = std::fopen(fileName, "a");
		if(fp_ == nullptr){
			mkdir(path_, 0777);
			fp_ = std::fopen(fileName, "a");
		}
		assert(fp_ != nullptr);
	}
}

void Log::write(int level, const char* format, ...){
	struct timeval now = {0, 0};
	gettimeofday(&now, nullptr);
	time_t tSec = now.tv_sec;
	struct std::tm *sysTime = std::localtime(&tSec);
	struct std::tm t = *sysTime;
	va_list vaList;

	if(toDay_!=t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))){
		std::unique_lock<std::mutex> locker(mtx_);
		char newFile[LOG_NAME_LEN];
		char tail[36];
		std::snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
		if(toDay_ != t.tm_mday){
			std::snprintf(newFile, LOG_NAME_LEN-72, "%s/%s%s", path_, tail, suffix_);
			toDay_ = t.tm_mday;
			lineCount_ = 0;
		}
		else{
			std::snprintf(newFile, LOG_NAME_LEN-72, "%s/%s-%d%s", path_, tail, (lineCount_/MAX_LINES), suffix_);
		}
		flush();
		fclose(fp_);
		fp_ = std::fopen(newFile, "a");
		assert(fp_!=nullptr);
	}

	{
		std::unique_lock<std::mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if(isAsync_ && deque_ && !deque_->full()) deque_->push_back(buff_.RetrieveAlltoStr()); 
		else fputs(buff_.Peek(), fp_);
        buff_.RetrieveAll();
	}
}

void Log::AppendLogLevelTitle_(int level){
	switch (level) {
		case 0: 
			buff_.Append("[debug]: ", 9);
			break;
		case 1: 
			buff_.Append("[info] : ", 9);
			break;
		case 2:
			buff_.Append("[warn] : ", 9);
			break;
		case 3:
			buff_.Append("[error]: ", 9);
			break;
		default:
			buff_.Append("[info] : ", 9);
			break;
	}
}
