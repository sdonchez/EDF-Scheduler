////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Queue.cpp - definitions for the object representing an IP Core that the EDF 
// algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Queue.h"

TaskQueue::TaskQueue()
{
	//empty ctor
} //ctor

TaskQueue::~TaskQueue()
{
	//empty dtor
}//dtor

Task* TaskQueue::fetchAndPop()
{
	Task* el = this->tasks.front();
	this->tasks.pop_front();
	return el;
} //pop

/**
 * Pushes the task to the end of the queue, then calls sort to get it in order.
*/
void TaskQueue::insert(Task* task)
{
	this->tasks.push_back(task);
	this->sort();
} //insert

size_t TaskQueue::length()
{
	return this->tasks.size();
} //length

bool TaskQueue::isEmpty()
{
	return !((bool) this->length());
}

Task* TaskQueue::swapTaskToQueue(Task* taskToQueue)
{
	//TODO: Handle empty queue (for robustness)
	Task* el = fetchAndPop();
	insert(taskToQueue);
	return el;
}

const TimeUnit* TaskQueue::earliestDeadline()
{
	return this->tasks.front()->deadline;
}

bool TaskQueue::swapTest(const Task* task1, const Task* task2)
{
	return (*task1->deadline < *task2->deadline);
} //swapTest

/**
 * Wrapper for std::sort. std::sort is a hybrid sort, and since only the last
 * element in the array is ever unsorted, effectively always performs an
 * insertion sort.
*/
void TaskQueue::sort()
{
	std::sort(tasks.begin(), tasks.end(), swapTest);
} //sort