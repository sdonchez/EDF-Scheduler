#pragma once
#include "Queue.h"
#include "Task.h"
#include "Core.h"
#include "TimeUnit.h"
#include "nlohmann\json.hpp"
#include <vector>
#include <thread>
#include <stdio.h>

#ifdef TARGET_MS_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#endif

#define NUM_CORES 5
#define CLOCKS_PER_UNIT 1000
//TODO: Determine CLOCKS_PER_UNIT value

#define UNITS_TO_SIM 100
//TODO: Replace with arg parsing