#include "car_processor.h"
#include <thread>
#include <algorithm>
#include <iostream>

using namespace std;

CarProcessor::CarProcessor(const vector<Car>& cars, int minP, int maxP, int maxM, int minY) : cars(cars), minPrice(minP), maxPrice(maxP), maxMileage(maxM), minYear(minY) {}

// Метод для обработки части массива
void CarProcessor::processChunk(size_t start, size_t end, vector<Car>& result) {
    vector<Car> localResult;
    
    // Фильтрация автомобилей в заданном диапазоне
    for (size_t i = start; i < end && i < cars.size(); ++i) {
        if (cars[i].matchesCriteria(minPrice, maxPrice, maxMileage, minYear)) {
            localResult.push_back(cars[i]);
        }
    }
    
    // Добавление результатов в общий список
    lock_guard<mutex> lock(resultMutex);
    result.insert(result.end(), localResult.begin(), localResult.end());
}

// Однопоточная обработка
vector<Car> CarProcessor::processSingleThread() {
    vector<Car> result;
    processChunk(0, cars.size(), result);
    return result;
}

// Многопоточная обработка
vector<Car> CarProcessor::processMultiThread(int numThreads) {
    vector<Car> result;
    vector<thread> threads;
    
    // Рассчитываем размер чанка для каждого потока
    size_t chunkSize = cars.size() / numThreads;
    
    // Создаем и запускаем потоки
    for (int i = 0; i < numThreads; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? cars.size() : start + chunkSize;
        
        if (start < cars.size()) {
            // Запускаем поток для обработки диапазона
            threads.emplace_back(&CarProcessor::processChunk, this, start, end, ref(result));
        }
    }
    
    // Ожидаем завершения всех потоков
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    return result;
}