#pragma once

#include <vector>
#include <mutex>
#include <atomic>
using namespace std;
class Table {
private:
    int philosophersCount;
    vector<mutex> forks;
    atomic<int> totalMeals{0};
    
public:
    Table(int philosophersCount);
    
    bool takeForks(int philosopherId);
    void releaseForks(int philosopherId);
    
    int leftFork(int id) const { return id; }
    int rightFork(int id) const { return (id + 1) % philosophersCount; }
    
    int getTotalMeals() const { return totalMeals.load(); }
    int getPhilosophersCount() const { return philosophersCount; }
    
    void printStatus();
    
    void stop() {}
};