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
			Task *task = threadPool->GetNextTask();
			InterlockedDecrement(&threadPool->nFreeThreads);
			task->threadIndex = threadIndex;
			threadPool->logFile << "Thread with index " << threadIndex
				<< " have got new task." << endl;
			LeaveCriticalSection(&threadPool->taskQueue);
			task->SetState(STATE_RUNNING);
			Job *job = (Job*)task->GetFunction();
			task->returnValue = job(task->GetArgs());
			InterlockedIncrement(&threadPool->nFreeThreads);
			threadPool->logFile << "Thread with index " << threadIndex
				<< " have done the task. Return value: " << task->returnValue << endl;
			task->SetState(STATE_COMPLETED);
		}
		else
		{
			LeaveCriticalSection(&threadPool->taskQueue);
			Sleep(50);
		}
	}

	delete args;
	return 0;
}