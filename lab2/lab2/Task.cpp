#include "Task.h"

Task::Task(LPVOID pFunction, LPVOID pArgs) : pFunction(pFunction), pArgs(pArgs)
{
	state = STATE_QUEUE;
}

BOOL Task::WaitForRun()
{
	if (state == STATE_INVALID)
		return FALSE;

	switch (state)
	{
	case STATE_INVALID:
		return FALSE;
	case STATE_COMPLETED:
	case STATE_RUNNING:
		return TRUE;
	}

	while (state != STATE_RUNNING && state != STATE_COMPLETED && state != STATE_INVALID)
		Sleep(10);

	return TRUE;
}

VOID Task::WaitForComplete()
{
	switch (state)
	{
	case STATE_INVALID:
	case STATE_COMPLETED:
		return;
	}

	while (state != STATE_COMPLETED && state != STATE_INVALID)
		Sleep(10);
}

VOID Task::SetState(TaskState state)
{
	this->state = state;
}

