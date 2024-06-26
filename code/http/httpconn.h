#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdlib.h>

#include "../log/log.h"
#include "../pool/sqlconnRALL.h"
#include "../buffer/buffer.h"
#include "./httprequest.h"
#include "./httpresponse.h"

class HttpConn{
public:
	HttpConn();
	~HttpConn();

	void init(int sockFd, const sockaddr_in& addr);

	ssize_t read(int* saveErrno);
	ssize_t write(int* saveErrno);

	void Close();

	int GetFd() const;
	int GetPort() const;
	const char* GetIP() const;
	sockaddr_in GetAddr() const;

	bool process();
	int toWriteBytes();
	bool isKeepAlive() const;
	
	static bool isET;
	static const char* srcDir;
	static std::atomic<int> userCount;

private:
	int fd_;
	struct sockaddr_in addr_;
	bool isClose_;
	int iovCnt_;
	struct iovec iov_[2];
	
	Buffer readBuff_;
	Buffer writeBuff_;

	HttpRequest request_;
	HttpResponse response_;

};

#endif
