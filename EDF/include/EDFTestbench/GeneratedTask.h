////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// GeneratedTask.h - header for implementation of the class which constructs
// Tasks for consumption by the Scheduler app.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CommonDefs.h"
#include "nlohmann\json.hpp" //For parsing tasks from stdin
#include <string> //for taskName
#include <functional>
#include <cstdlib>

/**
 * @brief Generates a Task for the Scheduler to parse and schedule.
*/
class GeneratedTask
{
public:

	/**
	 * @brief The TimeUnit the task needs to be completed by.
	*/
	unsigned int deadline;

	/**
	 * @brief The frequency with which the task is repeated, or 0 if it is
	 *		  aperiodic. (measured in TimeUnits)
	*/
	unsigned int period;

	/**
	 * @brief The total number of TimeUnits required to complete the task.
	*/
	unsigned int unitsToExecute;

	/**
	 * @brief The friendly name of the task, if one is provided, or NULL.
	*/
	std::string taskName;

	/**
	 * @brief The unique numeric identifier for the task.
	*/
	unsigned int taskId;

	/**
	 * @brief constructor for the GeneratedTask instance. Assigns random values
	 *        to all values but deadline, computes a deadline that conforms to
	 *        target utilization.
	 * @param oUD         - pointer to the Scheduler's outstandingUnitsToExecute
	 *                      array. Used for Deadline Computation.
	 * @param currUnit    - pointer to the unitNum of the Scheduler's currently
	 *                      executing TimeUnit. Used for Deadline Computation.
	 * @param utilization - target utilization percentage for the set of cores
	 *                      as a whole. Used for Deadline Computation.
	 * @note pointers are both const and volatile (memory location doesn't 
	 *       change, but every read needs to happen since the scheduler app is
	 *       constantly updating it). The contents of the array are also const,
	 *		 since the testbench is never modifying them.
	*/
	GeneratedTask(int const volatile* const oUD,
		unsigned int const volatile* const currUnit, unsigned int utilization);

	/**
	 * @brief destructor for the GeneratedTask instance.
	 */
	~GeneratedTask();

	/**
	 * @brief calculates a valid deadline for the new GeneratedTask, which
	 *        conforms to the given maximum average utilization.
	 * @param oUD         - pointer to the Scheduler's outstandingUnitsToExecute
	 *                      array. Used for Deadline Computation.
	 * @param currUnit    - pointer to the unitNum of the Scheduler's currently
	 *                      executing TimeUnit. Used for Deadline Computation.
	 * @param utilization - target utilization percentage for the set of cores
	 *                      as a whole. Used for Deadline Computation.
	 * @note pointers are both const and volatile (memory location doesn't
	 *       change, but every read needs to happen since the scheduler app is
	 *       constantly updating it). The contents of the array are also const,
	 *		 since the testbench is never modifying them.
	 * @return the unitNum of a valid deadline.
	*/
	unsigned int computeDeadline(int const volatile* const oUD,
		unsigned int const volatile* const currUnit, unsigned int utilization,
		unsigned int unitsToExecute);

	/**
	 * @brief 
	 * @param oUD         - pointer to the Scheduler's outstandingUnitsToExecute
	 *                      array. Used for Deadline Computation.
	 * @param currUnit    - pointer to the unitNum of the Scheduler's currently
	 *                      executing TimeUnit. Used for Deadline Computation.
	 * @param utilization - target utilization percentage for the set of cores
	 *                      as a whole. Used for Deadline Computation.
	 * @param unitNum     - the TimeUnit number to verify as a potential
	 *                      deadline. Must be after the current unit.
	 * @note pointers are both const and volatile (memory location doesn't
	 *       change, but every read needs to happen since the scheduler app is
	 *       constantly updating it). The contents of the array are also const,
	 *		 since the testbench is never modifying them.
	 * @return true if the deadline is valid, false otherwise.
	*/
	bool utilizationCheck(int const volatile* const oUD,
		unsigned int const volatile* const currUnit, unsigned int utilization,
		unsigned int unitNum);

	/**
	 * @brief parses the Task into a JSON string such that it can be sent.
	 * @return serialized JSON representation of the Task, including all
	 *         attributes. The deadline is stored simply as its unitNum.
	*/
	std::string toJson();
}; //GeneratedTask