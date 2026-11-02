//
// Created by dawid on 11/1/26.
//

#ifndef BSRENDERER_THREAD_POOL_H
#define BSRENDERER_THREAD_POOL_H
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "defines.h"

class thread_pool {
public:
    explicit thread_pool(u32 thread_count);
    ~thread_pool();

    void enqueue(std::function<void()> job);
    void wait_blocking();

    u32 get_allocated_thread_count() const { return _workers.size(); }
private:
    void worker_loop();

    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _jobs;

    std::mutex _mutex;
    std::condition_variable _cond;
    std::condition_variable _finished;

    bool _stop;
    u32 _active_count;
};

#endif //BSRENDERER_THREAD_POOL_H