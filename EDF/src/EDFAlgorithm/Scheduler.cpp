#include "scheduler.h"

std::vector<Core*> cores;
std::vector<TimeUnit*> timeUnits;
TimeUnit* currentUnit;
TaskQueue tasks;
bool doServiceCores = true;
bool doStartUnit = true;
std::vector<int> outstandingUnitsDue;


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
		outstandingUnitsDue.push_back(0);
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
	Task* newTask = new Task(jsonTask, deadline, outstandingUnitsDue.data());
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
#else
int ScuTimerConfig(XScuGic *IntcInstancePtr, XScuTimer * TimerInstancePtr,
			u16 TimerDeviceId, u16 TimerIntrId)
{
	int Status;
	int LastTimerExpired = 0;
	XScuTimer_Config *ConfigPtr;

	/*
	 * Initialize the Scu Private Timer driver.
	 */
	ConfigPtr = XScuTimer_LookupConfig(TimerDeviceId);

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XScuTimer_SelfTest(TimerInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device to interrupt subsystem so that interrupts
	 * can occur.
	 */
	Status = TimerSetupIntrSystem(IntcInstancePtr,
					TimerInstancePtr, TimerIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
		 * Enable Auto reload mode.
		 */
		XScuTimer_EnableAutoReload(TimerInstancePtr);

		/*
		 * Load the timer counter register.
		 */
		XScuTimer_LoadTimer(TimerInstancePtr, CLOCKS_PER_UNIT);
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the device.
*
* @param	IntcInstancePtr is a pointer to the instance of XScuGic driver.
* @param	TimerInstancePtr is a pointer to the instance of XScuTimer
*		driver.
* @param	TimerIntrId is the Interrupt Id of the XScuTimer device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int TimerSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XScuTimer *TimerInstancePtr, u16 TimerIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Xil_ExceptionInit();



	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				IntcInstancePtr);
#endif

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, TimerIntrId,
				(Xil_ExceptionHandler)TimerIntrHandler,
				(void *)TimerInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(IntcInstancePtr, TimerIntrId);

	/*
	 * Enable the timer interrupts for timer mode.
	 */
	XScuTimer_EnableInterrupt(TimerInstancePtr);

#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the Timer interrupt of the
* Timer device. It is called on the expiration of the timer counter in
* interrupt context.
*
* @param	CallBackRef is a pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TimerIntrHandler(void *CallBackRef)
{
	doStartUnit = true;
	doServiceCores = true;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the device.
*
* @param	IntcInstancePtr is the pointer to the instance of XScuGic
*		driver.
* @param	TimerIntrId is the Interrupt Id for the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TimerDisableIntrSystem(XScuGic *IntcInstancePtr, u16 TimerIntrId)
{
	/*
	 * Disconnect and disable the interrupt for the Timer.
	 */
	XScuGic_Disconnect(IntcInstancePtr, TimerIntrId);
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
#ifdef TARGET_ZED
	ScuTimerIntrExample(&IntcInstance, &TimerInstance,
					TIMER_DEVICE_ID, TIMER_IRPT_INTR);
	XScuTimer_Start(TimerInstancePtr);
#else
	nanoseconds duration{ NS_PER_TICK * CLOCKS_PER_UNIT };
#endif

	while (currentUnit->unitNum < timeUnits.back()->unitNum)
	{
#ifdef DEBUG_TIMER
		std::cout << "Starting new TimeUnit." << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS
		std::this_thread::sleep_for(duration);
		doStartUnit = true;
		doServiceCores = true;
#endif
		currentUnit = findTimeUnit(currentUnit->unitNum + 1);
		doStartUnit = false;
		//TODO: use HW timer on target
	}
#ifdef TARGET_ZED
	TimerDisableIntrSystem(IntcInstancePtr, TimerIntrId);
#endif
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

	exit(0);
}
