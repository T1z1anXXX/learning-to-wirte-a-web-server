#include <assert.h>
#include <mutex>
#include <mysql/mysql.h>
#include <semaphore.h>
#include <thread>
#include "sqlconnpool.h"
#include "../log/log.h"

SqlConnPool::SqlConnPool(){
	useCount_ = 0;
	freeCount_ = 0;
}

SqlConnPool* SqlConnPool::Instance(){
	static SqlConnPool connpool;
	return &connpool;
}

void SqlConnPool::Init(const char* host, int port,
						const char* user, const char* pwd,
						const char* dbname, int connsize = 10){
	assert(connsize>0);
	for(int i = 0; i < connsize; i++){
		MYSQL* sql = nullptr;
		sql = mysql_init(sql);
		if(!sql){
			LOG_ERROR("MySql init error!");
			assert(sql);
		}
		sql = mysql_real_connect(sql, host, 
								 user, pwd, 
								 dbname, port, 
								 nullptr, 0);
		if(!sql){
			LOG_ERROR("MySql connect error!");
		}
		connQue_.push(sql);
	}
	MAX_CONN_ = connsize;
	sem_init(&semId_, 0, MAX_CONN_);
}

MYSQL* SqlConnPool::GetConn(){
	MYSQL* sql = nullptr;
	if(connQue_.empty()){
		LOG_WARN("SqlConnPool busy!");
		return nullptr;
	}
	sem_wait(&semId_);
	{
		std::lock_guard<std::mutex> locker(mtx_);
		sql = connQue_.front();
		connQue_.pop();
	}
	return sql;
}

void SqlConnPool::FreeConn(MYSQL* conn){
	assert(conn);
	std::lock_guard<std::mutex> locker(mtx_);
	connQue_.push(conn);
	sem_post(&semId_);
}

int SqlConnPool::GetFreeConnCnt(){
	std::lock_guard<std::mutex> locker(mtx_);
	return connQue_.size();
}

void SqlConnPool::ClosePool(){
	std::lock_guard<std::mutex> locker(mtx_);
	while(!connQue_.empty()){
		MYSQL* item = connQue_.front();
		connQue_.pop();
		mysql_close(item);
	}
	mysql_library_end;
}

SqlConnPool::~SqlConnPool(){
	ClosePool();
}
