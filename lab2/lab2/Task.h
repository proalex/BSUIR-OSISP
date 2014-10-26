#pragma once

#include <Windows.h>

enum TaskState
{
    STATE_INVALID,
    STATE_QUEUE,
    STATE_RUNNING,
    STATE_COMPLETED
};

class Task
{
private:
    LPVOID pFunction;
    LPVOID pArgs;
    TaskState state;
public:
    INT threadIndex = 0;
    DWORD returnValue;

    Task(LPVOID pFunction, LPVOID pArgs);
    LPVOID GetFunction() { return pFunction; }
    LPVOID GetArgs() { return pArgs; }
    VOID SetState(TaskState state);
    BOOL WaitForRun();
    VOID WaitForComplete();
};