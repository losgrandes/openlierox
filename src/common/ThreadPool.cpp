/*
 *  ThreadPool.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.02.09.
 *  code under LGPL
 *
 */

#include <SDL_thread.h>
#include "ThreadPool.h"
#include "Debug.h"

static const unsigned int THREADNUM = 20;

ThreadPool::ThreadPool() {
	nextFunc = NULL; nextParam = NULL;
	mutex = SDL_CreateMutex();
	awakeThread = SDL_CreateCond();
	threadStartedWork = SDL_CreateCond();
	threadFinishedWork = SDL_CreateCond();
	
	notes << "ThreadPool: creating " << THREADNUM << " threads ..." << endl;
	while(availableThreads.size() < THREADNUM)
		prepareNewThread();
}

ThreadPool::~ThreadPool() {
	SDL_mutexP(mutex);
	while(usedThreads.size() > 0) {
		notes << "ThreadPool: waiting for " << usedThreads.size() << " threads to finish" << endl;
		SDL_CondWait(threadFinishedWork, mutex);
	}
	SDL_mutexV(mutex);

	nextFunc = NULL;
	SDL_CondBroadcast(awakeThread);
	for(std::set<ThreadPoolItem*>::iterator i = availableThreads.begin(); i != availableThreads.end(); ++i) {
		SDL_WaitThread((*i)->thread, NULL);
		SDL_DestroyCond((*i)->finishedSignal);
		SDL_DestroyCond((*i)->readyForNewWork);
		delete *i;
	}
	availableThreads.clear();
	
	SDL_DestroyCond(threadStartedWork);
	SDL_DestroyCond(threadFinishedWork);
	SDL_DestroyCond(awakeThread);
	SDL_DestroyMutex(mutex);
}

void ThreadPool::prepareNewThread() {
	ThreadPoolItem* t = new ThreadPoolItem();
	t->pool = this;
	t->finishedSignal = SDL_CreateCond();
	t->readyForNewWork = SDL_CreateCond();
	t->finished = false;
	t->working = false;
	availableThreads.insert(t);
	t->thread = SDL_CreateThread(threadWrapper, t);
}

int ThreadPool::threadWrapper(void* param) {
	ThreadPoolItem* data = (ThreadPoolItem*)param;
	
	SDL_mutexP(data->pool->mutex);
	while(true) {
		SDL_CondWait(data->pool->awakeThread, data->pool->mutex);
		if(data->pool->nextFunc == NULL) break;
		data->pool->usedThreads.insert(data);
		data->pool->availableThreads.erase(data);
		
		ThreadFunc func = data->pool->nextFunc; data->pool->nextFunc = NULL;
		void* param = data->pool->nextParam; data->pool->nextParam = NULL;
		data->finished = false;
		data->working = true;
		SDL_mutexV(data->pool->mutex);
		
		SDL_CondSignal(data->pool->threadStartedWork);
		data->ret = (*func) (param);
		data->finished = true;
		SDL_CondSignal(data->finishedSignal);
		SDL_CondSignal(data->pool->threadFinishedWork);
		
		SDL_mutexP(data->pool->mutex);
		while(data->working) SDL_CondWait(data->readyForNewWork, data->pool->mutex);
		data->pool->usedThreads.erase(data);
		data->pool->availableThreads.insert(data);
	}
	SDL_mutexV(data->pool->mutex);
		
	return 0;
}

ThreadPoolItem* ThreadPool::start(ThreadFunc fct, void* param) {
	SDL_mutexP(mutex);
	if(availableThreads.size() == 0) {
		warnings << "no available thread in ThreadPool, creating new one..." << endl;
		prepareNewThread();
	}
	ThreadPoolItem* data = *availableThreads.begin();
	assert(nextFunc == NULL);
	nextFunc = fct;
	nextParam = param;
	SDL_mutexV(mutex);
	
	SDL_CondSignal(awakeThread);
	SDL_mutexP(mutex);
	while(nextFunc != NULL) SDL_CondWait(threadStartedWork, mutex);
	SDL_mutexV(mutex);
	
	return data;
}

bool ThreadPool::wait(ThreadPoolItem* thread, int* status) {
	if(!thread) return false;
	SDL_mutexP(mutex);
	if(!thread->working) {
		warnings << "given thread is not working" << endl;
		SDL_mutexV(mutex);
		return false;
	}
	while(!thread->finished) SDL_CondWait(thread->finishedSignal, mutex);
	if(status) *status = thread->ret;
	thread->working = false;
	SDL_mutexV(mutex);
	
	SDL_CondSignal(thread->readyForNewWork);
	return true;
}

ThreadPool* threadPool = NULL;

void InitThreadPool() {
	threadPool = new ThreadPool();
}

void UnInitThreadPool() {
	delete threadPool;
	threadPool = NULL;
}


