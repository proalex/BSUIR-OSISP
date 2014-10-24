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
	TaskState taskState;
	INT threadIndex = 0;

public:
	Task(LPVOID pFunction, LPVOID pArgs) : pFunction(pFunction), pArgs(pArgs)
	{
		taskState = STATE_QUEUE;
	}

	LPVOID GetFunction()
	{
		return pFunction;
	}

	LPVOID GetArgs()
	{
		return pArgs;
	}

	TaskState GetTaskState()
	{
		return taskState;
	}

	INT GetThreadIndex()
	{
		return threadIndex;
	}
};