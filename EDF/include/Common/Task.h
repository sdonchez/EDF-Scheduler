////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Task.h - header for implementation of the class representing a decryption
// task that the EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "nlohmann\json.hpp" //For parsing tasks from stdin
#include "TimeUnit.h" //Since tasks have time constraints
#include <string> //for taskName

/**
 * @brief Implements a decryption task that the EDF algorithm is scheduling.
*/
class Task
{
public:

	/**
	 * @brief The TimeUnit the task needs to be completed by.
	*/
	const TimeUnit* deadline;

	/**
	 * @brief The frequency with which the task is repeated, or 0 if it is
	 *		  aperiodic. (measured in TimeUnits)
	*/
	const unsigned int period;

	/**
	 * @brief The total number of TimeUnits required to complete the task.
	*/
	const unsigned int unitsToExecute;

	/**
	 * @brief The friendly name of the task, if one is provided, or NULL.
	*/
	const std::string taskName;

	/**
	 * @brief The unique numeric identifier for the task.
	*/
	const unsigned int taskId;

	/**
	 * @brief How many units of the task have executed thus far.
	*/
	int unitsExecuted = 0;

	int* oUD;

	/**
	 * @brief Constructor for the Task. Assigns values to all of the task
	 *		  properties.
	 * @param deadline		 - The TimeUnit the task needs to be completed by. 
	 * @param unitsToExecute - The total number of TimeUnits required to 
							   complete the task.
	 * @param taskId		 - The unique numeric identifier for the task.
	 * @param taskName		 - The friendly name of the task, if one is 
							   provided, or NULL.
	 * @param period		 - The frequency with which the task is repeated, or
	 * 0 if it is aperiodic. (measured in TimeUnits)
	*/
	Task(const TimeUnit* deadline, const unsigned int unitsToExecute, 
		const unsigned int taskId, const std::string taskName = NULL, 
		const unsigned int period = 0, int oUD[]);

	Task(const nlohmann::json task, const TimeUnit* deadline, 
		int oUD[]);

	/**
	 * @brief Destructor for the Task instance.
	*/
	~Task();

	/**
	 * @brief Determines if a function is periodic or not
	 * @return True if the function is periodic, False otherwise.
	*/
	bool isPeriodic();

	/**
	 * @brief Calculates the number of TimeUnits remaining until the deadline
	 *		  for a given task.
	 * @param CurrentUnit - The TimeUnit to compare against (typically now())
	 * @return number of TimeUnits remaining
	*/
	unsigned int unitsToDeadline(TimeUnit* CurrentUnit);

	/**
	 * @brief Calculates the number of TimeUnits remainig to execute.
	 * @return Number of TimeUnits remaining to execute for the current Task.
	*/
	unsigned int unitsRemaining();

	/**
	 * @brief Executes the function for a single TimeUnit. Implementation is
	 *		  target dependent.
	*/
	void executeForTimeUnit();

}; //Task

