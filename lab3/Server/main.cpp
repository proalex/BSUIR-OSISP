#include <Windows.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define BUFFER_NAME "Global\\Lab3"
#define MUTEX_NAME "Global\\Lab3Mutex"

int main()
{
    HANDLE hMutex;
    LPSTR pBuffer;
    CHAR oldData[BUFFER_SIZE];
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, BUFFER_NAME);

    if (!hMapFile)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        return 1;
    }

    pBuffer = (LPSTR)MapViewOfFile(hMapFile, FILE_MAP_WRITE, NULL, NULL, BUFFER_SIZE);

    if (!pBuffer)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    memset((LPVOID)pBuffer, 0, BUFFER_SIZE);
    hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);

    if (!hMutex)
    {
        printf("Could not create mutex object (%d).\n", GetLastError());
        UnmapViewOfFile(pBuffer);
        return 1;
    }

    printf("Waiting for messages...\n");

    while (strcmp(pBuffer, "exit\n"))
    {
        WaitForSingleObject(hMutex, INFINITE);

        if (strcmp(oldData, pBuffer))
            printf(pBuffer);

        strcpy_s(oldData, pBuffer);
        ReleaseMutex(hMutex);
    }

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
    return 0;
}