#pragma once

#include <Windows.h>
#include <fstream>
#include <vector>
#include "Task.h"

using namespace std;

class ThreadPool
{
protected:
	INT nThreadsCount;
	BOOL bError = FALSE;
	BOOL bSuspend = FALSE;
	vector <HANDLE> threadList;
	vector <Task*> taskList;
public:
	CRITICAL_SECTION taskQueue;
	volatile long nFreeThreads;
	ofstream& logFile;

	ThreadPool(INT nThreadsCount, ofstream& logFile);
	~ThreadPool();
	VOID AddTask(Task *task);
	BOOL SetPriority(INT nThreadIndex, INT nPriority);
	BOOL CheckForErrors() { return bError; }
	BOOL WaitForTask() { return !bSuspend; }
	Task *GetNextTask();
	BOOL IsSomethingInQueue() { return taskList.size(); }
	VOID SuspendThreads(DWORD dwMilliseconds);
};