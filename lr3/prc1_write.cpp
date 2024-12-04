#include <windows.h>
#include <iostream>

const int BUFFER_SIZE = 256; 

int main() {
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hFileMapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,   
        NULL,                   // Default security attributes
        PAGE_READWRITE,         // Read-write access
        0,                       // Maximum object size (high-order DWORD)
        BUFFER_SIZE,            // Size of the mapping
        L"Local\\SharedMemoryForTask"  // Name of the mapping object
    );

    LPCTSTR pMemory = (LPTSTR)MapViewOfFile(
        hFileMapping,           // Handle to the file mapping object
        FILE_MAP_ALL_ACCESS,    // Access type
        0,                       // High-order DWORD of the file offset
        0,                       // Low-order DWORD of the file offset
        BUFFER_SIZE             // Number of bytes to map
    );

    HANDLE hMutex = CreateMutexW(
        NULL,                   // Default security attributes
        FALSE,                  // Initial ownership (false means no initial ownership)
        L"Local\\MutexForTask"  
    );

    while (true) {
       
        WaitForSingleObject(hMutex, INFINITE);

       
        std::cout << "Input to be written to file mapping: ";
        std::string userInput;
        std::getline(std::cin, userInput);

        // If i enter "404", exit the loop
        if (userInput == "404") {
            CopyMemory((PVOID)pMemory, "404", 5 * sizeof(char));  
            break;
        }

        std::string existingData = (char*)pMemory;
        if (!existingData.empty()) {
            existingData += "\n";  
        }

        existingData += userInput;
        
        // Copy the updated string into the shared memory
        CopyMemory((PVOID)pMemory, existingData.c_str(), (existingData.size() + 1) * sizeof(char));

        std::cout << "Input has been written to file mapping.\n";

        ReleaseMutex(hMutex);
    }

    UnmapViewOfFile(pMemory);
    CloseHandle(hFileMapping);
    CloseHandle(hMutex);

    return 0;
}
