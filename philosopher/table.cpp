#include "table.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
using namespace std;
Table::Table(int philosophersCount) : philosophersCount(philosophersCount), forks(philosophersCount) {}

bool Table::takeForks(int philosopherId) {
    int left = leftFork(philosopherId);
    int right = rightFork(philosopherId);
    
    int first = min(left, right);
    int second = max(left, right);
    
    forks[first].lock();
    
    // Пытаемся заблокировать вторую вилку
    if (forks[second].try_lock()) {
        // Обе вилки взяты
        totalMeals++;
        return true;
    } else {
        // Не удалось взять вторую вилку - освобождаем первую
        forks[first].unlock();
        return false;
    }
}

void Table::releaseForks(int philosopherId) {
    int left = leftFork(philosopherId);
    int right = rightFork(philosopherId);
    
    // Всегда освобождаем в порядке возрастания номеров
    int first = min(left, right);
    int second = max(left, right);
    
    forks[second].unlock();
    forks[first].unlock();
}

void Table::printStatus() {
    cout << endl << string(50, '=') << endl;
    cout << "Текущее состояние стола:" << endl;
    cout << string(50, '-') << endl;
    
    cout << "Всего приемов пищи: " << totalMeals << endl;
    
    int busyForks = 0;
    cout << endl << "Состояние вилок:" << endl;
    for (int i = 0; i < philosophersCount; ++i) {
        if (forks[i].try_lock()) {
            cout << "  Вилка " << i << ": свободна" << endl;
            forks[i].unlock();
        } else {
            busyForks++;
            cout << "  Вилка " << i << ": ЗАНЯТА" << endl;
        }
    }
    
    cout << endl << "Занято вилок: " << busyForks << " из " << philosophersCount << endl;
    cout << string(50, '=') << endl;
}