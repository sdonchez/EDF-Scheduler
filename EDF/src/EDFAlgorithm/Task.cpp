////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Task.cpp - definitions for the object representing a decryption task that the
// EDF algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Task.h"

Task::Task(const TimeUnit deadline, const unsigned int unitsToExecute, 
	const unsigned int taskId, const std::string taskName = NULL, 
	const unsigned int period = 0) :
	deadline(deadline), unitsToExecute(unitsToExecute), taskName(taskName), 
	taskId(taskId),	period(period)
{
	//empty ctor
} //ctor

Task::~Task()
{
	//empty dtor
} //dtor

bool Task::isPeriodic()
{
	return (this->period > 0);
} //isPeriodic

unsigned int Task::unitsToDeadline(TimeUnit currentUnit)
{
	return TimeUnit::unitDiff(this->deadline, currentUnit);
} //unitsToDeadline

unsigned int Task::unitsRemaining()
{
	return this->unitsToExecute - this->unitsExecuted;
} //unitsRemaining

#ifdef TARGET_MS_WINDOWS
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