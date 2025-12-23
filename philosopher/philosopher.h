#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <iostream>
using namespace std;
class Table;

class Philosopher {
private:
    int id;
    Table& table;
    
    std::thread thread;
    
    // Статистика
    atomic<int> mealsEaten{0};
    atomic<long long> thinkingTime{0};
    atomic<long long> eatingTime{0};
    atomic<long long> waitingTime{0};
    
    // Состояние
    atomic<bool> running{true};
    
    // Случайные задержки
    random_device rd;
    mt19937 gen;
    uniform_int_distribution<> thinkDist;
    uniform_int_distribution<> eatDist;
    
    void live();
    void think();
    void eat();
    
public:
    Philosopher(int id, Table& table);
    ~Philosopher();
    
    void start();
    void stop();
    void join();
    
    // Получение статистики
    int getMealsEaten() const { return mealsEaten.load(); }
    long long getThinkingTime() const { return thinkingTime.load(); }
    long long getEatingTime() const { return eatingTime.load(); }
    long long getWaitingTime() const { return waitingTime.load(); }
    int getId() const { return id; }
};