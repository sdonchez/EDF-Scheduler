#include "GeneratedTask.h"
#include "CommonDefs.h"
#include "TimeUnit.h"
#include <random>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

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

#define DEFAULT_UTILIZATION 95 //TODO: Replace with arg parsing

int volatile* oUD;
unsigned int volatile* currUnit;

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
		sb.sem_op =1;
		sb.sem_flg = 0;
		arg.val = 1;

		for (sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) {
			/* do a semop() to "free" the semaphores. */
			/* this sets the sem_otime field, as needed below. */
			if (semop(semid, &sb, 1) == -1) {
				int e = errno;
				semctl(semid, 0, IPC_RMID); /* clean up */
				errno = e;
				return -1; /* error, check errno */
			}
		}

		sb.sem_op =-1;
		for (sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) {
			/* do a semop() to "alocate" the semaphores we just "freed". */
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

int main(int argc, char* argv[])
{
	int unitsOpt, utilization;
	if ((argc == 2) && (unitsOpt = ((int) strtol(argv[1], nullptr, 10))))
	{
		if ((unitsOpt < 1) || (unitsOpt > 100))
		{
			std::cout<< "invalid utilization percentage"
					"passed. Using default value" << std::endl;
			utilization = DEFAULT_UTILIZATION;
		}
		else
		{
			utilization = unitsOpt;
		}
	}
	else
	{
		utilization = DEFAULT_UTILIZATION;
	}
	int maxUnits = (int)((UNITS_TO_SIM * NUM_CORES) * (utilization / 100.0));
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
	LPCTSTR CTUBuf;

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

	CTUBuf = (LPTSTR)MapViewOfFile(currUnitMap,   // handle to map object
		PAGE_READONLY, // read/write permission
		0,
		0,
		sizeof(unsigned int)
	);

	if (CTUBuf == NULL)
	{
		printf("Could not map view of file (%d).\n",
			GetLastError());

		CloseHandle(currUnitMap);

		return 1;
	}

	oUD = (int*) oUDBuf;
	currUnit = (unsigned int*)CTUBuf;

#else //if ARM

#ifdef _DEBUG
	std::cout << "Setting up SysV Message Queue for JSON" << std::endl;
#endif


	FILE *fifopathFile = fopen (FIFOPATH, "ab+");
	fclose(fifopathFile);
	struct sembuf sb;
	int semid;
	key_t queueKey = ftok(FIFOPATH, FIFOID);
	int queueID = msgget(queueKey, 0666 | IPC_CREAT);


#ifdef _DEBUG
	std::cout << "Setting up Semaphore to wait for scheduler" << std::endl;
#endif

//wait for connection
	key_t semkey;

	sb.sem_num = 0;
	sb.sem_op = -1;  /* set to allocate resource */
	sb.sem_flg = 0;

	FILE *sempathFile = fopen (SEMPATH, "ab+");
	fclose(sempathFile);

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
	std::cout << "Waiting for Scheduler to connect" << std::endl;
#endif
	//attempt to request one scheduler
	if (semop(semid, &sb, 1) == -1) {
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
	oUDBuf = (int *)shmat(oUDid, (void*)0, SHM_RDONLY);
	if (*oUDBuf == -1) {
		perror("shmat");
		exit(1);
	}

#ifdef _DEBUG
	std::cout << "Setting up shared CTU Map" << std::endl;
#endif
	key_t CTUKey;
	int CTUid;
	unsigned int* CTUBuf;

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
	CTUBuf = (unsigned int *)shmat(CTUid, (void*)0, 0);
	if (CTUBuf == (unsigned int*)(-1)) {
		perror("shmat");
		exit(1);
	}
currUnit = CTUBuf;
oUD = oUDBuf;

#endif

#ifdef _DEBUG
	std::cout << "Starting Task Generation for " << maxUnits <<" units" << 
		std::endl;
#endif
	bool done = false;
	std::srand(std::time(0));
	while(!done)
	{
#ifdef DEBUG_UNITS
		std::cout << "Current Total Units: " << totalUnits << " Target: "
			<< maxUnits << " Current Unit Number: " << *CTUBuf<< std::endl;
#endif
		numTasks = (int)round(5 * numTasksDistribution(generator));
		for (unsigned int taskNo = 0; taskNo < numTasks; taskNo++)
		{
			std::cout << "Generating Task #" << taskNo << std::endl;
			GeneratedTask task(oUD, currUnit, utilization);
			totalUnits += task.unitsToExecute;
			if (totalUnits > maxUnits)
			{
				std::cout << "Not sending Task - max units exceeded" << 
					std::endl;
				done = true;
				break;
			}
#ifdef USE_JSON
			std::string taskStr = task.toJson();
#else
			std::string taskStr = task.toXMLSerialized();
#endif

#ifdef TARGET_MS_WINDOWS
			WriteFile(hPipe, taskStr.c_str(), (DWORD) taskStr.length(), 
				&dwWritten, NULL);

#else
			struct task_msgbuf msg;
			msg.mtype = 1;
			strncpy (msg.json, taskStr.c_str(), 1023);
			std::cout << msg.json << "CTU:" << *currUnit << std::endl;
			msgsnd(queueID, &msg, sizeof(msg.json),0);

#endif
		}
		
		unitsToSleep = (int)round(10 * numTasksDistribution(generator));
#ifdef DEBUG_UNITS
		std::cout << "Sleeping for " << unitsToSleep << " units." << std::endl;
#endif
#ifdef TARGET_MS_WINDOWS
		//std::this_thread::sleep_for(std::chrono::nanoseconds(
		//	unitTimeNS * unitsToSleep));
		Sleep(unitsToSleep);
#else
		usleep(ceil(((double)(unitTimeNS * unitsToSleep)) / 1000));
#endif
	}

#ifdef TARGET_MS_WINDOWS
	DisconnectNamedPipe(hPipe);
	UnmapViewOfFile(oUDBuf);
	UnmapViewOfFile(CTUBuf);
	CloseHandle(oUDMap);
	CloseHandle(currUnitMap);

#else
//	msgctl(queueID, IPC_RMID, NULL);
//	semctl(semid, 0, IPC_RMID);
//	int shmdt(void* oUDBuf);
//	int shmdt(void* CTUBuf);
	//Don't actually delete shared mem since the scheduler generally finishes
	//after the testbench
#endif
}
