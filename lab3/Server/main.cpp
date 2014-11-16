#include <Windows.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define BUFFER_NAME "Global\\lab3"
#define MUTEX_NAME "Global\\lab3"

int main()
{
    HANDLE hMutex;
    LPCSTR pBuffer;
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, NULL, BUFFER_SIZE, BUFFER_NAME);

    if (!hMapFile)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        return 1;
    }

    pBuffer = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_WRITE, NULL, NULL, BUFFER_SIZE);

    if (!pBuffer)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    CloseHandle(hMapFile);
    hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME);

    if (!hMutex)
    {
        printf("Could not create mutex object (%d).\n", GetLastError());
        UnmapViewOfFile(pBuffer);
        return 1;
    }

    printf("Waiting for messages...\n");

    while (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0)
    {
        if (!strcmp(pBuffer, "exit"))
            break;

        printf(pBuffer);
    }

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMutex);
    return 0;
}