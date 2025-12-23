#pragma once
#include <vector>
#include <mutex>
#include "car.h"

using namespace std;
class CarProcessor {
private:
    vector<Car> cars;        // Исходный список автомобилей
    int minPrice;                 // Минимальная цена
    int maxPrice;                 // Максимальная цена
    int maxMileage;               // Максимальный пробег
    int minYear;                  // Минимальный год выпуска
    
    mutex resultMutex;       // Мьютекс для синхронизации доступа к результатам
    
    // Метод для обработки части массива автомобилей
    void processChunk(size_t start, size_t end, vector<Car>& result);
    
public:
    CarProcessor(const vector<Car>& cars, int minP, int maxP, int maxM, int minY);
    
    // Однопоточная обработка
    vector<Car> processSingleThread();
    
    // Многопоточная обработка
    vector<Car> processMultiThread(int numThreads);
};