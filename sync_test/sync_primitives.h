#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <semaphore>
#include <barrier>
#include <thread>
#include "race_common.h"

using namespace std;

// 1. Мьютекс
class MutexWrapper {
    mutex mtx;
public:
    MutexWrapper(const SyncParams& params = SyncParams()) {}
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
};

// 2. Семафор
class SemaphoreWrapper {
    counting_semaphore<> sem;
    int maxCount;
public:
    SemaphoreWrapper(const SyncParams& params = SyncParams()) : sem(params.semaphoreCount), maxCount(params.semaphoreCount) {}
    
    void lock() { sem.acquire(); }
    void unlock() { sem.release(); }
    
    int getMaxCount() const { return maxCount; }
};

// 3. SpinLock
class SpinLock {
    atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    SpinLock(const SyncParams& params = SyncParams()) {}
    void lock() {
        while (flag.test_and_set(memory_order_acquire)) {
            // активное ожидание
        }
    }
    void unlock() {
        flag.clear(memory_order_release);
    }
};

// 4. SpinWait
class SpinWait {
    atomic_flag flag = ATOMIC_FLAG_INIT;
    int maxSpinIterations;
public:
    SpinWait(const SyncParams& params = SyncParams()) : maxSpinIterations(params.spinWaitIterations) {}
    
    void lock() {
        int spinCount = 0;
        while (flag.test_and_set(memory_order_acquire)) {
            if (spinCount++ < maxSpinIterations) {
                this_thread::yield();
            } else {
                this_thread::sleep_for(chrono::microseconds(1));
                spinCount = 0;
            }
        }
    }
    void unlock() {
        flag.clear(memory_order_release);
    }
};

// 5. Monitor
class Monitor {
    mutex mtx;
    condition_variable cv;
public:
    Monitor(const SyncParams& params = SyncParams()) {
        // Monitor не имеет параметров
    }
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
    void wait() {
        unique_lock<mutex> lock(mtx, adopt_lock);
        cv.wait(lock);
        lock.release();
    }
    void notify() { cv.notify_one(); }
    void notify_all() { cv.notify_all(); }
};

// 6. Барьер
class BarrierWrapper {
    unique_ptr<std::barrier<>> barrier;  // Используем умный указатель
    int phases;
    int count;
public:
    // Конструктор по умолчанию
    BarrierWrapper(const SyncParams& params = SyncParams()) : barrier(nullptr), phases(params.barrierPhases), count(0) {}
    
    // Конструктор с параметрами
    BarrierWrapper(int count, const SyncParams& params = SyncParams()) 
        : barrier(make_unique<std::barrier<>>(count)), 
          phases(params.barrierPhases), 
          count(count) {}
    
    void lock() {
        if (!barrier) return;  // На всякий случай проверяем
        
        for (int i = 0; i < phases; ++i) {
            barrier->arrive_and_wait();
        }
    }
    
    void unlock() {
        // Для барьера unlock не нужен
    }
};