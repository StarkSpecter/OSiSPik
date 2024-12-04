#include <windows.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>

constexpr int ARRAY_COUNT = 7;
constexpr int ARRAY_SIZE = 100000;
constexpr int MIN_VALUE = 10000;
constexpr int MAX_VALUE = 99999;

void generateBinaryFile(const char* filename) {
    std::vector<int> buffer(ARRAY_COUNT * ARRAY_SIZE);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(MIN_VALUE, MAX_VALUE);

    for (auto& num : buffer) {
        num = dis(gen);
    }

    HANDLE hFile = CreateFileA(
            filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create file." << std::endl;
        return;
    }

    DWORD bytesWritten;
    WriteFile(hFile, buffer.data(), buffer.size() * sizeof(int), &bytesWritten, nullptr);
    CloseHandle(hFile);
}

void asyncReadAndSort(const char* filename) {

    HANDLE hFile = CreateFileA(
            filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file." << std::endl;
        return;
    }

    std::vector<int> buffer(ARRAY_SIZE);
    OVERLAPPED overlapped = { 0 };
    std::vector<HANDLE> events(ARRAY_COUNT);

    for (int i = 0; i < ARRAY_COUNT; ++i) {
        events[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        overlapped.Offset = i * ARRAY_SIZE * sizeof(int);
        overlapped.hEvent = events[i];

        // Read the array asynchronously
        if (!ReadFile(hFile, buffer.data(), ARRAY_SIZE * sizeof(int), nullptr, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "Failed to read file." << std::endl;
                CloseHandle(hFile);
                return;
            }
        }
    }

    // Wait for all read operations to complete
    WaitForMultipleObjects(ARRAY_COUNT, events.data(), TRUE, INFINITE);

    for (int i = 0; i < ARRAY_COUNT; ++i) {
        overlapped.Offset = i * ARRAY_SIZE * sizeof(int);

        // Sort the array
       // std::sort(buffer.begin(), buffer.end());
        for (int j=0; j < buffer.size(); j++){
            buffer[j] = buffer[j] * 2;
            buffer[j] = buffer[j] / 2;
        }
        // Write the sorted array back to the file
        overlapped.hEvent = events[i];
        if (!WriteFile(hFile, buffer.data(), ARRAY_SIZE * sizeof(int), nullptr, &overlapped)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "Failed to write file." << std::endl;
                CloseHandle(hFile);
                return;
            }
        }
    }

    // Wait for all write operations to complete
    WaitForMultipleObjects(ARRAY_COUNT, events.data(), TRUE, INFINITE);

    for (HANDLE event : events) {
        CloseHandle(event);
    }

    CloseHandle(hFile);
}

void syncReadAndSort(const char* filename) {
    HANDLE hFile = CreateFileA(
            filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file." << std::endl;
        return;
    }

    std::vector<int> buffer(ARRAY_SIZE);
    DWORD bytesRead;

    for (int i = 0; i < ARRAY_COUNT; ++i) {
        LARGE_INTEGER offset;
        offset.QuadPart = i * ARRAY_SIZE * sizeof(int);
        SetFilePointerEx(hFile, offset, nullptr, FILE_BEGIN);

        // Read the array synchronously
        if (!ReadFile(hFile, buffer.data(), ARRAY_SIZE * sizeof(int), &bytesRead, nullptr)) {
            std::cerr << "Failed to read file." << std::endl;
            CloseHandle(hFile);
            return;
        }

        // Sort the array
        // std::sort(buffer.begin(), buffer.end());
        for (int j=0; j < buffer.size(); j++){
            buffer[j] = buffer[j] * 2;
            buffer[j] = buffer[j] / 2;
        }
        // Write the sorted array back to the file
        SetFilePointerEx(hFile, offset, nullptr, FILE_BEGIN);
        DWORD bytesWritten;
        if (!WriteFile(hFile, buffer.data(), ARRAY_SIZE * sizeof(int), &bytesWritten, nullptr)) {
            std::cerr << "Failed to write file." << std::endl;
            CloseHandle(hFile);
            return;
        }
    }

    CloseHandle(hFile);
}

int main() {
    const char* filename = "integer_arrays.bin";
    generateBinaryFile(filename);

    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Processing with asynchronous I/O:" << std::endl;
    asyncReadAndSort(filename);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> asyncDuration = end - start;
    std::cout << "Total time for asynchronous processing: " << asyncDuration.count()  << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::cout << "Processing with synchronous I/O:" << std::endl;
    syncReadAndSort(filename);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> syncDuration = end - start;
    std::cout << "Total time for synchronous processing: " << syncDuration.count() << " seconds" << std::endl;

    std::cout << "Processing completed." << std::endl;
    return 0;
}
