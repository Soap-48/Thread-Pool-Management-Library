#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <atomic>
#include <string>
#include <fcntl.h>   
#include <unistd.h>  
#include <cstdio>    // For printf
#include "thread_pool.h"
#include "IO_Pool.h"

const bool IO_READ = false;
const bool IO_WRITE = true;

struct PrimeTaskData {
    int task_id;
    long long start_num;
    long long end_num;
    long long primes_found; // We will store the result here
};

bool is_prime(long long n) {
    if (n <= 1) return false;
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// The task function executed by the worker threads
void calculate_primes(void* arg) {
    PrimeTaskData* data = static_cast<PrimeTaskData*>(arg);
    
    long long count = 0;
    for (long long i = data->start_num; i <= data->end_num; i++) {
        if (is_prime(i)) {
            count++;
        }
    }
    
    data->primes_found = count; // Save the result
    printf("Task %d completed: Found %lld primes between %lld and %lld\n", 
           data->task_id, count, data->start_num, data->end_num);
}

// CPU-heavy dummy function
void heavy_cpu_compute(std::shared_ptr<std::vector<char>> buffer, int task_id) {
    printf("[CPU Worker] Task %d started heavy computation...\n", task_id);
    
    long long checksum = 0;
    for (char c : *buffer) {
        checksum += c; 
    }
    
    
    printf("[CPU Worker] Task %d finished computation. Checksum: %lld\n", task_id, checksum);
}

int main() {
    const int NUM_CPU_WORKERS = 18;
    const int NUM_IO_WORKERS = 36; 
    const int NUM_FILES = 1000;
    const size_t FILE_SIZE = 1024 *1024; // 10 MB
    
    printf("--- Initializing Engine ---\n");
    printf("CPU Workers: %d | I/O Workers: %d\n", NUM_CPU_WORKERS, NUM_IO_WORKERS);
    
    
    auto overall_start = std::chrono::high_resolution_clock::now();
    {
    //CPU TASK
    thread_pool cpu_pool(NUM_CPU_WORKERS);
    IO_Pool io_pool(NUM_IO_WORKERS, &cpu_pool);
    const int NUM_TASKS=1000;
    const int RANGE_PER_TASK=100000;
    std::vector<PrimeTaskData> tasks(NUM_TASKS);
     for (int i = 0; i < NUM_TASKS; ++i) {
            tasks[i].task_id = i + 1;
            tasks[i].start_num = i * RANGE_PER_TASK + 1;
            tasks[i].end_num = (i + 1) * RANGE_PER_TASK;
            tasks[i].primes_found = 0;
            std::function<void(void*)> func=calculate_primes;
            auto cop=tasks[i];
            std::function<void()> pp=[func,cop](){calculate_primes((void*)&cop);};
            if(i%2==0) cpu_pool.submit(pp,priority::HIGH); 
            else    cpu_pool.submit(pp,priority::LOW);
            
        }


    // ==========================================
    // PHASE 1: Asynchronous Write Test
    // ==========================================

    printf("\n[Phase 1] Submitting %d async write tasks...\n", NUM_FILES);
    std::atomic<int> completed_writes{0};
    int successful_write_submissions = 0;
    
    auto start_write = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_FILES; ++i) {
        std::string filename = "async_test_" + std::to_string(i) + ".dat";
        
        int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd < 0) {
            printf("[MAIN] ERROR: Failed to open file for writing: %s\n", filename.c_str());
            continue; // Skip this file
        }

        auto buffer = std::make_shared<std::vector<char>>(FILE_SIZE, 'A' + (i % 26));

        // Callback captures the task ID (i) for debugging
        auto write_callback = [fd, &completed_writes, i](std::shared_ptr<std::vector<char>> buf) {
            close(fd); 
            int current = completed_writes.fetch_add(1) + 1;
            printf("[IO Callback] Write %d completed. Total done: %d\n", i, current);
        };

        io_pool.submit(fd, FILE_SIZE, IO_WRITE, write_callback, false, buffer);
        successful_write_submissions++;
        printf("[MAIN] Successfully submitted write task %d\n", i);
    }

    // Spin-wait loop with debug output
    printf("\n[MAIN] Waiting for %d writes to finish...\n", successful_write_submissions);
    while (completed_writes.load() < successful_write_submissions) {
        printf("[MAIN] Progress: %d / %d writes done...\r", completed_writes.load(), successful_write_submissions);
        fflush(stdout); // Force print to terminal
        usleep(50000);  // Sleep for 50ms
    }

    auto end_write = std::chrono::high_resolution_clock::now();
    
    
    // ==========================================
    // PHASE 2: Asynchronous Read + Compute Test
    // ==========================================
    printf("\n[Phase 2] Submitting %d async read tasks...\n", NUM_FILES);
    std::atomic<int> completed_reads{0};
    int successful_read_submissions = 0;
    
    auto start_read = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_FILES; ++i) {
        std::string filename = "async_test_" + std::to_string(i) + ".dat";
        
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd < 0) {
            printf("[MAIN] ERROR: Failed to open file for reading: %s\n", filename.c_str());
            continue;
        }
        
        auto buffer = std::make_shared<std::vector<char>>(FILE_SIZE);
        
        auto read_callback = [fd, &completed_reads, i](std::shared_ptr<std::vector<char>> buf) {
            printf("[IO Callback] Read %d finished. Sending to Compute Pool...\n", i);
            
            heavy_cpu_compute(buf, i);
            
            close(fd); 
            int current = completed_reads.fetch_add(1) + 1;
            printf("[Compute Callback] Task %d fully complete. Total done: %d\n", i, current);
        };
        
        io_pool.submit(fd, FILE_SIZE, IO_READ, read_callback, false, buffer);
        successful_read_submissions++;
    }
    
    // Spin-wait loop with debug output
    printf("\n[MAIN] Waiting for %d reads & computations to finish...\n", successful_read_submissions);
    while (completed_reads.load() < successful_read_submissions) {
        printf("[MAIN] Progress: %d / %d reads done...\r", completed_reads.load(), successful_read_submissions);
        fflush(stdout);
        usleep(50000); 
    }
}
    
    auto end_read = std::chrono::high_resolution_clock::now();
    
    
        // ==========================================
        // CLEANUP
        // ==========================================
        
        // printf("\n\n[Phase 1 Complete] Write Time: %f seconds.\n", 
        //    std::chrono::duration<double>(end_write - start_write).count());
           
        //    printf("\n\n[Phase 2 Complete] Read + Compute Time: %f seconds.\n", 
        //        std::chrono::duration<double>(end_read - start_read).count());


        printf("\n\nTotal Complete: %f seconds\n",std::chrono::duration<double>(end_read - overall_start).count());

           printf("\n[MAIN] Cleaning up files...\n");
           for (int i = 0; i < NUM_FILES; ++i) {
               std::string filename = "async_test_" + std::to_string(i) + ".dat";
               remove(filename.c_str());
    }
    
    printf("[MAIN] All done. Graceful shutdown initiating...\n");
    return 0;
}