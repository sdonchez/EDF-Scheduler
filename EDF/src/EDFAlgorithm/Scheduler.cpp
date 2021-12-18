#include "scheduler.h"

std::vector<Core*> cores;
std::vector<TimeUnit*> timeUnits;
TimeUnit* currentUnit;
TaskQueue tasks;
bool doServiceCores = true;
bool doStartUnit = true;
std::vector<int> outstandingUnitsDue;

std::ofstream taskLog;
std::ofstream coreLog;


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
#if defined DEBUG_TIMER || defined DEBUG_TIMEUNITS
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
		std::cout << "Core #" << core->coreId << ": ";
		if (core->currentTask)
			std::cout << core->currentTask->deadline->unitNum << std::endl;
		else
			std::cout << "No Task" << std::endl;
		});
#endif // DEBUG_CORES

	Core* maxCore = *std::max_element(cores.begin(), cores.end(), 
		[](const Core* a, const Core* b)
		{
			if (a->currentTask != nullptr && b->currentTask != nullptr)
				return a->currentTask->deadline < b->currentTask->deadline;
			else if (!a->currentTask)
				return false;
			else
				return true;
		});
	if (core->coreId != maxCore->coreId)
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
		coreLog << core->currentTask->taskId << '(' << 
			core->currentTask->unitsExecuted << '/' <<
			core->currentTask->unitsToExecute << ',' <<
			core->currentTask->deadline->unitNum << ')' << '\t';
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
			coreLog << currentUnit->unitNum << ':' << '\t';
			std::for_each(std::begin(cores), std::end(cores), serviceCore);
			coreLog << std::endl;
			doServiceCores = false;
		}
	}
}

void timerManagerThread(LPCTSTR* oUDBuf, LPCTSTR* currUnitBuf)
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
#ifdef TARGET_MS_WINDOWS
#ifdef DEBUG_IPC
		std::cout << "updating shared oUD Map" << std::endl;
#endif
		CopyMemory((PVOID)*oUDBuf, outstandingUnitsDue.data(),
			UNITS_TO_SIM * sizeof(int));
#ifdef DEBUG_IPC
		std::cout << "updating shared currUnit Map" << std::endl;
#endif
		CopyMemory((PVOID)*currUnitBuf, &(currentUnit->unitNum),
			sizeof(unsigned int));

#else
#ifdef DEBUG_IPC
		std::cout << "updating shared oUD Map" << std::endl;
#endif
		memcpy(oUDBuf, outstandingUnitsDue.data(),
			UNITS_TO_SIM * sizeof(int));
#ifdef DEBUG_IPC
		std::cout << "updating shared currUnit Map" << std::endl;
#endif
		memcpy(currUnitBuf, &(currentUnit->unitNum),
			sizeof(unsigned int));

#endif
		//TODO: use HW timer on target
	}
#ifdef TARGET_ZED
	TimerDisableIntrSystem(IntcInstancePtr, TimerIntrId);
#endif
	return;
}

void taskParserThread(
#ifdef TARGET_MS_WINDOWS
	HANDLE* hPipe,
	DWORD dwRead
#endif
)
{
#ifdef DEBUG_TASKS
	std::cout << "Task Parser thread spawned" << std::endl;
#endif
	std::string taskStr = "";
	BOOL result;
	char pipeBuf[BUFSIZ * 10];
	while (true)
	{
#ifdef DEBUG_TASKS
		std::cout << "Waiting for input" << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS

		result = ReadFile(
			*hPipe,
			pipeBuf,
			1024 * sizeof(char),
			&dwRead,
			NULL
		);
		pipeBuf[dwRead] = '\0';
		taskStr = pipeBuf;
#else
		task_msgbuf msgbuf;
		msgrecv(queueID, &msgbuf, sizeof(struct task_msgbuf), 0, 0);
		taskStr = msgbuf.json
#endif
#ifdef DEBUG_TASKS
		std::cout << "Input Received" << std::endl;
#endif
		size_t pos = 0;
		std::string token;
		while ((pos = taskStr.find("}")) != std::string::npos) {
			token = taskStr.substr(0, pos+1);
			taskLog << token << std::endl;
			processNewTask(nlohmann::json::parse(token));
			taskStr.erase(0, pos + 1);
		}
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
	std::cout << "Setting up Logs" << std::endl;
#endif

	taskLog.open("log/tasks.log");
	coreLog.open("log/cores.tsv");


	if (!taskLog.is_open() || !coreLog.is_open())
	{
		std::cout << "Error opening log files!" << std::endl;
		taskLog.close();
		coreLog.close();
		return -1;
	}

	coreLog << "TimeUnit \t";
	std::for_each(std::begin(cores), std::end(cores), [](Core* core) {
		coreLog << "Core " << core->coreId << '\t';
		});

#ifdef TARGET_MS_WINDOWS

	HANDLE oUDMap;
	LPCTSTR oUDBuf;
	HANDLE currUnitMap;
	LPCTSTR currUnitBuf;

#ifdef _DEBUG
	std::cout << "Setting up shared oUD Map" << std::endl;
#endif
	oUDMap = CreateFileMappingA(
		INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
		(sizeof(int) * UNITS_TO_SIM), "oUDMap"
	);
	if (oUDMap == NULL)
	{
		printf("Could not create file mapping object (%d).\n",
			GetLastError());
		return 1;
	}

#ifdef _DEBUG
	std::cout << "Setting up shared oUD View" << std::endl;
#endif
	oUDBuf = (LPTSTR)MapViewOfFile(oUDMap,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		(sizeof(int) * UNITS_TO_SIM)
	);

	if (oUDBuf == NULL)
	{
		printf("Could not map view of file (%d).\n",
			GetLastError());

		CloseHandle(oUDMap);

		return 1;
	}

#ifdef _DEBUG
	std::cout << "Setting up shared currUnit Map" << std::endl;
#endif

	currUnitMap = CreateFileMappingA(
		INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
		sizeof(unsigned int), "currUnitMap"
	);
	if (currUnitMap == NULL)
	{
		printf("Could not create file mapping object (%d).\n",
			GetLastError());
		return 1;
	}

#ifdef _DEBUG
	std::cout << "Setting up shared currUnit View" << std::endl;
#endif

	currUnitBuf = (LPTSTR)MapViewOfFile(currUnitMap,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		sizeof(unsigned int)
	);

	if (currUnitBuf == NULL)
	{
		printf("Could not map view of file (%d).\n",
			GetLastError());

		CloseHandle(currUnitMap);

		return 1;
	}

#ifdef _DEBUG
	std::cout << "Waiting for Generator" << std::endl;
#endif

	while (!WaitNamedPipe(TEXT(PIPENAME), NMPWAIT_WAIT_FOREVER));

#ifdef _DEBUG
	std::cout << "Connecting to Generator" << std::endl;
#endif

	HANDLE hPipe;
	DWORD dwRead;

	hPipe = CreateFile(
		TEXT(PIPENAME),
		PIPE_ACCESS_INBOUND,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		printf("Could not open named pipe (%d)\n",
			GetLastError());
		return 1;
	}

#else //if ARM

#ifdef _DEBUG
	std::cout << "Setting up shared oUD Map" << std::endl;
#endif
 
	key_t oUDKey;
	int oUDid;
	char* oUDBuf;

	if ((key = ftok("shmdemo.c", 'R')) == -1) {
		perror("ftok");
		exit(1);
	}

	/* connect to (and possibly create) the segment: */
	if ((shmid = shmget(oUDKey, (sizeof(int) * UNITS_TO_SIM), 
						0644 | IPC_CREAT)) == -1) {
		perror("shmget");
		exit(1);
	}

	/* attach to the segment to get a pointer to it: */
	oUDBuf = shmat(oUDid, (void*)0, 0);
	if (oUDBuf == (char*)(-1)) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up shared CTU Map" << std::endl;
#endif
	key_t CTUKey;
	int CTUid;
	char* currUnitBufBuf;

	if ((key = ftok("shmdemo.c", 'R')) == -1) {
		perror("ftok");
		exit(1);
	}

	/* connect to (and possibly create) the segment: */
	if ((shmid = shmget(CTUKey, (sizeof(int) * UNITS_TO_SIM),
		0644 | IPC_CREAT)) == -1) {
		perror("shmget");
		exit(1);
	}

	/* attach to the segment to get a pointer to it: */
	currUnitBuf = shmat(CTUid, (void*)0, 0);
	if (CTUBuf == (char*)(-1)) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up Semaphore to wait for generator".
#endif

		union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	key_t semkey;
	int semid;
	struct sembuf sb;

	sb.sem_num = 0;
	sb.sem_op = -1;  /* set to allocate resource */
	sb.sem_flg = SEM_UNDO;

	if ((semkey = ftok(SEMPATH, SEMID)) == -1) {
		perror("ftok");
		exit(1);
	}

	/* grab the semaphore set created by seminit.c: */
	if ((semid = initsem(semkey, 1)) == -1) {
		perror("initsem");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up SysV Message Queue for JSON" << std::endl;
#endif

	key_t queueKey = ftok(FIFOPATH, FIFOID);
	int queueID = msgget(queueKey, 0666 | IPC_CREAT);


#ifdef _DEBUG
	std::cout << "Waiting for Generator" << std::endl;
#endif

	//wait for testbench to consume the "available scheduler"
	if (semop(semid, &sb, 0) == -1) {
		int e = errno;
		semctl(semid, 0, IPC_RMID); /* clean up */
		errno = e;
		return -1; /* error, check errno */
	}

#endif
	
#ifdef _DEBUG
	std::cout << "Spawning Threads" << std::endl;
#endif
	std::thread parserThread(taskParserThread, &hPipe, dwRead);
	std::thread servicerThread(coreServicerThread);
	std::thread timerThread(timerManagerThread, &oUDBuf, &currUnitBuf);

#ifdef _DEBUG
	std::cout << "Waiting for timer to end" << std::endl;
#endif // _DEBUG

	timerThread.join();

#ifdef _DEBUG
	std::cout << "Timer ended, shutting down." << std::endl;
#endif // _DEBUG

	taskLog.close();
	coreLog.close();

#ifdef TARGET_MS_WINDOWS
	CloseHandle(hPipe);
	UnmapViewOfFile(oUDBuf);
	UnmapViewOfFile(currUnitBuf);
	CloseHandle(oUDMap);
	CloseHandle(currUnitMap);

#else
	msgctl(queueID, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	int shmdt(void* oUDBuf);
	int shmdt(void* currUnitBuf);

	//actually delete shared mem since both sides are done now
	shmctl(oUDid, IPC_RMID, NULL);
	shmctl(CTUid, IPC_RMID, NULL);
#endif

	exit(0);
}

#ifndef TARGET_MS_WINDOWS
/*
** initsem() -- more-than-inspired by W. Richard Stevens' UNIX Network
** Programming 2nd edition, volume 2, lockvsem.c, page 295.
*/
int initsem(key_t key, int nsems)  /* key from ftok() */
{
	int i;
	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

	if (semid >= 0) { /* we got it first */
		sb.sem_op = 1; sb.sem_flg = 0;
		arg.val = 1;

		printf("press return\n"); getchar();

		for (sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) {
			/* do a semop() to indicate 1 scheduler is available. */
			/* this sets the sem_otime field, as needed below. */
			if (semop(semid, &sb, 1) == -1) {
				int e = errno;
				semctl(semid, 0, IPC_RMID); /* clean up */
				errno = e;
				return -1; /* error, check errno */
			}
		}

	}
	else if (errno == EEXIST) { /* someone else got it first */
		int ready = 0;

		semid = semget(key, nsems, 0); /* get the id */
		if (semid < 0) return semid; /* error, check errno */

		/* wait for other process to initialize the semaphore: */
		arg.buf = &buf;
		while (!ready) {
			semctl(semid, nsems - 1, IPC_STAT, arg);
			if (arg.buf->sem_otime != 0) {
				ready = 1;
			}
			else {
				sleep(1);
			}
		}
	}
	else {
		return semid; /* error, check errno */
	}

	return semid;
}
#endif
