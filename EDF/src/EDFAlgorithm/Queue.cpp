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

void TaskQueue::pop()
{
	this->tasks.pop_front();
} //pop

void TaskQueue::insert(Task* task)
{
	this->tasks.push_back(task);
	this->sort();
} //insert

unsigned int TaskQueue::length()
{
	return this->tasks.size();
} //length

bool TaskQueue::swapTest(const Task& task1, const Task& task2)
{
	return (task1.deadline > task2.deadline);
} //swapTest

void TaskQueue::sort()
{
	std::sort(tasks.begin(), tasks.end(), swapTest);
} //sort