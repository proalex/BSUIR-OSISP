#pragma once
#include <Windows.h>
#include <fstream>
#include <vector>
#include "Task.h"

using namespace std;

class ThreadPool
{
protected:
	INT threadsCount;
	ifstream logFile;
	vector <HANDLE> threadList;
	vector <Task> taskList;
public:
	ThreadPool(INT threadsCount, ifstream logFile);
	INT AddTask(Task task);
	VOID SetPriority(INT threadIndex, INT priority);
};