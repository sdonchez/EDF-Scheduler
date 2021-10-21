////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Task.cpp - definitions for the object representing a decryption task that the
// EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Task.h"

Task::Task(const TimeUnit* deadline, const unsigned int unitsToExecute, 
	const unsigned int taskId, const std::string taskName = NULL, 
	const unsigned int period = 0) :
	deadline(deadline), unitsToExecute(unitsToExecute), taskName(taskName), 
	taskId(taskId),	period(period)
{
	//empty ctor
} //ctor

Task::Task(const nlohmann::json task, const TimeUnit* deadline): 
	deadline(deadline), unitsToExecute(task["unitsToExecute"]), 
	taskName(task["taskName"]),taskId(task["taskId"]), period(task["period"])
{
	//empty ctor
}

Task::~Task()
{
	//empty dtor
} //dtor

/**
 * Since the constructor sets the period to zero if the task is aperiodic
 * simply checks if period is greater than 0
*/
bool Task::isPeriodic()
{
	return (this->period > 0);
} //isPeriodic

unsigned int Task::unitsToDeadline(TimeUnit* currentUnit)
{
	return TimeUnit::unitDiff(*this->deadline, *currentUnit);
} //unitsToDeadline

unsigned int Task::unitsRemaining()
{
	return this->unitsToExecute - this->unitsExecuted;
} //unitsRemaining

#ifdef TARGET_MS_WINDOWS
/**
 * For Windows, there's nothing to actually do with the task, so this function
 * just increments the number of unitsExecuted
*/ 
void Task::executeForTimeUnit()
{
	this->unitsExecuted++;
} //executeForTimeUnit

#else
void Task::executeForTimeUnit()
{
	//TODO: determine implementation for SoC
}

#endif