#pragma comment(lib, "..\\Debug\\ThreadPoolLibrary.lib")

#include <iostream>
#include <vector>
#include "ThreadPool.h"

using namespace std;

DWORD WINAPI job(LPVOID lpParam)
{
    int result = *(int*)lpParam;
    Sleep(1000*result);
    return result;
}

int main()
{
    ofstream logFile;
    int threadsCount = 5;
    int *args = new int[threadsCount];

    logFile.open("ThreadPool.log");
    ThreadPool *threadPool = new ThreadPool(threadsCount, logFile);
    vector<Task*> taskList;

    if (threadPool->CheckForErrors())
    {
        delete threadPool;
        delete args;
        return 0;
    }
    
    for (auto i = 0; i < threadsCount; i++)
    {
        args[i] = i;
        taskList.push_back(new Task(job, &args[i]));
    }

    for (auto i = taskList.begin(); i != taskList.end(); i++)
    {
        threadPool->AddTask(*i);
    }

    for (auto i = taskList.begin(); i != taskList.end(); i++)
    {
        (*i)->WaitForRun();
    }

    for (auto i = taskList.begin(); i != taskList.end(); i++)
    {
        threadPool->SetPriority((*i)->threadIndex, THREAD_PRIORITY_NORMAL);
    }

    for (auto i = taskList.begin(); i != taskList.end(); i++)
    {
        (*i)->WaitForComplete();
    }

    for (auto i = taskList.begin(); i != taskList.end(); i++)
    {
        cout << "Thread index " << (*i)->threadIndex << " Result: " << (*i)->returnValue << endl;
    }

    threadPool->SuspendThreads(10000);
    delete threadPool;
    delete args;
    logFile.close();
    system("pause");
    return 0;
}