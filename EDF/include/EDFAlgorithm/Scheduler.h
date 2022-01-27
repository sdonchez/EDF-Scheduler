#pragma once

//Custom classes
#include "CommonDefs.h"
#include "Queue.h"
#include "Task.h"
#include "Core.h"
#include "TimeUnit.h"
#ifdef USEJSON
#include "nlohmann\json.hpp" //for input parsing
#else
#include "pugixml/pugixml.hpp" //for input parsing
#endif

#include <vector> //for processor and task arrays
#include <thread> //for concurrent operations
#include <stdio.h> //for input ingestion
#include <algorithm>
#include <iostream>
#include <fstream>

#ifdef TARGET_MS_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <winbase.h>
#include <conio.h>

#else
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#endif

#ifdef DEBUG_ALL
#define DEBUG_TASKS
#define DEBUG_TIMER
#define DEBUG_CORES
#define DEBUG_TIMEUNITS
#define DEBUG_IPC
#ifndef _DEBUG
#define _DEBUG
#endif
#endif

/**
 * @brief Creates the Core instances to be scheduled.
 * @param numCores - the number of Core instances to create. 
*/
void initCores(unsigned int numCores);

/**
 * @brief	Initializes the array of TimeUnit instances that the algorithm will 
 *			schedule over.
 * @param numUnits   - The number of TimeUnit instances to create.
 * @param numClocks  - The number of clock cycles per TimeUnit.
*/
void initTimeUnits(unsigned int numUnits, unsigned int numClocks);

/**
 * @brief Finds a TimeUnit instance in the timeUnits vector based on its UnitId.
 * @param unitId - the ID of the TimeUnit to find.
 * @return a pointer to the TimeUnit in question.
*/
TimeUnit* findTimeUnit(unsigned int unitId);


#ifdef USE_JSON
/**
 * @brief Converts a JSON task description to a Task object.
 * @param jsonTask - The JSON object (not a string) containing the task
 *					 information.
*/
void processNewTask(nlohmann::json jsonTask);
#else
/**
 * @brief Converts an XML task description to a Task object.
 * @param xmlTask - The XML object (not a string) containing the task
 *					 information.
*/
void processNewTask(pugi::xml_document xmlTask);
#endif

/**
 * @brief Swaps the current task on a core if necessary, then runs the task for
 *		  one TimeUnit.
 * @param core - the Core instance to service.
*/
void serviceCore(Core* core);

#ifdef TARGET_MS_WINDOWS
/**
 * @brief Callback function for the timer. Sets the flags for the other threads
 *		  and increments the current TimeUnit.
 * @param hwnd    - The window to attach to (if in a Windows GUI App).
 * @param uMsg	  - The system defined message.
 * @param timerId - The instance ID of the timer.
 * @param dwTime  - The tick count when the event ocurred.
*/
void CALLBACK timerCallback(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
#endif

/**
 * @brief Loop calls serviceCore on each core in succession at the start of each
 *		  TimeUnit.
*/
void coreServicerThread();

/**
 * @brief Restarts the TimeUnit timer when the callback sets the doStartUnit
 *		  flag, provided the last TimeUnit hasn't already executed.
*/
#ifdef TARGET_MS_WINDOWS
void timerManagerThread(LPCTSTR* oUDBuf, LPCTSTR* currUnitBuf);
#else
void timerManagerThread(void* oUDBuf, void* currUnitBuf);
#endif

/**
 * @brief Loop that waits for a line on standard input, then parses it to create
 *		  a Task object
*/
void taskParserThread(
#ifdef TARGET_MS_WINDOWS
	HANDLE* hPipe,
	DWORD dwRead
#else
	int queueID
#endif
);

int main(int argc, char* argv[]);
