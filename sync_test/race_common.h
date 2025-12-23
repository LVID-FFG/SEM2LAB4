#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>
#include <memory>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <thread>

using namespace std;

// Конфигурация гонки
struct RaceConfig {
    int threadCount;      // Количество потоков
    int iterations;       // Итераций на поток
    int workPerIteration; // "Работы" на итерацию
    bool verboseOutput;   // Подробный вывод символов
};

// Параметры примитивов синхронизации
struct SyncParams {
    int semaphoreCount;    // Количество разрешений семафора
    int spinWaitIterations; // Итераций спин-ожидания перед сном
    int barrierPhases;     // Количество фаз барьера
    
    SyncParams() : semaphoreCount(1), spinWaitIterations(100), barrierPhases(1) {}
};

// Результаты тестирования
struct TimingResult {
    string primitiveName;
    long long totalTimeNs;
    long long avgTimePerThreadNs;
    SyncParams params;     // Параметры, с которыми тестировался примитив
};

// Функция для имитации работы
inline void doWork(int units) {
    // Компилятор не может предсказать результат random()
    mt19937 gen(random_device{}());
    uniform_int_distribution<> dis(1, 100);
    
    int sum = 0;
    for (int i = 0; i < units; ++i) {
        sum += dis(gen);  // Случайные числа нельзя оптимизировать
    }
    
    // Используем результат, чтобы компилятор не удалил цикл
    if (sum == 0) {
        cout << "..." << endl;
    }
}

// Генератор случайных ASCII символов
inline char randomAsciiChar() {
    static thread_local random_device rd;
    static thread_local mt19937 gen(rd());
    uniform_int_distribution<> dis(33, 126);
    return static_cast<char>(dis(gen));
}