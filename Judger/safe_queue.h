//
// Created by kjctar on 2022/3/21.
//

#ifndef UNTITLED_SAFE_QUEUE_H
#define UNTITLED_SAFE_QUEUE_H

#endif //UNTITLED_SAFE_QUEUE_H
#include <queue>
#include <mutex>
#include <condition_variable>
template<typename T>
class tsqueue{
public:
    tsqueue()=default;
    tsqueue(tsqueue const& other){
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value){
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    T pop(){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty(); });
        T value = data_queue.front();
        data_queue.pop();
        return value;
    }
    bool empty() const{
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
    int size() const{
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
};