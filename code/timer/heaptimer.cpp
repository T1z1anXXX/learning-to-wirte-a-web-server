#include "heaptimer.h"

void HeapTimer::swapNode_(size_t i, size_t j){
	assert(i >= 0 && i < heap_.size());
	assert(j >= 0 && j < heap_.size());
	std::swap(heap_[i], heap_[j]);
	ref_[heap_[i].id] = i;
	ref_[heap_[j].id] = j;
}

void HeapTimer::siftup_(size_t i){
	assert(i >= 0 && i < heap_.size());
	size_t j = (i-1)/2;
	while(j >= 0){
		if(heap_[j] < heap_[i]) break;
		swapNode_(i, j);
		i = j;
		j = (i-1)/2;
	}

}

bool HeapTimer::siftdown_(size_t index, size_t n){
	assert(index >= 0 && index < heap_.size());
	assert(n >= 0 && n < heap_.size());
	size_t i = index;
	size_t j = i*2+1;
	while(j < n){
		if(j+1 < n && heap_[j+1]<heap_[j]) j++;
		if(heap_[i] < heap_[j]) break;
		swapNode_(i, j);
		i = j;
		j = i*2+1;
	}
	return i > index;
}

void HeapTimer::del_(size_t index){
	assert(!heap_.empty() && index>=0 && index<heap_.size());
	size_t i = index;
	size_t n = heap_.size();
	assert(i < n);
	swapNode_(i, n-1);
	ref_.erase(heap_.back().id);
	heap_.pop_back();
	
	if(!siftdown_(i, n)){
		siftup_(i);
	}
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb){
	assert(id >= 0);
	size_t i = 0;
	if(!ref_.count(id)){
		i = heap_.size();
		ref_[id] = i;
		heap_.push_back({id, Clock::now()+MS(timeout), cb});
		siftup_(i);
	}
	else{
		i = ref_[id];
		heap_[i].expires = Clock::now()+MS(timeout);
		heap_[i].cb = cb;
		if(!siftdown_(i, heap_.size())){
			siftup_(i);
		}
	}
}

void HeapTimer::doWork(int id){
	if(heap_.empty() || !ref_.count(id)) return;
	size_t i = ref_[id];
	auto node = heap_[i];
	node.cb();
	del_(i);
}

void HeapTimer::adjust(int id, int newExpires){
	assert(!heap_.empty() && !ref_.count(id));
	heap_[ref_[id]].expires = Clock::now()+MS(newExpires);
	siftdown_(ref_[id], heap_.size());
}

void HeapTimer::clear(){
	ref_.clear();
	heap_.clear();
}

void HeapTimer::pop(){
	assert(!heap_.empty());
	del_(0);
}

void HeapTimer::tick(){
	if(heap_.empty()) return;
	while(!heap_.empty()){
		auto node = heap_.front();
		if(std::chrono::duration_cast<MS>(node.expires-Clock::now()).count() > 0) break;
		node.cb();
		pop();
	}
}

int HeapTimer::GetNextTick(){
	tick();
	size_t res = -1;
	if(!heap_.empty()){
		res = std::chrono::duration_cast<MS>(heap_.front().expires-Clock::now()).count();
		if(res<0) res=0;
	}
	return res;
}


