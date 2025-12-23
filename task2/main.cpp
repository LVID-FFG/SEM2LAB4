#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include <limits>
#include <iomanip>
#include <climits>
#include "car.h"
#include "car_processor.h"

using namespace std;

// Функция для безопасного ввода целого числа с валидацией
int inputInt(const string& prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        cout << prompt << " [" << minValue << " - " << maxValue << "]: ";
        cin >> value;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ошибка! Пожалуйста, введите целое число." << endl;
        } else if (value < minValue || value > maxValue) {
            cout << "Ошибка! Значение должно быть в диапазоне от " << minValue << " до " << maxValue << "." << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// Функция для генерации случайного автомобиля
Car generateRandomCar(int index, mt19937& gen) {
    vector<string> brands = {"Toyota", "Honda", "BMW", "Audi", "Ford", 
                            "Mercedes", "Volkswagen", "Hyundai", "Kia", 
                            "Nissan", "Mazda", "Subaru", "Lexus", "Volvo",
                            "Chevrolet", "Renault", "Peugeot", "Skoda", "Citroen"};
    vector<string> bodyTypes = {"Седан", "Хэтчбек", "Внедорожник", 
                               "Кроссовер", "Универсал", "Купе", "Минивэн", "Пикап"};
    
    uniform_int_distribution<> brandDist(0, brands.size() - 1);
    uniform_int_distribution<> priceDist(5000, 100000);
    uniform_int_distribution<> mileageDist(0, 300000);
    uniform_int_distribution<> bodyDist(0, bodyTypes.size() - 1);
    uniform_int_distribution<> yearDist(1990, 2024);
    
    return Car(
        brands[brandDist(gen)] + " Model-" + to_string(2000 + index % 25),
        priceDist(gen),
        mileageDist(gen),
        bodyTypes[bodyDist(gen)],
        yearDist(gen)
    );
}

// Функция для генерации тестовых данных
vector<Car> generateTestData(int dataSize) {
    cout << endl << "Генерация тестовых данных..." << endl;
    vector<Car> cars;
    cars.reserve(dataSize);
    
    random_device rd;
    mt19937 gen(rd());
    
    for (int i = 0; i < dataSize; ++i) {
        cars.push_back(generateRandomCar(i, gen));
        
        // Показываем прогресс для больших наборов данных
        if (dataSize >= 100000 && (i + 1) % (dataSize / 10) == 0) {
            cout << "Сгенерировано: " << (i + 1) << " из " << dataSize  << " автомобилей (" << ((i + 1) * 100 / dataSize) << "%)" << endl;
        }
    }
    
    // Вычисляем статистику по сгенерированным данным
    if (dataSize > 0) {
        int minPrice = INT_MAX, maxPrice = 0;
        int minYear = INT_MAX, maxYear = 0;
        int totalMileage = 0;
        
        for (const auto& car : cars) {
            if (car.price < minPrice) minPrice = car.price;
            if (car.price > maxPrice) maxPrice = car.price;
            if (car.year < minYear) minYear = car.year;
            if (car.year > maxYear) maxYear = car.year;
            totalMileage += car.mileage;
        }
        
        cout << endl << "СТАТИСТИКА СГЕНЕРИРОВАННЫХ ДАННЫХ" << endl;
        cout << "Всего автомобилей: " << dataSize << endl;
        cout << "Диапазон цен: от " << minPrice << " до " << maxPrice << endl;
        cout << "Диапазон годов выпуска: от " << minYear << " до " << maxYear << endl;
        cout << "Средний пробег: " << (totalMileage / dataSize) << " км" << endl;
    }
    
    return cars;
}

int main() {
    
    int dataSize = inputInt("Введите количество автомобилей для теста", 1000, 10000000);
    
    // Генерация тестовых данных
    vector<Car> cars = generateTestData(dataSize);
    
    cout << "Укажите критерии для поиска подходящих автомобилей:" << endl;
    
    int minPrice = inputInt("Минимальная цена", 0, 100000);
    int maxPrice = inputInt("Максимальная цена", minPrice, 100000);
    int maxMileage = inputInt("Максимальный пробег", 0, 500000);
    int minYear = inputInt("Минимальный год выпуска", 1990, 2024);
    
    int maxThreads = thread::hardware_concurrency();
    if (maxThreads == 0) maxThreads = 16; // Запасное значение
    
    cout << "Доступно логических ядер процессора: " << maxThreads << endl;
    int numThreads = inputInt("Введите количество потоков для многопоточной обработки", 1, maxThreads * 2);
    
    cout << "ВЫБРАННЫЕ ПАРАМЕТРЫ" << endl;
    cout << "Размер массива данных: " << dataSize << " автомобилей" << endl;
    cout << "Критерии фильтрации:" << endl;
    cout << "  - Диапазон цены: от " << minPrice << " до " << maxPrice << endl;
    cout << "  - Максимальный пробег: " << maxMileage << " км" << endl;
    cout << "  - Минимальный год выпуска: " << minYear << endl;
    cout << "Количество потоков: " << numThreads << endl;
    
    // Создание процессора для обработки автомобилей
    CarProcessor processor(cars, minPrice, maxPrice, maxMileage, minYear);
    
    // Однопоточная обработка
    cout << "ОДНОПОТОЧНАЯ ОБРАБОТКА" << endl;
    auto start = chrono::high_resolution_clock::now();
    vector<Car> singleThreadResult = processor.processSingleThread();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> singleThreadTime = end - start;
    
    cout << "Найдено автомобилей: " << singleThreadResult.size() << endl;
    cout << "Время обработки: " << fixed << setprecision(6) << singleThreadTime.count() << " секунд" << endl;
    
    // Многопоточная обработка
    cout << "МНОГОПОТОЧНАЯ ОБРАБОТКА" << endl;
    cout << "Используется потоков: " << numThreads << endl;
    
    start = chrono::high_resolution_clock::now();
    vector<Car> multiThreadResult = processor.processMultiThread(numThreads);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> multiThreadTime = end - start;
    
    cout << "Найдено автомобилей: " << multiThreadResult.size() << endl;
    cout << "Время обработки: " << fixed << setprecision(6) << multiThreadTime.count() << " секунд" << endl;
    
    // Проверка корректности результатов
    cout << "ПРОВЕРКА РЕЗУЛЬТАТОВ" << endl;
    if (singleThreadResult.size() != multiThreadResult.size()) {
        cout << "ВНИМАНИЕ: Разное количество результатов!" << endl;
        cout << "  Однопоточный режим: " << singleThreadResult.size() << " автомобилей" << endl;
        cout << "  Многопоточный режим: " << multiThreadResult.size() << " автомобилей" << endl;
    } else {
        cout << "Количество результатов совпадает: " 
             << singleThreadResult.size() << " автомобилей" << endl;
    }
    
    // Сравнение времени выполнения
    cout << "СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ" << endl;
    cout << fixed << setprecision(6);
    cout << "Однопоточная обработка: " << singleThreadTime.count() << " сек" << endl;
    cout << "Многопоточная обработка: " << multiThreadTime.count() << " сек" << endl;
}