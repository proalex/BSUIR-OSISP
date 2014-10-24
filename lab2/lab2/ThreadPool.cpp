#include "ThreadPool.h"
#include "Thread.h"

ThreadPool::ThreadPool(INT nThreadsCount, ofstream& logFile)
	: nThreadsCount(nThreadsCount), logFile(logFile)
{
	for (auto i = 0; i < nThreadsCount; i++)
	{
		ThreadArgs args(i, this);
		HANDLE hThreadHandle = CreateThread(0, 0, ThreadEntryPoint, &args, 0, 0);
		
		if (hThreadHandle == NULL)
		{
			logFile << "Failed to create thread. Index: " << i << " Error: %ul" << GetLastError() << "." << endl;
			bError = TRUE;
			break;
		}

		threadList.push_back(hThreadHandle);
		logFile << "Thread with index " << i << "was successfully created." << endl;
	}
}

VOID ThreadPool::AddTask(Task task)
{
	logFile << "Adding new task in queue..." << endl;
	taskList.push_back(task);
}

BOOL ThreadPool::SetPriority(INT nThreadIndex, INT nPriority)
{
	BOOL bResult;

	logFile << "Setting priority " << nPriority << "for thread with index " << nThreadIndex << "." << endl;
	bResult = SetThreadPriority(threadList[nThreadIndex], nPriority);

	if (!bResult)
		logFile << "Failed to set priority " << nPriority << "for thread with index " << nThreadIndex << "." << endl;

	return bResult;
}