////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Task.cpp - definitions for the object representing a decryption task that the
// EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Task.h"

Task::Task(const TimeUnit* deadline, const unsigned int unitsToExecute, 
	const unsigned int taskId, int oUD[], const std::string taskName,
	const unsigned int period) :
	deadline(deadline), unitsToExecute(unitsToExecute), taskName(taskName), 
	taskId(taskId),	period(period), oUD(oUD)
{
	oUD[this->deadline->unitNum] += this->unitsToExecute;
} //ctor

#ifdef USE_JSON
Task::Task(const nlohmann::json task, const TimeUnit* deadline, 
	int oUD[]):
	deadline(deadline), unitsToExecute(task["unitsToExecute"]), 
	taskName(task["taskName"]),taskId(task["taskId"]), period(task["period"]),
	oUD(oUD)
{
	oUD[this->deadline->unitNum] += this->unitsToExecute;
}

#else
Task::Task(const pugi::xml_document* task, const TimeUnit* deadline,
	int oUD[]) :
	deadline(deadline), 
	unitsToExecute(atoi(task->child("Task").attribute("unitsToExecute").value())),
	taskName(task->child("Task").attribute("taskName").value()), 
	taskId(atoi(task->child("Task").attribute("taskId").value())), 
	period(atoi(task->child("Task").attribute("period").value())),
	oUD(oUD)
{
	oUD[this->deadline->unitNum] += this->unitsToExecute;
}

#endif

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
	this->oUD[this->deadline->unitNum] -= 1;
} //executeForTimeUnit

#else
void Task::executeForTimeUnit()
{
	//TODO: determine implementation for SoC
}

#endif //TARGET_MS_WINDOWS