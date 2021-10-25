#include "scheduler.h"

std::vector<Core*> cores;
std::vector<TimeUnit*> timeUnits;
TimeUnit* currentUnit;
TaskQueue tasks;
bool doServiceCores = true;
bool doStartUnit = true;


void initCores(unsigned int numCores)
{
#ifdef _DEBUG
	std::cout << "Initializing Cores" << std::endl;
#endif // _DEBUG

	for (unsigned int i = 0; i < numCores; i++)
	{
		cores.push_back(new Core(i));
#ifdef DEBUG_CORES
		std::cout << "Created Core " << i << std::endl;
#endif // DEBUG_CORES

	}
}

void initTimeUnits(unsigned int numUnits, unsigned int numClocks)
{
#ifdef _DEBUG
	std::cout << "Initializing TimeUnits" << std::endl;
#endif // _DEBUG
	for (unsigned int i = 0; i < numUnits; i++)
	{
		timeUnits.push_back(new TimeUnit(i, numClocks));
#ifdef DEBUG_TIMEUNITS
		std::cout << "Created TimeUnit " << i << std::endl;
#endif // DEBUG_TIMEUNITS

	}
	currentUnit = timeUnits.front();
}

TimeUnit* findTimeUnit(unsigned int unitNum)
{
#ifdef DEBUG_TIMEUNITS
	std::cout << "findTimeUnit called with unitNum = " << unitNum << std::endl;
#endif
	auto unit = std::find_if(timeUnits.begin(), timeUnits.end(), 
		[&unitNum](const TimeUnit* unit) {
			return unit->unitNum == unitNum;
		});

	if (unit != timeUnits.end())
	{
#ifdef DEBUG_TIMEUNITS
		std::cout << "TimeUnit Found!" << std::endl;
#endif // DEBUG_TIMEUNITS

		return *unit;
	}

#ifdef DEBUG_TIMEUNITS
	std::cout << "TimeUnit not found!" << std::endl;
#endif
	return nullptr;
}

/**
 * expects a JSON containing the fields "unitsToExecute", "taskName", "taskId",
 * and "period".
*/
void processNewTask(nlohmann::json jsonTask)
{
#ifdef _DEBUG
	std::cout << "New Task Received:" << jsonTask.dump() << std::endl;
#endif // _DEBUG

	TimeUnit* deadline = findTimeUnit(jsonTask["deadline"]);
	Task* newTask = new Task(jsonTask, deadline);
#ifdef DEBUG_TASKS
	std::cout << "Created Task object. taskName = " << newTask->taskName <<
		". taskId = " << newTask->taskId << ". unitsToExecute = " <<
		newTask->unitsToExecute << ". Period = " << newTask->period <<
		". unitNum = " << newTask->deadline->unitNum << "." << std::endl;
#endif
	tasks.insert(newTask);
#ifdef DEBUG_TASKS
	std::cout << "Task inserted into TaskQueue." << std::endl;
#endif
}

bool isLatestCore(Core* core)
{

#ifdef DEBUG_CORES
	std::cout << "isLatestCore called for Core # " << core->coreId <<
		std::endl << "Current Deadlines: " << std::endl;
	std::for_each(std::begin(cores), std::end(cores), [](Core* core) {
		std::cout << "Core #" << core->coreId << ": " <<
			core->currentTask->deadline->unitNum << std::endl;
		});
#endif // DEBUG_CORES

	Core* maxCore = *std::max_element(cores.begin(), cores.end(), 
		[](const Core* a, const Core* b)
		{
			return a->currentTask->deadline > b->currentTask->deadline;
		});
	if (core->currentTask->deadline < maxCore->currentTask->deadline)
	{
#ifdef DEBUG_CORES
		std::cout << "Not the latest core!" << std::endl;
#endif // DEBUG_CORES

		return false;
	}

#ifdef DEBUG_CORES
	std::cout << "This is the latest core!" << std::endl;
#endif // DEBUG_CORES
	return true;
}
/**
 * Provided the queue isn't empty, checks to see if the task at the front of the
 * TaskQueue (the earliest deadline) is due before the Core's current task. If 
 * it is, and the current Core is the one with the latest deadline, then the
 * the current Core is swapped with the first Task from the TaskQueue, and the 
 * Core's previous Task is inserted into the TaskQueue in the apropriate place 
 * via the swapTaskToQueue function.
*/
void serviceCore(Core* core)
{
#ifdef DEBUG_CORES
	std::cout << "isLatestCore called for Core # " << core->coreId << std::endl;
#endif
	if (!tasks.isEmpty())
	{
#ifdef DEBUG_CORES
		std::cout << "Tasks exist in TaskQueue." << std::endl;
#endif
		if (!(core->currentTask))
		{
			core->currentTask = tasks.fetchAndPop();
		}
		else if (core->currentTask->deadline > tasks.earliestDeadline() && 
			isLatestCore(core))
		{
			core->currentTask = tasks.swapTaskToQueue(core->currentTask);
		}
	}

	if (core->currentTask)
	{
#ifdef DEBUG_CORES
		std::cout << "Executing Task " << core->currentTask->taskId <<
			"for one TimeUnit" << std::endl;
#endif
		core->executeForTimeUnit();
	}
}

#ifdef TARGET_MS_WINDOWS
void CALLBACK timerCallback(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime)
{
#ifdef DEBUG_TIMER
	std::cout << "Timer Callback called for TimeUnit " << 
		currentUnit->unitNum << std::endl;
#endif // DEBUG_TIMER

	doServiceCores = true;
	doStartUnit = true;
}
#endif

void coreServicerThread()
{
#ifdef DEBUG_CORES
	std::cout << "Core Servicer thread spawned." << std::endl;
#endif
	while (true)
	{
		if (doServiceCores)
		{
#ifdef DEBUG_CORES
			std::cout << "Servicing Cores." << std::endl;
#endif
			std::for_each(std::begin(cores), std::end(cores), serviceCore);
			doServiceCores = false;
		}
	}
}

void timerManagerThread()
{
#ifdef DEBUG_TIMER
	std::cout << "Timer Manager thread spawned." << std::endl;
#endif // DEBUG_TIMER

	while (currentUnit->unitNum < timeUnits.back()->unitNum)
	{
		if (doStartUnit)
		{
#ifdef DEBUG_TIMER
			std::cout << "Starting new TimeUnit." << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS
			SetTimer(NULL, 0, CLOCKS_PER_UNIT/CLOCKS_PER_SEC, 
				(TIMERPROC)&timerCallback);
			currentUnit = findTimeUnit(currentUnit->unitNum + 1);
			doStartUnit = false;
#endif
		}
	}
	return;
}

void taskParserThread()
{
#ifdef DEBUG_TASKS
	std::cout << "Task Parser thread spawned" << std::endl;
#endif
	std::string taskStr = "";
	while (true)
	{
#ifdef DEBUG_TASKS
		std::cout << "Waiting for input" << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS
		std::getline(std::cin, taskStr, '\n');
#endif
#ifdef DEBUG_TASKS
		std::cout << "Input Received" << std::endl;
#endif
		processNewTask(nlohmann::json::parse(taskStr));
	}
}

int main(unsigned int argc, char* argv[])
{
#ifdef _DEBUG
	std::cout << "Beginning Initialization" << std::endl;
#endif
	initCores(NUM_CORES);
	initTimeUnits(UNITS_TO_SIM, CLOCKS_PER_UNIT);
	
#ifdef _DEBUG
	std::cout << "Spawning Threads" << std::endl;
#endif
	std::thread parserThread(taskParserThread);
	std::thread servicerThread(coreServicerThread);
	std::thread timerThread(timerManagerThread);

#ifdef _DEBUG
	std::cout << "Waiting for timer to end" << std::endl;
#endif // _DEBUG

	timerThread.join();

#ifdef _DEBUG
	std::cout << "Timer ended, shutting down." << std::endl;
#endif // _DEBUG
	parserThread.~parserThread();

#ifdef _DEBUG
	std::cout << "Closed parser, Goodbye!" << std::endl;
#endif // _DEBUG

	exit(0);
}