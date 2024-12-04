#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

HANDLE resource_mutex;             
HANDLE read_count_semaphore;        
HANDLE first_reader_mutex;
HANDLE all_readers_done_event;

std::atomic<int> active_readers(0);

struct Stats {
    std::atomic<int> successful_locks;
    std::atomic<int> total_activity_time;
    std::atomic<int> total_wait_time;
};

Stats writer_stats;
Stats reader_stats;


long long get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void writer(int id, int wait_time, int sleep_time_after_cycle) {
    long long start_time, end_time, lock_start, lock_end;
    for (int j = 0; j < 5; j++) {
        start_time = get_current_time_ms();

        // Писатель должен захватить мьютекс ресурса
        lock_start = get_current_time_ms();
        if (WaitForSingleObject(resource_mutex, INFINITE) == WAIT_OBJECT_0) {
            writer_stats.successful_locks++;  
        }
        lock_end = get_current_time_ms();

        std::cout << "Writer " << id << " is writing..." << std::endl;
        Sleep(wait_time); 
        std::cout << "Writer " << id << " finished writing." << std::endl;

        ReleaseMutex(resource_mutex); 

        writer_stats.total_activity_time += (get_current_time_ms() - start_time);
        writer_stats.total_wait_time += (lock_end - lock_start);

        // Задержка после цикла работы
        std::cout << "Writer " << id << " sleeping for " << sleep_time_after_cycle << " ms after cycle." << std::endl;
        Sleep(sleep_time_after_cycle); 
    }
}

void reader(int id, int wait_time, int sleep_time_after_cycle, int num_readers) {
    long long start_time, end_time, lock_start, lock_end;
    int status;
    for (int j = 0; j < 5; j++) {
        start_time = get_current_time_ms();

        // Читатель ждет на одном из двух объектов: мьютексе первого читателя или семафоре остальных
        DWORD wait_result = WaitForMultipleObjects(2, 
            new HANDLE[2]{first_reader_mutex, read_count_semaphore}, 
            FALSE, INFINITE);  // Ожидаем на первом доступном объекте

        if (wait_result == WAIT_OBJECT_0) {
            status = 1;
            lock_start = get_current_time_ms();
            if (WaitForSingleObject(resource_mutex, INFINITE) == WAIT_OBJECT_0) {
                reader_stats.successful_locks++;
            }
            lock_end = get_current_time_ms();
            // Если это первый читатель, освобождаем семафор для остальных
            if (ReleaseSemaphore(read_count_semaphore, num_readers - 1, NULL) == 0) {
                std::cout << "Error releasing semaphore!" << std::endl;
            }
        }  
        else{ 
            status = 0;
            active_readers++;
            }  

        std::cout << "Reader " << id << " is reading..." << std::endl;
        Sleep(wait_time);  // Читатель тратит время на чтение
        std::cout << "Reader " << id << " finished reading." << std::endl;

        if (status == 1) {
            WaitForSingleObject(all_readers_done_event, INFINITE);
            ReleaseMutex(resource_mutex);
            ResetEvent(all_readers_done_event);
        }

        else {
            active_readers--;
            if (active_readers == 0) {
                SetEvent(all_readers_done_event);  
            }
        }
        // Статистика
        reader_stats.total_activity_time += (get_current_time_ms() - start_time);
        reader_stats.total_wait_time += (lock_end - lock_start);

        // Задержка после цикла работы
        std::cout << "Reader " << id << " sleeping for " << sleep_time_after_cycle << " ms after cycle." << std::endl;
        Sleep(sleep_time_after_cycle);
    }
}

int main() {
    int num_writers, num_readers, writer_wait_time, reader_wait_time;
    int writer_sleep_time_after_cycle, reader_sleep_time_after_cycle;

    std::cout << "Enter the number of writers: ";
    std::cin >> num_writers;
    std::cout << "Enter the number of readers: ";
    std::cin >> num_readers;
    std::cout << "Enter the writer's working time (in ms): ";
    std::cin >> writer_wait_time;
    std::cout << "Enter the reader's working time (in ms): ";
    std::cin >> reader_wait_time;
    std::cout << "Enter the writer's sleep time after each cycle (in ms): ";
    std::cin >> writer_sleep_time_after_cycle;
    std::cout << "Enter the reader's sleep time after each cycle (in ms): ";
    std::cin >> reader_sleep_time_after_cycle;

    resource_mutex = CreateMutex(NULL, FALSE, NULL);  
    first_reader_mutex = CreateMutex(NULL, FALSE, NULL);
    read_count_semaphore = CreateSemaphore(NULL, 0, num_readers - 1, NULL);  
    all_readers_done_event = CreateEvent(NULL, TRUE, FALSE, NULL);


    std::vector<std::thread> threads;
   
    for (int i = 0; i < num_writers; i++) {
        threads.push_back(std::thread(writer, i, writer_wait_time, writer_sleep_time_after_cycle));
    }

    
    for (int i = 0; i < num_readers; i++) {
        threads.push_back(std::thread(reader, i, reader_wait_time, reader_sleep_time_after_cycle, num_readers));
    }


    for (auto& t : threads) {
        t.join();
    }


    std::cout << "\nWriter Statistics:" << std::endl;
    std::cout << "Successful Locks: " << writer_stats.successful_locks << std::endl;
    std::cout << "Total Activity Time: " << writer_stats.total_activity_time << " ms" << std::endl;
    std::cout << "Total Wait Time: " << writer_stats.total_wait_time << " ms" << std::endl;

    std::cout << "\nReader Statistics:" << std::endl;
    std::cout << "Total Activity Time: " << reader_stats.total_activity_time << " ms" << std::endl;
    std::cout << "Total Wait Time: " << reader_stats.total_wait_time << " ms" << std::endl;


    CloseHandle(resource_mutex);
    CloseHandle(read_count_semaphore);
    CloseHandle(first_reader_mutex);
    CloseHandle(all_readers_done_event);

    return 0;
}
