#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

class Buffer{
public:
	Buffer(int initBuffSize = 1024);
	~Buffer() = default;

	size_t WritableBytes() const;
	size_t ReadableBytes() const;
	size_t PrependableBytes() const;

	const char* Peek() const;
	void EnsureWritable(size_t len);
	void HasWritten(size_t len);

	void Retrieve(size_t len);
	void RetrieveUntil(const char* end);
	void RetrieveAll();
	std::string RetrieveAlltoStr();

	const char* BeginWriteConst() const;
	char * BeginWrite();

	void Append(const char* str, size_t len);
	void Append(const void* data, size_t len);
	void Append(const std::string& str);
	void Append(const Buffer& buff);

	ssize_t ReadFd(int fd, int* Errno);
	ssize_t WriteFd(int fd, int* Errno);

private:
	char* BeginPtr_();
	const char* BeginPtr_() const;
	void MakeSpace_(size_t len);

	std::vector<char> buffer_;
	std::atomic<size_t> readPos_;
	std::atomic<size_t> writePos_;
};



#endif
