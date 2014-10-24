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
	ofstream& logFile;
	vector <HANDLE> threadList;
	vector <Task> taskList;
public:
	ThreadPool(INT nThreadsCount, ofstream& logFile);
	VOID AddTask(Task task);
	BOOL SetPriority(INT nThreadIndex, INT nPriority);
	BOOL CheckForErrors() { return bError; }
};