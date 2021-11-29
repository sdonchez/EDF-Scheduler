#include "GeneratedTask.h"

GeneratedTask::GeneratedTask(int const volatile* const oUD, 
	int const volatile* const currUnit,	unsigned int utilization)
{
	this->taskId = std::rand();
	std::hash<unsigned int> idHash;
	this->taskName = idHash(taskId); //for lack of an arbitrary word generator
	this->unitsToExecute = std::rand() % 100; //TODO: Parameterize this
	this->period = 0; //TODO: Parameterize this
	this->deadline = computeDeadline(oUD, currUnit, utilization);
} //ctor

GeneratedTask::~GeneratedTask()
{
	//empty dtor
}  //dtor

unsigned int GeneratedTask::computeDeadline(int const volatile* const oUD,
	int const volatile* const currUnit, unsigned int utilization)
{
	bool found = false;
	unsigned int guess = 0;
	unsigned int range = 0;
	unsigned int currFixed = *currUnit; //eliminates edge case of TU change
	while (!found)
	{
		range = UNITS_TO_SIM - currFixed;
		guess = currFixed + std::rand() % range;
		found = utilizationCheck(oUD, currUnit, utilization, guess);

		currFixed = *currUnit;
	} //while
	return guess;
}

bool GeneratedTask::utilizationCheck(int const volatile* const oUD,
	int const volatile* const currUnit, unsigned int utilization,
	unsigned int unitNum)
{
	//sanity check in case the TU changed between computing range and verifying
	//guess value.
	if (unitNum < *currUnit)
	{
		return false;
	} //if

	/**
	 * the average utilization of the processor between the currentUnit and a
	 * a given TimeUnit can be determined by computing the sum of all
	 * outstanding units due up to that TimeUnit (including those that have
	 * missed their deadlines, since they are soft deadlines), and dividing this
	 * value by the number of units to that TimeUnit mutliplied by the
	 * parallelization factor (the number of cores sharing the load).
	 * Adding the total number of units to execute for the new task to the sum
	 * provides an indication as to whether or not adding the task will cause
	 * the Scheduler to exceed the target utilization.
	 * 
	*/
	int runningSum = this->unitsToExecute;
	for (int i = 0; i <= unitNum; i++)
	{
		runningSum += oUD[i];
	} //for

	//multiplying by 100 to convert to a percentage corresponding to the value 
	//of utilization.
	unsigned int load = runningSum / (NUM_CORES * (unitNum - *currUnit)) * 100;
	return ( load < utilization);
} //utilizationCheck

std::string GeneratedTask::toJson()
{
	nlohmann::json j;
	j["taskName"] = this->taskName;
	j["taskId"] = this->taskId;
	j["unitsToExecute"] = this->unitsToExecute;
	j["deadline"] = this->deadline;
	j["period"] = this->period;

	return j.dump();
} //toJson