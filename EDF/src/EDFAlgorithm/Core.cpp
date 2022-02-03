////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Core.cpp - definitions for the object representing an IP Core that the EDF 
// algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Core.h"

/**
 * Assigns null to the currentTask, since there isn't one when the Core is 
 * created.
*/
Core::Core(const unsigned int coreId) : coreId(coreId)
{
	this->currentTask = nullptr;
} //ctor

Core::~Core()
{
	//empty dtor
} //dtor

void Core::assignTask(Task* task)
{
	this->currentTask = task;
} //assignTask

void Core::executeForTimeUnit()
{
	(*currentTask).executeForTimeUnit();
	if (this->currentTask->unitsRemaining() <= 0)
	{
		this->currentTask = nullptr;
	}
}

