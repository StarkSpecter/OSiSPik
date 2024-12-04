#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>


const int ARRAY_SIZE = 100000; // Size of the array to be sorted
const int NUM_THREADS = 6;

// Thread data structure to pass multiple parameters
struct ThreadData {
    int threadID;
    std::vector<int> array;
};

// Function to generate a random array
std::vector<int> generateRandomArray(int size) {
    std::vector<int> arr(size);
    std::generate(arr.begin(), arr.end(), std::rand);
    return arr;
}

// Sorting function to be executed by each thread
DWORD WINAPI sortArray(LPVOID param) {
    ThreadData* data = static_cast<ThreadData*>(param);

    // Measure time taken for sorting
    auto start = std::chrono::high_resolution_clock::now();

    std::sort(data->array.begin(), data->array.end());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    // Get the thread's priority
    int priority = GetThreadPriority(GetCurrentThread()); // Get current thread's priority

    std::cout << "Thread " << data->threadID
              << " (Priority: " << priority << ") "
              << "sorted the array in " << duration.count() << " ms\n";

    return 0;
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr))); // Seed random number generator

    HANDLE threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    // Set specific priorities for each thread
    int priorities[NUM_THREADS] = {
        THREAD_PRIORITY_LOWEST,
        THREAD_PRIORITY_BELOW_NORMAL,
        THREAD_PRIORITY_NORMAL,
        THREAD_PRIORITY_ABOVE_NORMAL,
        THREAD_PRIORITY_HIGHEST,
        THREAD_PRIORITY_TIME_CRITICAL
    };
    for (int i = 0; i < NUM_THREADS; ++i) {
        threadData[i].threadID = i;
        std::vector<int> random_array = generateRandomArray(ARRAY_SIZE); // Generate a random array
        threadData[i].array = random_array;

        // Create thread
        threads[i] = CreateThread(nullptr, 0, sortArray, &threadData[i], 0, nullptr);

        // Set thread priority
        SetThreadPriority(threads[i], priorities[i]);
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    // Clean up thread handles
    for (int i = 0; i < NUM_THREADS; ++i) {
        CloseHandle(threads[i]);
    }

    return 0;
}
