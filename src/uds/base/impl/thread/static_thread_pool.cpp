#include "static_thread_pool.h"
#include <algorithm>
#include <chrono>

namespace ic {
namespace uds {

/**
 * @brief 构造函数.
 * 
 * @param size 线程池大小
 */
StaticThreadPool::StaticThreadPool(size_t size)
    : shared_src_(std::make_shared<pool_src>()), size_(size)
{
    for (size_t i = 0; i < size_; ++i) {
        std::thread t([this]{
            auto src = this->shared_src_;
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lck(src->queue_mutex);
                    src->cv.wait(lck, [&]{
                        return src->shutdown || !src->queue.empty();
                    });
                    if (src->shutdown && src->queue.empty()) {
                        return;
                    }
                    task = std::move(src->queue.front());
                    src->queue.pop();
                }
                task();
                if (src->running_tasks_count.fetch_sub(1) == 1) {
                    src->wait_cv.notify_one();
                }
            }
        });
        t.detach();
    }
}

/**
 * @brief 析构函数.
 */
StaticThreadPool::~StaticThreadPool() {
    shared_src_->shutdown = true;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    shared_src_->cv.notify_all();
    /* 暂停10毫秒，等待所有线程真正退出，释放资源 */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

/**
 * @brief 等待所有任务完成.
 */
void StaticThreadPool::Wait() {
    std::unique_lock<std::mutex> lck(shared_src_->wait_mutex);
    shared_src_->wait_cv.wait(lck, [this]{
        return shared_src_->running_tasks_count.load() == 0;
    });
}

} // namespace uds
} // namespace ic
