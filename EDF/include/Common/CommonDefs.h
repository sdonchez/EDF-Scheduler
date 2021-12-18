#pragma once
#define NUM_CORES 4
#define CLOCKS_PER_UNIT 1000
//TODO: Determine CLOCKS_PER_UNIT value

#define UNITS_TO_SIM 1000
//TODO: Replace with arg parsing

#ifdef TARGET_MS_WINDOWS
#define PIPENAME "\\\\.\\pipe\\EDFPipe"

#else

#define FIFONAME "EDFPipe"
#define FIFOPATH "./EDFPIPE"
#define FIFOID "A"

#define SEMPATH "./EDFSEM"
#define SEMID "B"

#define OUDPATH "./OUDSHM"
#define OUDID "C"

#define CTUPATH "./CTUSHM"
#define CTUID "D"

struct task_msgkbuf {
	long mtype; /* must be positive*/
	char* json;
};
union semun {
	int val;
	struct semid_ds* buf;
	ushort *array
};

#endif