#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <netinet/in.h>
#include "../buffer/buffer.h"

class HttpConn{
public:




private:
	int fd_;
	struct sockaddr_in addr_;
	bool isClose_;
	int iovCnt_;
	struct iovec iov_[2];
	
	Buffer readBuff_;
	Buffer writeBuff_;



};

#endif
