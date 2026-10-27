//
// Created by walte on 23/10/2026.
//

#ifndef LOW_LEVEL_SETUP_CONCURRENTVECTOR_H
#define LOW_LEVEL_SETUP_CONCURRENTVECTOR_H
#include <vector>
#include <iostream>
#include <mutex>

#endif //LOW_LEVEL_SETUP_CONCURRENTVECTOR_H

template <typename T>
class ConcurrentVector
{
    public:
    ConcurrentVector()
    {
        internal = std::vector<T>();
    };
    ~ConcurrentVector() = default;



    T TryGet(int index)
    {
        std::lock_guard<std::mutex> lock(vecMtx);
        if (index >= internal.size()){ std::cerr << "Index out of bounds" << std::endl; return T(); }
        return internal[index];
    };
    bool UpdateOrAdd(int index, T value)
    {
        std::lock_guard<std::mutex> lock(vecMtx);
        if (index >= internal.size()) { std::cerr << "Index out of bounds" << std::endl; return false; }
        internal[index] = value;
        return true;
    };



    void PushBack(T value)
    {
        std::lock_guard<std::mutex> lock(vecMtx);
        internal.push_back(value);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(vecMtx);
        internal.clear();
    }

    int Size()
    {
        std::lock_guard<std::mutex> lock(vecMtx);
        return internal.size();
    }

    private:
    std::vector<T> internal;
    std::mutex vecMtx;
};