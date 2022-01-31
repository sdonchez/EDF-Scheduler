#include "GeneratedTask.h"

GeneratedTask::GeneratedTask(int const volatile* const oUD, 
	unsigned int const volatile* const currUnit,	unsigned int utilization)
{
	this->taskId = std::rand();
	this->taskName = "foobar"; //for lack of an arbitrary word generator
	this->unitsToExecute = std::rand() % 100; //TODO: Parameterize this
	this->period = 0; //TODO: Parameterize this
	this->deadline = computeDeadline(oUD, currUnit, utilization,
		this->unitsToExecute);
} //ctor

GeneratedTask::~GeneratedTask()
{
	//empty dtor
}  //dtor

unsigned int GeneratedTask::computeDeadline(int const volatile* const oUD,
	unsigned int const volatile* const currUnit, unsigned int utilization,
	unsigned int unitsToExecute)
{
	bool found = false;
	unsigned int guess = 0;
	unsigned int range = 0;
	std::cout << "cD - currUnit=" << *currUnit;
	unsigned int minDeadline = *currUnit + unitsToExecute; //eliminates edge case of TU change
	std::cout << " mD=" << minDeadline << std::endl;
	while (!found)
	{
		range = UNITS_TO_SIM - minDeadline;
		guess = minDeadline + std::rand() % range;
		std::cout << "uCheckPre" << std::endl;
		found = utilizationCheck(oUD, currUnit, utilization, guess);
		std::cout << "uCheckPost" <<std::endl;

		minDeadline = *currUnit + unitsToExecute; //refresh in case of TU change;
	} //while
	return guess;
}

bool GeneratedTask::utilizationCheck(int const volatile* const oUD,
	unsigned int const volatile* const currUnit, unsigned int utilization,
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
	for (unsigned int i = 0; i <= unitNum; i++)
	{
		runningSum += oUD[i];
	} //for
	//multiplying by 100 to convert to a percentage corresponding to the value 
	//of utilization.
	unsigned int load = runningSum / (NUM_CORES * (unitNum - *currUnit)) * 100;
	return ( load < utilization);
} //utilizationCheck

#ifdef USE_JSON
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

#else
std::string GeneratedTask::toXMLSerialized()
{

	//initialize document with single node
	pugi::xml_document xmlTaskDoc;
	pugi::xml_node xmlTask = xmlTaskDoc.append_child("Task");

	//create attributes
	pugi::xml_attribute taskName = xmlTask.append_attribute("taskName");
	pugi::xml_attribute taskId = xmlTask.append_attribute("taskId");
	pugi::xml_attribute unitsToExecute = xmlTask.append_attribute("unitsToExecute");
	pugi::xml_attribute deadline = xmlTask.append_attribute("deadline");
	pugi::xml_attribute period = xmlTask.append_attribute("period");

	//set attribute values
	taskName.set_value(this->taskName.c_str());
	taskId.set_value(this->taskId);
	unitsToExecute.set_value(this->unitsToExecute);
	deadline.set_value(this->deadline);
	period.set_value(this->period);

	//init writer (write to string)
	xml_string_writer xmlTaskSerialized;
	xmlTaskDoc.save(xmlTaskSerialized);

	//return serialized output
	return xmlTaskSerialized.result;
}

#endif
