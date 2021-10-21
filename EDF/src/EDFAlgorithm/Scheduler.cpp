#include "scheduler.h"

std::vector<Core*> cores;
std::vector<TimeUnit*> timeUnits;
std::vector<TimeUnit*>::iterator currentUnit = timeUnits.begin();
TaskQueue tasks;
bool doServiceCores = true;
bool doStartUnit = true;


void initCores(unsigned int numCores)
{
	for (int i = 0; i < numCores; i++)
	{
		cores.push_back(new Core(i));
	}
}

void initTimeUnits(unsigned int numUnits, unsigned int numClocks)
{
	for (int i = 0; i < numUnits; i++)
	{
		timeUnits.push_back(new TimeUnit(i, numClocks));
	}
}

unsigned int findTimeUnit(unsigned int unitId)
{
	auto unit = std::find(timeUnits.begin(), timeUnits.end(), unitId);
	return unit - timeUnits.begin();
}

void processNewTask(nlohmann::json jsonTask)
{
	TimeUnit* deadline = timeUnits[findTimeUnit(jsonTask["unitNum"])];
	Task* newTask = new Task(jsonTask, deadline);
	tasks.insert(newTask);
}

void serviceCore(Core& core)
{
	if (!tasks.isEmpty())
	{
		if (!core.currentTask)
		{
			core.currentTask = tasks.fetchAndPop();
		}
		else if (core.currentTask->deadline > tasks.earliestDeadline())
		{
			core.currentTask = tasks.swapTaskToQueue(core.currentTask);
		}
	}

	if (core.currentTask)
	{
		core.currentTask->executeForTimeUnit();
	}
}

void CALLBACK timerCallback(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime)
{
	doServiceCores = true;
	doStartUnit = true;
	std::advance(currentUnit, 1);
}

nlohmann::json inputToJson(char* inputString)
{

}

void coreServicerThread()
{
	while (true)
	{
		if (doServiceCores)
		{
			std::for_each(std::begin(cores), std::end(cores), serviceCore);
			doServiceCores = false;
		}
	}
}

void timerManagerThread()
{
	while (*currentUnit <= timeUnits.back())
	{
		if (doStartUnit)
		{
			SetTimer(NULL, 0, CLOCKS_PER_UNIT/CLOCKS_PER_SEC, 
				(TIMERPROC)&timerCallback);
		}
	}
	return;
}

void taskParserThread()
{
	std::string taskStr = nullptr;

	size_t size;
	while (true)
	{
		std::getline(std::cin, taskStr, '\n');
		processNewTask(nlohmann::json::parse(
			std::getline(std::cin, taskStr, '\n')));
		
		//TODO: Need to gracefully end this thread?
	}
}

int main(unsigned int argc, char* argv[])
{

	initCores(NUM_CORES);
	initTimeUnits(UNITS_TO_SIM, CLOCKS_PER_UNIT);
	//TODO: parse input in blocking thread

	std::thread servicerThread(coreServicerThread);
	std::thread timerThread(timerManagerThread);

	timerThread.join();
	return 0;
}