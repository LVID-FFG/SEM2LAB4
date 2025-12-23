#pragma once
#include <string>
#include <iostream>
using namespace std;
// Структура для хранения информации об автомобиле
struct Car {
    string brand;     // Марка автомобиля
    int price;             // Цена
    int mileage;           // Пробег
    string bodyType;  // Тип кузова
    int year;              // Год выпуска
    
    Car(const string& b, int p, int m, const string& bt, int y) : brand(b), price(p), mileage(m), bodyType(bt), year(y) {}
    
    // Метод для проверки соответствия критериям
    bool matchesCriteria(int minPrice, int maxPrice, int maxMileage, int minYear) const {
        return (price >= minPrice && price <= maxPrice) && (mileage <= maxMileage) && (year >= minYear);
    }
    
    // Метод для вывода информации об автомобиле
    void printInfo() const {
        cout << brand << " | Цена: " << price << " | Пробег: " << mileage << " | Кузов: " << bodyType << " | Год: " << year << endl;
    }
};