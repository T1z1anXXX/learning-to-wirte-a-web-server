#include <chrono>
#include <iostream>
#include <thread>

#include "threadpool.h"

int main(){
	ThreadPool threadpool(2);
	for(int i = 0; i < 10; i++){
		threadpool.AddTask([i]{
				std::cout<<"task: "<<i<<" is running"<<std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout<<"task: "<<i<<" is done"<<std::endl;
				});
	}
	return 0;
}
