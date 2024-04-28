CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = testHttpRequest
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp \
       ../code/buffer/*.cpp ../test/testHttpRequest.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
