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
	nanoseconds duration{ 10 * numClocks };
	endTime = startTime + duration;
} //start

clock_t TimeUnit::ticksRemaining()
{
	return this->timeRemaining().count() / 10;
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

bool operator!= (const TimeUnit& unit1, const TimeUnit& unit2)
{
	return (unit1.unitNum != unit2.unitNum);
} //operator!=

