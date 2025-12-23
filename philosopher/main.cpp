#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <climits>
#include <limits>
#include "philosopher.h"
#include "table.h"

using namespace std;

atomic<bool> running(true);

// Функция для безопасного ввода целого числа
int inputInt(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt << " [" << minVal << "-" << maxVal << "]: ";
        cin >> value;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ошибка! Введите целое число." << endl;
        } else if (value < minVal || value > maxVal) {
            cout << "Ошибка! Значение должно быть от " << minVal 
                 << " до " << maxVal << "." << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

void printStatistics(const vector<unique_ptr<Philosopher>>& philosophers, const Table& table, int elapsedSeconds) {
    // Даем немного времени для завершения вывода сообщений
    this_thread::sleep_for(chrono::milliseconds(100));
    
    cout << endl << "СТАТИСТИКА (" << elapsedSeconds << " сек)" << endl;
    
    cout << left << setw(10) << "Философ "  
         << setw(10) << "Пища " 
         << setw(12) << "Думал(с) " 
         << setw(12) << "Ел(с) " 
         << setw(12) << "Ждал(с) " 
         << setw(12) << "Эффект(%) " << endl;
    cout << string(80, '-') << endl;
    
    long long totalMeals = 0;
    long long totalThink = 0;
    long long totalEat = 0;
    long long totalWait = 0;
    
    for (const auto& philosopher : philosophers) {
        long long thinkMs = philosopher->getThinkingTime();
        long long eatMs = philosopher->getEatingTime();
        long long waitMs = philosopher->getWaitingTime();
        
        // Рассчитываем эффективность для каждого философа
        double efficiency = 0.0;
        if (eatMs + waitMs > 0) {
            efficiency = (double)eatMs / (eatMs + waitMs) * 100;
        }
        
        cout << left << setw(10) << philosopher->getId()
             << setw(10) << philosopher->getMealsEaten()
             << setw(12) << fixed << setprecision(1) << (thinkMs / 1000.0)
             << setw(12) << fixed << setprecision(1) << (eatMs / 1000.0)
             << setw(12) << fixed << setprecision(1) << (waitMs / 1000.0)
             << setw(12) << fixed << setprecision(1) << efficiency << endl;
        
        totalMeals += philosopher->getMealsEaten();
        totalThink += thinkMs;
        totalEat += eatMs;
        totalWait += waitMs;
    }
    
    cout << string(80, '-') << endl;
    cout << left << setw(10) << "ИТОГО:"
         << setw(10) << totalMeals
         << setw(12) << fixed << setprecision(1) << (totalThink / 1000.0)
         << setw(12) << fixed << setprecision(1) << (totalEat / 1000.0)
         << setw(12) << fixed << setprecision(1) << (totalWait / 1000.0)
         << endl;
    
    cout << "\nВсего приемов пищи за столом: " << table.getTotalMeals() << endl;
    
    // Анализ эффективности
    cout << endl << "АНАЛИЗ ЭФФЕКТИВНОСТИ" << endl;
    long long totalActiveTime = totalEat + totalWait + totalThink;
    if (totalActiveTime > 0) {
        cout << fixed << setprecision(1);
        cout << "Распределение времени:" << endl;
        cout << "  - Думал: " << (double)totalThink / totalActiveTime * 100 << "%" << endl;
        cout << "  - Ел: " << (double)totalEat / totalActiveTime * 100 << "%" << endl;
        cout << "  - Ждал: " << (double)totalWait / totalActiveTime * 100 << "%" << endl;
    }
    
    // Общая эффективность
    if (totalEat + totalWait > 0) {
        double totalEfficiency = (double)totalEat / (totalEat + totalWait) * 100;
        cout << "  - Эффективность (время еды): " << totalEfficiency << "%" << endl;
    }
    
    if (totalMeals > 0) {
        int minMeals = INT_MAX;
        int maxMeals = 0;
        for (const auto& philosopher : philosophers) {
            int meals = philosopher->getMealsEaten();
            if (meals < minMeals) minMeals = meals;
            if (meals > maxMeals) maxMeals = meals;
        }
        
        cout << "Мин. приемов пищи: " << minMeals << endl;
        cout << "Макс. приемов пищи: " << maxMeals << endl;
        
        if (maxMeals > 0) {
            double fairness = (double)minMeals / maxMeals * 100;
            cout << "Справедливость: " << fixed << setprecision(1) << fairness << "%" << endl;
        }
    }
    
    // Анализ производительности
    if (elapsedSeconds > 0) {
        double mealsPerSecond = (double)totalMeals / elapsedSeconds;
        cout << endl << "ПРОИЗВОДИТЕЛЬНОСТЬ" << endl;
        cout << "Приемов пищи в секунду: " << fixed << setprecision(2) << mealsPerSecond << endl;
        cout << "Среднее время на прием пищи: " << fixed << setprecision(1) << (elapsedSeconds * 1000.0 / totalMeals) << " мс" << endl;
    }
}

int main() {
    // Настройка параметров
    cout << "НАСТРОЙКА ПАРАМЕТРОВ:" << endl;
    int philosophersCount = inputInt("Количество философов", 3, 50);
    int simulationTime = inputInt("Время симуляции (сек)", 10, 600);
    
    // Создаем стол
    Table table(philosophersCount);
    
    // Создаем философов
    vector<unique_ptr<Philosopher>> philosophers;
    for (int i = 0; i < philosophersCount; ++i) {
        philosophers.push_back(make_unique<Philosopher>(i, table));
    }
    
    // Запускаем философов
    cout << "Запуск " << philosophersCount << " философов..." << endl;
    for (auto& philosopher : philosophers) {
        philosopher->start();
    }
    
    // Основной цикл мониторинга
    cout << endl << "НАЧАЛО СИМУЛЯЦИИ" << endl;
    int seconds = 0;
    while (seconds < simulationTime) {
        this_thread::sleep_for(chrono::seconds(10));
        seconds += 10;
        
        cout << endl << "Прошло: " << seconds << " сек" << endl;
        table.printStatus();
        printStatistics(philosophers, table, seconds);
    }
    
    // Останавливаем философов
    cout << endl << "ЗАВЕРШЕНИЕ СИМУЛЯЦИИ" << endl;
    cout << "Остановка философов..." << endl;
    running = false;
    
    // Останавливаем стол
    table.stop();
    
    // Даем время на завершение текущих циклов
    this_thread::sleep_for(chrono::seconds(1));
    
    // Останавливаем потоки
    for (auto& philosopher : philosophers) {
        philosopher->stop();
    }
    
    // Ждем завершения всех потоков
    for (auto& philosopher : philosophers) {
        philosopher->join();
    }
    
    // Финальная статистика
    cout << "ФИНАЛЬНАЯ СТАТИСТИКА (" << simulationTime << " сек)" << endl;
    
    int totalMeals = 0;
    long long totalWait = 0;
    
    for (const auto& philosopher : philosophers) {
        totalMeals += philosopher->getMealsEaten();
        totalWait += philosopher->getWaitingTime();
    }
    
    cout << "Всего приемов пищи: " << totalMeals << endl;
    cout << "Общее время ожидания: " << (totalWait / 1000.0) << " сек" << endl;
    cout << "Среднее время ожидания на философа: " << (totalWait / philosophersCount / 1000.0) << " сек" << endl;
    cout << "Среднее количество приемов пищи на философа: " << fixed << setprecision(2) << (double)totalMeals / philosophersCount << endl;
    
    // Проверка на deadlock (никто не должен голодать вечно)
    bool deadlockDetected = false;
    for (const auto& philosopher : philosophers) {
        if (philosopher->getMealsEaten() == 0) {
            cout << "Философ " << philosopher->getId() << " не поел ни разу!" << endl;
            deadlockDetected = true;
        }
    }
    
    if (!deadlockDetected) {
        cout << endl << "Все философы поели, deadlock не обнаружен" << endl;
    } else {
        cout << endl << "Обнаружен возможный deadlock!" << endl;
    }
    
    return 0;
}