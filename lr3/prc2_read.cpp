#include <windows.h>
#include <iostream>

const int BUFFER_SIZE = 256;  

int main() {
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hFileMapping = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,     // Access type
        FALSE,                   // No inheritance
        L"Local\\SharedMemoryForTask"  
    );

    LPCTSTR pMemory = (LPTSTR)MapViewOfFile(
        hFileMapping,           // Handle to the file mapping object
        FILE_MAP_ALL_ACCESS,    // Access type
        0,                       // High-order DWORD of the file offset
        0,                       // Low-order DWORD of the file offset
        BUFFER_SIZE             // Number of bytes to map
    );

    HANDLE hMutex = OpenMutexW(
        MUTEX_ALL_ACCESS,       // Access type
        FALSE,                  // No inheritance
        L"Local\\MutexForTask"  
    );

    for (int i = 0; i < 5; i++) {
        WaitForSingleObject(hMutex, INFINITE);

        std::cout << "Content of file mapping:" << std::endl;
        std::cout << pMemory << std::endl;

        ReleaseMutex(hMutex);
        Sleep(100);

    }

    UnmapViewOfFile(pMemory);
    CloseHandle(hFileMapping);
    CloseHandle(hMutex);

    return 0;
}
