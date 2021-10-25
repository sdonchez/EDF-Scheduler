////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Core.h - header for implementation of the class representing an IP Core that
// the EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Task.h" //since a queue is full of tasks
#include <deque> //for queue itself
#include <algorithm> //for std::sort 
#include "TimeUnit.h"

/**
 * @brief Implements the queue of tasks that are not currently executing. 
*/
class TaskQueue
{
public:

	/**
	 * @brief The actual queue of tasks pending execution.
	*/
	std::deque<Task*> tasks;

	/**
	 * @brief Constructor for the Queue instance.
	*/
	TaskQueue();

	/**
	 * @brief Destructor for the Queue instance.
	*/
	~TaskQueue();

	/**
	 * @brief wrapper for the Deque::pop_front() function. Pops the first
	 * element from the queue.
	*/
	Task* fetchAndPop();

	/**
	 * @brief inserts the new Task into the queue, sorted by deadline.
	 * @param task 
	*/
	void insert(Task* task);

	/**
	 * @brief returns the number of tasks in the queue.
	 * @return 
	*/
	size_t length();	

	bool isEmpty();

	Task* swapTaskToQueue(Task* taskToQueue);

	const TimeUnit* earliestDeadline();

private:

	/**
	 * @brief Determines if Task 1 is due after Task 2.
	 * @param task1 - The task being sorted against (the one currently in the
	 *				  queue).
	 * @param task2 - The task being sorted into the array.
	 * @return bool - True if Task 1 is due later, False if Task 1 is due 
	 *				  earlier than task 2.
	*/
	static bool swapTest(const Task* task1, const Task* task2);

	/**
	 * @brief Sorts the queue. Effectively sorts a new element into the queue,
	 *		  since the queue is maintained in a sorted format. Sorts in order
	 *		  of ascending deadline (earliest first).
	*/
	void sort();

}; //TaskQueue