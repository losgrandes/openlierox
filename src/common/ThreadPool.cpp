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
	nextAction = NULL; nextIsHeadless = false; nextData = NULL;
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
		warnings << "ThreadPool: waiting for " << usedThreads.size() << " threads to finish:" << endl;
		for(std::set<ThreadPoolItem*>::iterator i = usedThreads.begin(); i != usedThreads.end(); ++i) {
			if((*i)->working && (*i)->finished) {
				warnings << "thread " << (*i)->name << " is ready but was not cleaned up" << endl;
				(*i)->working = false;
				SDL_CondSignal((*i)->readyForNewWork);	
			}
			else if((*i)->working && !(*i)->finished) {
				warnings << "thread " << (*i)->name << " is still working" << endl;
			}
			else {
				warnings << "thread " << (*i)->name << " is in an invalid state" << endl;
			}
		}
		SDL_CondWait(threadFinishedWork, mutex);
	}
	SDL_mutexV(mutex);

	nextAction = NULL;
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
		if(data->pool->nextAction == NULL) break;
		data->pool->usedThreads.insert(data);
		data->pool->availableThreads.erase(data);
		
		Action* act = data->pool->nextAction; data->pool->nextAction = NULL;
		data->headless = data->pool->nextIsHeadless;
		data->name = data->pool->nextName;
		data->finished = false;
		data->working = true;
		data->pool->nextData = data;
		SDL_mutexV(data->pool->mutex);
		
		SDL_CondSignal(data->pool->threadStartedWork);
		data->ret = act->handle();
		delete act;
		SDL_mutexP(data->pool->mutex);
		data->finished = true;

		if(!data->headless) { // headless means that we just can clean it up right now without waiting
			SDL_CondSignal(data->finishedSignal);
			while(data->working) SDL_CondWait(data->readyForNewWork, data->pool->mutex);
		} else
			data->working = false;
		data->pool->usedThreads.erase(data);
		data->pool->availableThreads.insert(data);
		SDL_CondSignal(data->pool->threadFinishedWork);
	}
	SDL_mutexV(data->pool->mutex);
		
	return 0;
}

ThreadPoolItem* ThreadPool::start(Action* act, const std::string& name, bool headless) {
	SDL_mutexP(mutex);
	if(availableThreads.size() == 0) {
		warnings << "no available thread in ThreadPool for " << name << ", creating new one..." << endl;
		prepareNewThread();
	}
	assert(nextAction == NULL);
	nextAction = act;
	nextIsHeadless = headless;
	nextName = name;
	assert(nextData == NULL);
	SDL_mutexV(mutex);
	
	SDL_CondSignal(awakeThread);
	SDL_mutexP(mutex);
	while(nextData == NULL) SDL_CondWait(threadStartedWork, mutex);
	ThreadPoolItem* data = nextData; nextData = NULL;
	SDL_mutexV(mutex);
		
	return data;
}

ThreadPoolItem* ThreadPool::start(ThreadFunc fct, void* param, const std::string& name) {
	struct StaticAction : Action {
		ThreadFunc fct; void* param;
		int handle() { return (*fct) (param); }
	};
	StaticAction* act = new StaticAction();
	act->fct = fct;
	act->param = param;
	ThreadPoolItem* item = start(act, name);
	if(item) return item;
	delete act;
	return NULL;
}

bool ThreadPool::wait(ThreadPoolItem* thread, int* status) {
	if(!thread) return false;
	SDL_mutexP(mutex);
	if(!thread->working) {
		warnings << "given thread " << thread->name << " is not working anymore" << endl;
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
	if(!threadPool)
		threadPool = new ThreadPool();
	else
		errors << "ThreadPool inited twice" << endl;
}

void UnInitThreadPool() {
	if(threadPool) {
		delete threadPool;
		threadPool = NULL;
	} else
		errors << "ThreadPool already uninited" << endl;
}


