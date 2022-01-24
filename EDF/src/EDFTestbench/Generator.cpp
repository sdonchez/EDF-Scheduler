#include "GeneratedTask.h"
#include "CommonDefs.h"
#include "TimeUnit.h"
#include <random>
#include <string>
#include <iostream>
#ifdef TARGET_MS_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <winbase.h>
#include <conio.h>
#include <chrono>
#include <thread>

#else
#include <unistd.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cmath>
#endif

#ifdef DEBUG_ALL
#define DEBUG_UNITS
#define DEBUG_TASKS
#ifndef _DEBUG
#define _DEBUG
#endif
#endif

#define TARGET_UTILIZATION 100 //TODO: Replace with arg parsing

int volatile* oUD;
unsigned int volatile* currUnit;

int main(unsigned int argc, char* argv[])
{
	int maxUnits = (int)((UNITS_TO_SIM * NUM_CORES) * (TARGET_UTILIZATION / 100.0));
	int totalUnits = 0;
	std::default_random_engine generator;
	std::exponential_distribution <double> numTasksDistribution(3.5);
	unsigned int numTasks = 0;
	unsigned int unitsToSleep = 0;
	unsigned int unitTimeNS = NS_PER_TICK * CLOCKS_PER_UNIT;

#ifdef TARGET_MS_WINDOWS

#ifdef _DEBUG
	std::cout << "Setting up named pipe" << std::endl;
#endif

	HANDLE hPipe;
	DWORD dwWritten;

	hPipe = CreateNamedPipe(
		TEXT(PIPENAME),
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_MESSAGE,
		1,
		1024 * sizeof(char),
		1024 * sizeof(char),
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL
		);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		printf("Could not create named pipe (%d)\n",
			GetLastError());
		return 1;
	}

#ifdef _DEBUG
	std::cout << "Waiting for Scheduler to connect" << std::endl;
#endif
	//wait for connection
	while (ConnectNamedPipe(hPipe, NULL) == FALSE);
#ifdef _DEBUG
	std::cout << "Scheduler has connected!" << std::endl;
#endif

	HANDLE oUDMap;
	LPCTSTR oUDBuf;
	HANDLE currUnitMap;
	LPCTSTR currUnitBuf;

#ifdef _DEBUG
	std::cout << "Setting up shared oUD Map" << std::endl;
#endif

	oUDMap = OpenFileMappingA(
		PAGE_READONLY,
		FALSE,
		"oUDMap"
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
		PAGE_READONLY, // read/write permission
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

	currUnitMap = OpenFileMappingA(
		PAGE_READONLY,
		FALSE,
		"currUnitMap"
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
		PAGE_READONLY, // read/write permission
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

	oUD = (int*) oUDBuf;
	currUnit = (unsigned int*)currUnitBuf;

#else //if ARM

#ifdef _DEBUG
std::cout << "Setting up SysV Message Queue for JSON" << std::endl;
#endif

	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;
	key_t queueKey = ftok(FIFOPATH, FIFOID);
	int queueID = msgget(queueKey, 0666 | IPC_CREAT);


#ifdef _DEBUG
	std::cout << "Setting up Semaphore to wait for scheduler".
#endif

#ifdef _DEBUG
std::cout << "Waiting for Scheduler to connect" << std::endl;
#endif

//wait for connection
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

	//attempt to request one scheduler
	if (semop(semid, &sb, -1) == -1) {
		int e = errno;
		semctl(semid, 0, IPC_RMID); /* clean up */
		errno = e;
		return -1; /* error, check errno */
	}

#ifdef _DEBUG
	std::cout << "Scheduler has connected!" << std::endl;
#endif


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
	oUDBuf = shmat(oUDid, (void*)0, SHM_RDOMLY);
	if (oUDBuf == (char*)(-1)) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up shared CTU Map" << std::endl;
#endif
	key_t CTUKey;
	int CTUid;
	unsigned int* currUnitBuf;

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
currUnit = *currUnitBuf;

#endif

#ifdef _DEBUG
	std::cout << "Starting Task Generation for " << maxUnits <<" units" << 
		std::endl;
#endif
	while(totalUnits < maxUnits)
	{
#ifdef DEBUG_UNITS
		std::cout << "Current Total Units: " << totalUnits << " Target: "
			<< maxUnits << "Current Unit Number: " << *currUnit << std::endl;
#endif
		numTasks = (int)round(5 * numTasksDistribution(generator));
		for (unsigned int taskNo = 0; taskNo < numTasks; taskNo++)
		{
			GeneratedTask task(oUD, currUnit, TARGET_UTILIZATION);
			totalUnits += task.unitsToExecute;
#ifdef USE_JSON
			std::string taskStr = task.toJson();
#else
			std::string taskStr = task.toXMLSerialized();
#endif
			std::cout << taskStr << std::endl;

#ifdef TARGET_MS_WINDOWS
			WriteFile(hPipe, taskStr.c_str(), (DWORD) taskStr.length(), 
				&dwWritten, NULL);

#else

			struct task_msgbuf msg = { 1, taskStr.c_str() };
			msgsnd(queueId, &msg, sizeof(*(msg.json)));

#endif
		}
		
		unitsToSleep = (int)round(500 * numTasksDistribution(generator));
#ifdef DEBUG_UNITS
		std::cout << "Sleeping for " << unitsToSleep << " units." << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS
		//std::this_thread::sleep_for(std::chrono::nanoseconds(
		//	unitTimeNS * unitsToSleep));
		Sleep(unitsToSleep);
#else
		usleep(ceil(((double)(unitTimeNs * unitsToSleep)) / 1000));
#endif
	}


#ifdef TARGET_MS_WINDOWS
	DisconnectNamedPipe(hPipe);
	UnmapViewOfFile(oUDBuf);
	UnmapViewOfFile(currUnitBuf);
	CloseHandle(oUDMap);
	CloseHandle(currUnitMap);

#else
	msgctl(queueID, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	int shmdt(void* oUDBuf);
	int shmdt(void* currUnitBuf);
	//Don't actually delete shared mem since the scheduler generally finishes
	//after the testbench
#endif
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
			/* do a semop() to "free" the semaphores. */
			/* this sets the sem_otime field, as needed below. */
			if (semop(semid, &sb, 0) == -1) {
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
		while( !ready) {
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
