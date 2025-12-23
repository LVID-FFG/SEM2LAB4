#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "race_common.h"
#include "sync_primitives.h"
#include "race_runner.h"

using namespace std;

// Функция для безопасного ввода целого числа
int safeInputInt(const string& prompt, int minValue = 1, int maxValue = 1000) {
    int value;
    while (true) {
        cout << prompt << " [" << minValue << "-" << maxValue << "]: ";
        
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ошибка: введите число\n";
            continue;
        }
        
        if (value < minValue || value > maxValue) {
            cout << "Ошибка: от " << minValue << " до " << maxValue << "\n";
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value;
    }
}

bool askYesNo(const string& question) {
    string answer;
    while (true) {
        cout << question << " (y/n): ";
        getline(cin, answer);
        
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
        cout << "Введите y или n\n";
    }
}

void printStatistics(const ExtendedTimingResult& res) {
    cout << fixed << setprecision(2);
    cout << "\nСтатистика для " << res.primitiveName << ":\n";
    cout << "  Прогрев: " << 3 << " итераций, Измерений: " << res.measurementRuns << "\n";
    cout << "  Минимум:   " << res.minTimeNs / 1000.0 << " µs\n";
    cout << "  Максимум:  " << res.maxTimeNs / 1000.0 << " µs\n";
    cout << "  Среднее:   " << res.meanTimeNs / 1000.0 << " µs\n";
    cout << "  Медиана:   " << res.medianTimeNs / 1000.0 << " µs\n";
    cout << "  Станд.откл: " << res.stdDevNs / 1000.0 << " µs ";
    cout << "(" << (res.stdDevNs / res.meanTimeNs * 100.0) << "%)\n";
    
    // Относительный разброс
    double spread = (res.maxTimeNs - res.minTimeNs) / res.meanTimeNs * 100.0;
    cout << "  Разброс:   " << spread << "%\n";
}

void runComparativeAnalysis(const RaceConfig& config, const SyncParams& params) {
    cout << "\nНастройки: " << config.threadCount << " потоков, "
              << config.iterations << " итераций, работа=" 
              << config.workPerIteration << "\n";
    
    cout << "Параметры: семафор=" << params.semaphoreCount 
              << ", spinWait=" << params.spinWaitIterations 
              << ", барьер фаз=" << params.barrierPhases << "\n";
    
    if (config.verboseOutput) {
        cout << "Вывод символов: ВКЛ\n";
    }
    cout << endl;
    
    vector<ExtendedTimingResult> results;
    
    // Запрашиваем количество измерений
    cout << "Количество прогонов для статистики (1-50): ";
    int measurementRuns;
    cin >> measurementRuns;
    measurementRuns = max(1, min(50, measurementRuns));
    
    // 1. Mutex
    {
        cout << "\n[1/6] Тестирование Mutex...";
        MutexWrapper mutex(params);
        RaceRunner<MutexWrapper> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "Mutex";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // 2. Semaphore
    {
        cout << "\n[2/6] Тестирование Semaphore(" << params.semaphoreCount << ")...";
        SemaphoreWrapper semaphore(params);
        RaceRunner<SemaphoreWrapper> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "Semaphore";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // 3. SpinLock
    {
        cout << "\n[3/6] Тестирование SpinLock...";
        SpinLock spinlock(params);
        RaceRunner<SpinLock> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "SpinLock";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // 4. SpinWait
    {
        cout << "\n[4/6] Тестирование SpinWait(" << params.spinWaitIterations << ")...";
        SpinWait spinwait(params);
        RaceRunner<SpinWait> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "SpinWait";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // 5. Monitor
    {
        cout << "\n[5/6] Тестирование Monitor...";
        Monitor monitor(params);
        RaceRunner<Monitor> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "Monitor";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // 6. Barrier
    {
        cout << "\n[6/6] Тестирование Barrier(фазы=" << params.barrierPhases << ")...";
        BarrierWrapper barrier(config.threadCount, params);
        RaceRunner<BarrierWrapper> runner(config, params);
        auto result = runner.runWithStats(3, measurementRuns);
        result.primitiveName = "Barrier";
        result.params = params;
        results.push_back(result);
        cout << " OK\n";
        printStatistics(result);
    }
    
    // Вывод сводной таблицы
    cout << endl;
    cout << "СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ (" << measurementRuns << " измерений)\n";
    cout << endl;
    cout << "Примитив       Параметр     Среднее(мкс)  Медиана(мкс)  Разброс(%)\n";
    cout << endl;
    
    for (const auto& res : results) {
        double meanUs = res.meanTimeNs / 1000.0;
        double medianUs = res.medianTimeNs / 1000.0;
        double spread = (res.maxTimeNs - res.minTimeNs) / res.meanTimeNs * 100.0;
        
        cout.width(12); cout << left << res.primitiveName;
        
        // Вывод параметра
        if (res.primitiveName == "Semaphore") {
            cout.width(12); cout << left << res.params.semaphoreCount;
        } else if (res.primitiveName == "SpinWait") {
            cout.width(12); cout << left << res.params.spinWaitIterations;
        } else if (res.primitiveName == "Barrier") {
            cout.width(12); cout << left << res.params.barrierPhases;
        } else {
            cout.width(12); cout << left << "-";
        }
        
        cout.width(14); cout << right << fixed << setprecision(2) << meanUs;
        cout.width(14); cout << right << medianUs;
        cout.width(12); cout << right << spread << "%" << endl;;
    }
    
    // Анализ стабильности
    cout << endl << "АНАЛИЗ СТАБИЛЬНОСТИ:" << endl;
    
    // Находим самый стабильный (наименьший разброс)
    auto mostStable = min_element(results.begin(), results.end(),
        [](const ExtendedTimingResult& a, const ExtendedTimingResult& b) {
            double spreadA = (a.maxTimeNs - a.minTimeNs) / a.meanTimeNs;
            double spreadB = (b.maxTimeNs - b.minTimeNs) / b.meanTimeNs;
            return spreadA < spreadB;
        });
    
    // Находим самый быстрый (по медиане)
    auto fastest = min_element(results.begin(), results.end(),
        [](const ExtendedTimingResult& a, const ExtendedTimingResult& b) {
            return a.medianTimeNs < b.medianTimeNs;
        });
    
    // Находим самый медленный
    auto slowest = max_element(results.begin(), results.end(),
        [](const ExtendedTimingResult& a, const ExtendedTimingResult& b) {
            return a.medianTimeNs < b.medianTimeNs;
        });
    
    cout << "Самый стабильный: " << mostStable->primitiveName << " (разброс " << ((mostStable->maxTimeNs - mostStable->minTimeNs) / mostStable->meanTimeNs * 100.0)<< "%)" << endl;
    
    cout << "Самый быстрый: " << fastest->primitiveName << " (" << fastest->medianTimeNs / 1000.0 << " µs)" << endl;
    
    cout << "Самый медленный: " << slowest->primitiveName << " (" << slowest->medianTimeNs / 1000.0 << " µs)"<< endl;
    
    cout << "Отношение быстрый/медленный: " << fixed << setprecision(2) << (double)slowest->medianTimeNs / fastest->medianTimeNs << "x" << endl;
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    cout << "Гонка символов ASCII" << endl;
    
    RaceConfig config;
    SyncParams params;
    
    config.threadCount = safeInputInt("Потоков", 1, 100);
    config.iterations = safeInputInt("Итераций", 1, 1000);
    config.workPerIteration = safeInputInt("Работы", 1, 1000000);
    
    // Ввод параметров примитивов
    cout << endl << "Параметры примитивов" << endl;
    params.semaphoreCount = safeInputInt("Разрешений семафора", 1, config.threadCount);
    params.spinWaitIterations = safeInputInt("Итераций SpinWait перед сном", 1, 10000);
    params.barrierPhases = safeInputInt("Фаз барьера", 1, 10);
    
    config.verboseOutput = askYesNo("Выводить символы?");
    
    cout << endl << "Запуск тестирования...";
    runComparativeAnalysis(config, params);
    
    return 0;
}