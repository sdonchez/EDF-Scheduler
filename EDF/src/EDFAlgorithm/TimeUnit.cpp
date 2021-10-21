////////////////////////////////////////////////////////////////////////////////
// S.Donchez
// TimeUnit.cpp - function definitions for the atomic unit of time that the
// EDF algorithm is scheduling a task for.
////////////////////////////////////////////////////////////////////////////////
#include "TimeUnit.h"

TimeUnit::TimeUnit(const unsigned int unitNum, const unsigned int numClocks) 
	: unitNum(unitNum), numClocks(numClocks)
{
	//empty ctor
} //ctor

TimeUnit::~TimeUnit()
{
	//empty dtor
} //dtor

void TimeUnit::start()
{
	startTime = system_clock::now();
	nanoseconds duration{ NS_PER_TICK * numClocks };
	endTime = startTime + duration;
} //start

/**
 * Since everything behind the scenes uses nanoseconds, ticks is obtained by 
 * dividing the nanosecond count by 10 (100 MHz clock)
*/
clock_t TimeUnit::ticksRemaining()
{
	return this->timeRemaining().count() / NS_PER_TICK;
} //ticksRemaining

nanoseconds TimeUnit::timeRemaining()
{
	time_point<system_clock, nanoseconds> currTime = system_clock::now();
	if (currTime > this->endTime)
	{
		return (nanoseconds)0;
	} //if
	else
	{
		return endTime - currTime;
	} //else
} //timeRemaining

unsigned int TimeUnit::unitDiff(const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum - unit2.unitNum);
} //unitDiff

//Operator overloads to enable unitId comparison

bool operator> (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum > unit2.unitNum);
} //operator>

bool operator< (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum < unit2.unitNum);
} //operator<

bool operator>= (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum >= unit2.unitNum);
} //operator>=

bool operator<= (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum <= unit2.unitNum);
} //operator<=

bool operator== (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum == unit2.unitNum);
} //operator==

bool operator== (const unsigned int unitNum, const TimeUnit& unit)
{
	return(unitNum == unit.unitNum);
}

bool operator== (const TimeUnit& unit, const unsigned int unitNum)
{
	return(unitNum == unit.unitNum);
}

bool operator!= (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum != unit2.unitNum);
} //operator!=

