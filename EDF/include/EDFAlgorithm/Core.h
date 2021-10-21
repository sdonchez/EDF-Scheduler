////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Core.h - header for implementation of the class representing an IP Core that
// the EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Task.h" //since the core is assigned a task

/**
 * @brief Defines an object representing an IP Core that the EDF algorithm is
 * assigning tasks to.
*/
class Core
{
public:

	/**
	 * @brief A unique identifier used to identify this Core in a multi-core
	 * system
	*/
	const unsigned int coreId;

	/**
	 * @brief A pointer to the task object that is currently being processed by
	 * this Core.
	*/
	Task* currentTask;

	/**
	 * @brief Constructor for the Core. Creates the Core object and assigns the
	 *		  CoreId.
	 * @param coreId - The unique identifier to be assigned to this Core.
	*/
	Core(const unsigned int coreId);

	/**
	 * @brief Destructor for the Core.
	*/
	~Core();

	/**
	 * @brief Assigns a new Task object to the current Core
	 * @param task - The Task object to be assigned to the current Core
	*/
	void assignTask(Task* task);

	void executeForTimeUnit();
}; //Core