#pragma once
#include <Windows.h>
#include "ThreadPool.h"

struct ThreadArgs
{
public:
	INT threadIndex;
	ThreadPool *pool;

	ThreadArgs(INT threadIndex, ThreadPool *pool) : threadIndex(threadIndex), pool(pool) {}
};

DWORD WINAPI ThreadEntryPoint(LPVOID lpParam);