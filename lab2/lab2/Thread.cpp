#include "Thread.h"

DWORD WINAPI ThreadEntryPoint(LPVOID lpParam)
{
	ThreadArgs *args = (ThreadArgs *)lpParam;
	return 0;
}