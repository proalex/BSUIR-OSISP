#pragma once

#include <Windows.h>
#include "ThreadPool.h"

struct ThreadArgs
{
public:
	INT threadIndex;
	ThreadPool *threadPool;

	ThreadArgs(INT threadIndex, ThreadPool *threadPool) : threadIndex(threadIndex), threadPool(threadPool) {}
};

DWORD WINAPI ThreadEntryPoint(LPVOID lpParam);