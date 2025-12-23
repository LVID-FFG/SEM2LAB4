#include "philosopher.h"
#include "table.h"
#include <iostream>
#include <chrono>
using namespace std;
Philosopher::Philosopher(int id, Table& table)
    : id(id), table(table), gen(rd() + id),
      thinkDist(1000, 3000), eatDist(1000, 2000) {
}

Philosopher::~Philosopher() {
    stop();
    if (thread.joinable()) {
        thread.join();
    }
}

void Philosopher::start() {
    running = true;
    thread = std::thread(&Philosopher::live, this);
}

void Philosopher::stop() {
    running = false;
}

void Philosopher::join() {
    if (thread.joinable()) {
        thread.join();
    }
}

void Philosopher::live() {
    while (running) {
        // Думаем
        think();
        
        if (!running) break;
        
        // Пытаемся взять вилки с несколькими попытками
        bool success = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 10;
        
        auto waitStart = chrono::steady_clock::now();
        
        while (!success && running && attempts < MAX_ATTEMPTS) {
            if (table.takeForks(id)) {
                success = true;
            } else {
                attempts++;
                if (running) {
                    this_thread::sleep_for(chrono::milliseconds(50));
                }
            }
        }
        
        auto waitEnd = chrono::steady_clock::now();
        waitingTime += chrono::duration_cast<chrono::milliseconds>(waitEnd - waitStart).count();
        
        if (success) {
            // Едим
            eat();
            mealsEaten++;
            
            // Освобождаем вилки
            table.releaseForks(id);
        }
        
        // Небольшая пауза перед следующей попыткой
        if (running) {
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
}

void Philosopher::think() {
    if (!running) return;
    
    int thinkTime = thinkDist(gen);
    
    auto start = chrono::steady_clock::now();
    
    // Делим время на части для возможности прервать
    int steps = thinkTime / 100;
    for (int i = 0; i < steps && running; i++) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    auto end = chrono::steady_clock::now();
    
    if (running) {
        thinkingTime += chrono::duration_cast<chrono::milliseconds>(end - start).count();
    }
}

void Philosopher::eat() {
    if (!running) return;
    
    int eatTime = eatDist(gen);
    
    auto start = chrono::steady_clock::now();
    
    int left = table.leftFork(id);
    int right = table.rightFork(id);
    cout << "Философ " << id << " ест (вилки: " << left << " и " << right << ")..." << endl;
    
    // Делим время на части для возможности прервать
    int steps = eatTime / 100;
    for (int i = 0; i < steps && running; i++) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    auto end = chrono::steady_clock::now();
    
    if (running) {
        eatingTime += chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "Философ " << id << " закончил есть (освободил вилки " << left << " и " << right << ")" << endl;
    }
}