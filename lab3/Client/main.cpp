#include <Windows.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define BUFFER_NAME "Global\\Lab3"
#define MUTEX_NAME "Global\\Lab3Mutex"

int main()
{
    HANDLE hMutex;
    LPSTR pBuffer;
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, BUFFER_NAME);

    if (!hMapFile)
    {
        printf("Could not open file mapping object (%d).\n", GetLastError());
        return 1;
    }

    pBuffer = (LPSTR)MapViewOfFile(hMapFile, FILE_MAP_WRITE, NULL, NULL, BUFFER_SIZE);

    if (!pBuffer)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);

    if (!hMutex)
    {
        printf("Could not open mutex object (%d).\n", GetLastError());
        UnmapViewOfFile(pBuffer);
        return 1;
    }

    while (strcmp(pBuffer, "exit\n"))
    {
        WaitForSingleObject(hMutex, INFINITE);
        printf(">");
        fflush(stdin);
        fgets(pBuffer, BUFFER_SIZE, stdin);
        ReleaseMutex(hMutex);
    }

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
    return 0;
}