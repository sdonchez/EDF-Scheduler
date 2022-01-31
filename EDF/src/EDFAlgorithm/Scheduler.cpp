#include "Scheduler.h"

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

#ifdef USE_JSON
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

#else
/**
 * expects an XML Document containing the fields "unitsToExecute", "taskName",
 * "taskId", and "period".
*/
void processNewTask(pugi::xml_document* xmlTaskDoc)
{
#ifdef _DEBUG
	xml_string_writer xmlStr;
	xmlTaskDoc->print(xmlStr);
	std::cout << "New Task Received:" << xmlStr.result << std::endl;
#endif // _DEBUG
	pugi::xml_node xmlTask = xmlTaskDoc->child("Task");
	TimeUnit* deadline = findTimeUnit(atoi(xmlTask.attribute("deadline")
		.value()));
	Task* newTask = new Task(xmlTaskDoc, deadline, outstandingUnitsDue.data());
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
#endif

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

#ifdef TARGET_MS_WINDOWS
void timerManagerThread(LPCTSTR* oUDBuf, LPCTSTR* currUnitBuf)
{
#ifdef DEBUG_TIMER
	std::cout << "Timer Manager thread spawned." << std::endl;
#endif // DEBUG_TIMER

	nanoseconds duration{ NS_PER_TICK * CLOCKS_PER_UNIT };


	while (currentUnit->unitNum < timeUnits.back()->unitNum)
	{
#ifdef DEBUG_TIMER
		std::cout << "Starting new TimeUnit." << std::endl;
#endif
		std::this_thread::sleep_for(duration);
		doStartUnit = true;
		doServiceCores = true;

		currentUnit = findTimeUnit(currentUnit->unitNum + 1);
		doStartUnit = false;

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
	}
	return;
}
#else
void timerManagerThread(int* oUDBuf, int* currUnitBuf)
{
#ifdef DEBUG_TIMER
	std::cout << "Timer Manager thread spawned." << std::endl;
#endif // DEBUG_TIMER

	nanoseconds duration{ NS_PER_TICK * CLOCKS_PER_UNIT };


	while (currentUnit->unitNum < timeUnits.back()->unitNum)
	{
#ifdef DEBUG_TIMER
		std::cout << "Starting new TimeUnit." << std::endl;
#endif
		std::this_thread::sleep_for(duration);
		doStartUnit = true;
		doServiceCores = true;

		currentUnit = findTimeUnit(currentUnit->unitNum + 1);
		doStartUnit = false;

#ifdef DEBUG_IPC
		std::cout << "updating shared oUD Map" << std::endl;
#endif
		memcpy(oUDBuf, outstandingUnitsDue.data(),
			UNITS_TO_SIM);
#ifdef DEBUG_IPC
		std::cout << "updating shared currUnit Map - unit " << currentUnit->unitNum << std::endl;
#endif
		memcpy(currUnitBuf, &(currentUnit->unitNum),
			sizeof(unsigned int));
	}
	return;
}
#endif

void taskParserThread(
#ifdef TARGET_MS_WINDOWS
	HANDLE* hPipe,
	DWORD dwRead
#else
	int queueID
#endif
)
{
#ifdef DEBUG_TASKS
	std::cout << "Task Parser thread spawned" << std::endl;
#endif
	std::string taskStr = "";
#ifdef TARGET_MS_WINDOWS
	bool result;
	char pipeBuf[BUFSIZ * 10];
#endif
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
		msgrcv(queueID, &msgbuf, sizeof(msgbuf.json), 0, 0);
		taskStr = msgbuf.json;
#endif
#ifdef DEBUG_TASKS
		std::cout << "Input Received:";
		printf("%s;%s\n",taskStr.c_str(),msgbuf.json);
#endif
		size_t pos = 0;
		std::string token;

#ifdef USE_JSON
		while ((pos = taskStr.find("}")) != std::string::npos) {
			token = taskStr.substr(0, pos+1);
			taskLog << token << std::endl;
			processNewTask(nlohmann::json::parse(token));
			taskStr.erase(0, pos + 1);
		}
#else
		while ((pos = taskStr.find("/>")) != std::string::npos) {
			token = taskStr.substr(0, pos + 7);
			taskLog << token << std::endl;
			pugi::xml_document doc;
			pugi::xml_parse_result parseResult = doc.load_string(token.c_str());
#ifdef DEBUG_TASKS
			std::cout << "Task Parse Result: " << parseResult.description <<
				std::endl;
#endif
			processNewTask(&doc);
			taskStr.erase(0, pos + 7);
		}
#endif
	}
}

#ifndef TARGET_MS_WINDOWS
/*
** initsem() -- more-than-inspired by W. Richard Stevens' UNIX Network
** Programming 2nd edition, volume 2, lockvsem.c, page 295.
*/
int initsem(key_t key, int nsems)  /* key from ftok() */
{
	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

	if (semid >= 0) { /* we got it first */
		sb.sem_op = 1; sb.sem_flg = 0;
		arg.val = 1;

		std::cout << "start semop init" << std::endl;
		for (sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) {
			/* do a semop() to indicate 1 scheduler is available. */
			/* this sets the sem_otime field, as needed below. */
			if (semop(semid, &sb, 1) == -1) {
				int e = errno;
				semctl(semid, 0, IPC_RMID); /* clean up */
				errno = e;
				return -1; /* error, check errno */
			}
			std::cout << "Done semop init" << std::endl;
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

		if (!ready) {
			errno = ETIME;
			return -1;
		}
	}
	else {
		std::cout << "reterr 1" << std::endl;
		return semid; /* error, check errno */
	}
	std::cout << "good ret" << std::endl;
	return semid;
}
#endif

int main(int argc, char* argv[])
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
	int* oUDBuf;

	FILE *oudpathFile = fopen (OUDPATH, "ab+");
	fclose(oudpathFile);
	if ((oUDKey = ftok(OUDPATH, 'R')) == -1) {
		perror("ftok");
		exit(1);
	}

	/* connect to (and possibly create) the segment: */
	if ((oUDid = shmget(oUDKey, (sizeof(int) * UNITS_TO_SIM),
						0644 | IPC_CREAT)) == -1) {
		perror("shmget");
		exit(1);
	}

	/* attach to the segment to get a pointer to it: */
	oUDBuf = (int *)shmat(oUDid, (void*)0, 0);
	if (*oUDBuf == -1) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up shared CTU Map" << std::endl;
#endif
	key_t CTUKey;
	int CTUid;
	int* CTUBuf;

	FILE *ctupathFile = fopen (CTUPATH, "ab+");
	fclose(ctupathFile);

	if ((CTUKey = ftok(CTUPATH, 'R')) == -1) {
		perror("ftok");
		exit(1);
	}

	/* connect to (and possibly create) the segment: */
	if ((CTUid = shmget(CTUKey, (sizeof(int) * UNITS_TO_SIM),
		0644 | IPC_CREAT)) == -1) {
		perror("shmget");
		exit(1);
	}

	/* attach to the segment to get a pointer to it: */
	CTUBuf = (int *)shmat(CTUid, (void*)0, 0);
	if (*CTUBuf == -1) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up Semaphore to wait for generator." << std::endl;
#endif

	struct sembuf sb;
	int semid;

	key_t semkey;

	FILE *sempathFile = fopen (SEMPATH, "ab+");
	fclose(sempathFile);

	if ((semkey = ftok(SEMPATH, (int) SEMID)) == -1) {
		perror("ftok");
		exit(1);
	}

	/* grab the semaphore set created by seminit: */
	if ((semid = initsem(semkey, 1)) == -1) {
		perror("initsem");
		exit(1);
	}

//#ifdef _DEBUG
//	std::cout << "Allocating one scheduler to semaphore." << std::endl;
//#endif
//	sb.sem_num = 0;
//	sb.sem_op = 1;  /* set to provide resource */
//	sb.sem_flg = 0;
//
//	if (semop(semid, &sb, 1) == -1) {
//		int e = errno;
//		semctl(semid, 0, IPC_RMID); /* clean up */
//		errno = e;
//		std::cout << "err here:"  << errno<< std::endl;
//		return -1; /* error, check errno */
//	}

#ifdef _DEBUG
	std::cout << "Setting up SysV Message Queue for JSON" << std::endl;
#endif

	FILE *fifopathFile = fopen (FIFOPATH, "ab+");
	fclose(fifopathFile);
	key_t queueKey = ftok(FIFOPATH, (int)FIFOID);
	int queueID = msgget(queueKey, 0666 | IPC_CREAT);


#ifdef _DEBUG
	std::cout << "Waiting for Generator" << std::endl;
#endif
	sb.sem_num = 0;
	sb.sem_op = 0;  /* set to wait for resource consumption*/
	sb.sem_flg = 0;

	//wait for testbench to consume the "available scheduler"
	if (semop(semid, &sb, 1) == -1) {
		int e = errno;
		semctl(semid, 0, IPC_RMID); /* clean up */
		errno = e;
		return -1; /* error, check errno */
	}

#endif
	
#ifdef _DEBUG
	std::cout << "Spawning Threads" << std::endl;
#endif

#ifdef TARGET_MS_WINDOWS
	std::thread parserThread(taskParserThread, &hPipe, dwRead);
#else
	std::thread parserThread(taskParserThread, queueID);
#endif
	std::thread servicerThread(coreServicerThread);
	std::thread timerThread(timerManagerThread, oUDBuf, CTUBuf);

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
	std::cout << "Starting Cleanup" << std::endl;
	msgctl(queueID, IPC_RMID, NULL);
	std::cout << "C1" << std::endl;
	semctl(semid, 0, IPC_RMID);
	std::cout << "C2" << std::endl;
	int shmdt(void* oUDBuf);
	std::cout << "C3" << std::endl;
	int shmdt(void* currUnitBuf);
	std::cout << "C4" << std::endl;

	//actually delete shared mem since both sides are done now
	shmctl(oUDid, IPC_RMID, NULL);
	std::cout << "C5" << std::endl;
	shmctl(CTUid, IPC_RMID, NULL);
	std::cout << "C6" << std::endl;
#endif

	exit(0);
}


