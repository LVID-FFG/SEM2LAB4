#pragma once

#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "sync_primitives.h"

using namespace std;

// Глобальный мьютекс для безопасного вывода
inline mutex g_cout_mutex;

// Структура для расширенной статистики
struct ExtendedTimingResult {
    string primitiveName;
    long long totalTimeNs;
    long long avgTimePerThreadNs;
    SyncParams params;
    
    // Новая статистика
    long long minTimeNs;
    long long maxTimeNs;
    double meanTimeNs;
    double medianTimeNs;
    double stdDevNs;
    int measurementRuns;
};

template<typename SyncPrimitive>
class RaceRunner {
    RaceConfig config;
    SyncPrimitive primitive;
    vector<thread> threads;
    vector<long long> threadTimes;
    chrono::high_resolution_clock::time_point globalStart;
    
    void threadFunction(int threadId) {
        auto threadStart = chrono::high_resolution_clock::now();
        
        for (int i = 0; i < config.iterations; ++i) {
            primitive.lock();
            
            if (config.verboseOutput) {
                lock_guard<mutex> coutLock(g_cout_mutex);
                cout << "[" << threadId << "] " << i << ": ";
                for (int j = 0; j < 5; ++j) {
                    cout << randomAsciiChar();
                }
                cout << endl;
            }
            
            doWork(config.workPerIteration);
            
            primitive.unlock();
            
            this_thread::sleep_for(chrono::microseconds(10));
        }
        
        auto threadEnd = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(threadEnd - threadStart);
        threadTimes[threadId] = duration.count();
    }
    
    TimingResult runSingle() {
        threadTimes.clear();
        threadTimes.resize(config.threadCount);
        
        globalStart = chrono::high_resolution_clock::now();
        
        // Создаем потоки
        for (int i = 0; i < config.threadCount; ++i) {
            threads.emplace_back(&RaceRunner::threadFunction, this, i);
        }
        
        // Ждем завершения
        for (auto& t : threads) {
            t.join();
        }
        
        auto globalEnd = chrono::high_resolution_clock::now();
        auto totalDuration = chrono::duration_cast<chrono::nanoseconds>(globalEnd - globalStart);
        
        long long totalTime = totalDuration.count();
        long long sumThreadTime = 0;
        for (auto t : threadTimes) {
            sumThreadTime += t;
        }
        
        threads.clear();
        
        TimingResult result;
        result.totalTimeNs = totalTime;
        result.avgTimePerThreadNs = sumThreadTime / config.threadCount;
        
        return result;
    }
    
public:
    RaceRunner(RaceConfig cfg, const SyncParams& params = SyncParams()) 
        : config(cfg), primitive(params), threadTimes(cfg.threadCount) {}
    
    ExtendedTimingResult runWithStats(int warmupRuns = 3, int measurementRuns = 10) {
        vector<long long> measurements;
        measurements.reserve(measurementRuns);
        
        // 1. ПРОГРЕВ (не измеряем)
        for (int i = 0; i < warmupRuns; ++i) {
            if (config.verboseOutput && i == 0) {
                cout << "[Warmup " << (i+1) << "/" << warmupRuns << "]..." << endl;
            }
            runSingle(); // Запускаем, но результат игнорируем
        }
        
        // 2. ОСНОВНЫЕ ИЗМЕРЕНИЯ
        for (int i = 0; i < measurementRuns; ++i) {
            if (config.verboseOutput) {
                cout << "[Measurement " << (i+1) << "/" << measurementRuns << "]..." << endl;
            }
            auto result = runSingle();
            measurements.push_back(result.totalTimeNs);
        }
        
        // 3. СТАТИСТИЧЕСКАЯ ОБРАБОТКА
        ExtendedTimingResult extendedResult;
        
        // Минимум и максимум
        auto minmax = minmax_element(measurements.begin(), measurements.end());
        extendedResult.minTimeNs = *minmax.first;
        extendedResult.maxTimeNs = *minmax.second;
        
        // Среднее значение
        long long sum = accumulate(measurements.begin(), measurements.end(), 0LL);
        extendedResult.meanTimeNs = static_cast<double>(sum) / measurementRuns;
        
        // Медиана
        vector<long long> sorted = measurements;
        sort(sorted.begin(), sorted.end());
        extendedResult.medianTimeNs = (measurementRuns % 2 == 0) 
            ? (sorted[measurementRuns/2 - 1] + sorted[measurementRuns/2]) / 2.0
            : sorted[measurementRuns/2];
        
        // Стандартное отклонение
        double variance = 0.0;
        for (auto val : measurements) {
            double diff = val - extendedResult.meanTimeNs;
            variance += diff * diff;
        }
        variance /= measurementRuns;
        extendedResult.stdDevNs = sqrt(variance);
        
        // Остальные поля
        extendedResult.totalTimeNs = static_cast<long long>(extendedResult.meanTimeNs);
        extendedResult.avgTimePerThreadNs = extendedResult.totalTimeNs / config.threadCount;
        extendedResult.measurementRuns = measurementRuns;
        
        return extendedResult;
    }
    
    TimingResult run() {
        auto result = runSingle();
        result.primitiveName = ""; // Заполнится позже
        result.params = SyncParams(); // Заполнится позже
        return result;
    }
};
