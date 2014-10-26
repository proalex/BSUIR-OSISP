#include "ThreadPool.h"
#include "Thread.h"

ThreadPool::ThreadPool(INT nThreadsCount, ofstream& logFile)
    : nThreadsCount(nThreadsCount), logFile(logFile), nFreeThreads(nThreadsCount)
{
    InitializeCriticalSection(&taskQueue);

    for (auto i = 0; i < nThreadsCount; i++)
    {
        ThreadArgs *args = new ThreadArgs(i, this);
        HANDLE hThreadHandle = CreateThread(0, 0, ThreadEntryPoint, args, 0, 0);
        
        if (hThreadHandle == NULL)
        {
            logFile << "Failed to create thread. Index: " << i << " Error: %ul" << GetLastError() << "." << endl;
            bError = TRUE;
            break;
        }

        threadList.push_back(hThreadHandle);
        logFile << "Thread with index " << i << " was successfully created." << endl;
    }
}

ThreadPool::~ThreadPool()
{
    logFile << "Closing thread pool..." << endl;

    for (auto i = 0; i < nThreadsCount; i++)
    {
        if (WaitForSingleObject(threadList[i], 0) != WAIT_OBJECT_0)
        {
            logFile << "Terminating thread with index " << i << "..." << endl;
            TerminateThread(threadList[i], 0);
        }

        CloseHandle(threadList[i]);
    }

    DeleteCriticalSection(&taskQueue);
    logFile << "Thread pool closed." << endl;
}

VOID ThreadPool::AddTask(Task *task)
{
    if (!nFreeThreads)
    {
        logFile << "Failed to add new task: there are no free threads." << endl;
        task->SetState(STATE_INVALID);
        return;
    }

    logFile << "Adding new task in queue..." << endl;
    taskList.push_back(task);
}

BOOL ThreadPool::SetPriority(INT nThreadIndex, INT nPriority)
{
    BOOL bResult;

    logFile << "Setting priority " << nPriority << " for thread with index " << nThreadIndex << "." << endl;
    bResult = SetThreadPriority(threadList[nThreadIndex], nPriority);

    if (!bResult)
        logFile << "Failed to set priority " << nPriority << "for thread with index " << nThreadIndex << "." << endl;

    return bResult;
}

Task *ThreadPool::GetNextTask()
{
    Task *task = taskList.back();
    taskList.pop_back();
    return task;
}

VOID ThreadPool::SuspendThreads(DWORD dwMilliseconds)
{
    logFile << "Telling to all threads to suspend..." << endl;
    bSuspend = TRUE;

    for (auto i = 0; i < nThreadsCount; i++)
    {
        if (WaitForSingleObject(threadList[i], dwMilliseconds) == WAIT_OBJECT_0)
            logFile << "Thread with index " << i << " was successfully closed." << endl;
        else
            logFile << "Timeout waiting for thread with index " << i << "." << endl;
    }
}