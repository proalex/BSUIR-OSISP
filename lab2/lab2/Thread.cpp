#include "Thread.h"

typedef DWORD WINAPI Job(LPVOID lpParam);

DWORD WINAPI ThreadEntryPoint(LPVOID lpParam)
{
	ThreadArgs *args = (ThreadArgs *)lpParam;
	ThreadPool *threadPool = args->threadPool;
	INT threadIndex = args->threadIndex;

	while (threadPool->WaitForTask())
	{
		EnterCriticalSection(&threadPool->taskQueue);

		if (threadPool->IsSomethingInQueue())
		{
			Task task = threadPool->GetNextTask();
			LeaveCriticalSection(&threadPool->taskQueue);
			InterlockedDecrement(&threadPool->nFreeThreads);
			task.threadIndex = threadIndex;
			task.taskState = STATE_RUNNING;
			string logMessage = "Thread with index " + threadIndex;
			logMessage += " have got new task.";
			threadPool->WriteLog(logMessage);
			Job *job = (Job*)(task.GetFunction());
			task.returnValue = job(task.GetArgs());
			InterlockedIncrement(&threadPool->nFreeThreads);
			logMessage = "Thread with index " + threadIndex;
			logMessage += " have done the task. Return value: " + task.returnValue;
			threadPool->WriteLog(logMessage);
			task.taskState = STATE_COMPLETED;
		}
		else
		{
			LeaveCriticalSection(&threadPool->taskQueue);
			Sleep(50);
		}
	}

	string logMessage = "Thread with index " + threadIndex;
	logMessage += " was successfully closed.";
	threadPool->WriteLog(logMessage);
	return 0;
}