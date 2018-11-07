#pragma once
#include <queue>
#include <SDL_mutex.h>
#include "SDL_cpuinfo.h"
#include <SDL_thread.h>
#include <vector>
#include <functional>

// JASON POWER - THREADPOOL CLASS
/////////////////////////////////////
/*
	Threadpool class holds all the threads and workqueue data
	Threads take the getWork function thread name and a pointer to the workqueue data
	Workqueue holds the a queue of task functions, mutex and condition locks.
*/

static int getWork(void* param);

class WorkQueue {
public:
	WorkQueue();
	void addTask(std::function<void()> newTask);
	std::function<void()> nextTask();
	void finish();
	bool hasWork();
private:
	SDL_mutex* m_lockQueue;
	SDL_cond* m_condition;
	std::queue<std::function<void()>> m_tasks;
	bool finished;
};
class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void addTask(std::function<void()> newTask);
	void waitForCompletion();
private:
	std::vector<SDL_Thread*> m_threads;
	WorkQueue workQueue;
};