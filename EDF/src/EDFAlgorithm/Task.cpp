////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Task.cpp - definitions for the object representing a decryption task that the
// EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Task.h"

Task::Task(const TimeUnit* deadline, const unsigned int unitsToExecute, 
	const unsigned int taskId, const std::string taskName, 
	const unsigned int period) :
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
#ifdef DEBUG_TASKS
	cout << "isPeriodic called for Task " << this->taskId << std::endl;
#endif //DEBUG_TASKS
	return (this->period > 0);
} //isPeriodic

unsigned int Task::unitsToDeadline(TimeUnit* currentUnit)
{
#ifdef DEBUG_TASKS
	cout << "unitsToDeadline called for Task " << this->taskId << std::endl;
#endif //DEBUG_TASKS
	return TimeUnit::unitDiff(*this->deadline, *currentUnit);
} //unitsToDeadline

unsigned int Task::unitsRemaining()
{
#ifdef DEBUG_TASKS
	cout << "unitsRemaining Called for Task " << this->taskId << std::endl;
#endif //DEBUG_TASKS
	return this->unitsToExecute - this->unitsExecuted;
} //unitsRemaining

#ifdef TARGET_MS_WINDOWS
/**
 * For Windows, there's nothing to actually do with the task, so this function
 * just increments the number of unitsExecuted
*/ 
void Task::executeForTimeUnit()
{
#ifdef DEBUG_TASKS
	cout << "executeForTimeUnit Called for Task " << this->taskId << std::endl;
#endif //DEBUG_TASKS
	this->unitsExecuted++;
} //executeForTimeUnit

#else
void Task::executeForTimeUnit()
{
	//TODO: determine implementation for SoC
}

#endif //TARGET_MS_WINDOWS