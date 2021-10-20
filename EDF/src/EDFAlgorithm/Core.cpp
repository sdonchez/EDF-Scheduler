////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// Core.cpp - definitions for the object representing an IP Core that the EDF 
// algorithm is scheduling.
////////////////////////////////////////////////////////////////////////////////

#include "Core.h"

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
	currentTask = task;
} //assignTask

