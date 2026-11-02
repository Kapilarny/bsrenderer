//
// Created by dawid on 11/1/26.
//

#include "thread_pool.h"

void thread_pool::worker_loop() {
    while (true) {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock, [this] {
                return _stop || !_jobs.empty();
            });

            if (_stop && _jobs.empty()) {
                return;
            }

            job = std::move(_jobs.front());
            _jobs.pop();

            ++_active_count;
        }

        job();

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _active_count--;
        }

        _finished.notify_all();
    }
}

thread_pool::thread_pool(const u32 thread_count) : _stop(false), _active_count(0) {
    for (u32 i = 0; i < thread_count; i++) {
        _workers.emplace_back([this] {
             worker_loop();
        });
    }
}

thread_pool::~thread_pool() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
    }

    _cond.notify_all();

    for (auto& worker : _workers) {
        worker.join();
    }
}

void thread_pool::enqueue(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _jobs.push(std::move(job));
    }

    _cond.notify_one();
}

void thread_pool::wait_blocking() {
    std::unique_lock<std::mutex> lock(_mutex);

    _finished.wait(lock, [this] {
        return _jobs.empty() && _active_count == 0;
    });
}