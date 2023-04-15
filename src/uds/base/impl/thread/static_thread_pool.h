/**
 * @file static_thread_pool.h
 * @brief 静态线程池（线程数量固定）.
 * @author Jinbao Chen (leopard.c@outlook.com)
 * @version 0.1
 * @date 2022-12-30
 * 
 * @copyright Copyright (c) 2022 Leopard-C
 */
#ifndef IC_UDS_BASE_IMPL_THREAD_STATIC_THREAD_POOL_
#define IC_UDS_BASE_IMPL_THREAD_STATIC_THREAD_POOL_
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace ic {
namespace uds {

class StaticThreadPool {
public:
    explicit StaticThreadPool(size_t size = std::thread::hardware_concurrency() + 2);
    ~StaticThreadPool();

    /**
     * @brief 添加任务到队列.
     */
    template<typename Func, typename... Args>
    auto Enqueue(Func&& f, Args &&... args) -> std::future<typename std::result_of<Func(Args...)>::type>;

    /**
     * @brief 等待所有任务完成.
     */
    void Wait();

    /**
     * @brief 线程池的大小.
     */
    size_t size() const { return size_; }

    /**
     * @brief 正在运行的任务.
     */
    size_t running_tasks_count() const { return shared_src_->running_tasks_count; }

private:
    template <typename Type, typename Func, typename ... Args>
    inline void TryAllocate(Type& task, Func&& f, Args&& ... args);

private:
    using task_type = std::function<void()>;
    struct pool_src {
        bool shutdown{false};
        std::mutex queue_mutex;
        std::mutex wait_mutex;
        std::condition_variable cv;
        std::condition_variable wait_cv;
        std::queue<task_type> queue;
        std::atomic_size_t running_tasks_count{0};
    };
    const size_t size_;
    std::shared_ptr<pool_src> shared_src_;
};

template <typename Type, typename Func, typename ... Args>
inline void StaticThreadPool::TryAllocate(Type& task, Func&& f, Args&& ... args) {
    try {
        task = new typename std::remove_pointer<Type>::type(
            std::bind(std::forward<Func>(f),
            std::forward<Args>(args)...)
        );
    } catch (const std::exception& e) {
        if (task != nullptr) {
            delete task;
            task = nullptr;
        }
        throw e;
    }
}

/**
 * @brief 添加任务到队列.
 */
template<typename Func, typename... Args>
auto StaticThreadPool::Enqueue(Func&& f, Args &&... args)
    -> std::future<typename std::result_of<Func(Args...)>::type>
{
    using return_type = typename std::result_of<Func(Args...)>::type;
    std::packaged_task<return_type()>* task = nullptr;
    TryAllocate(task, std::forward<Func>(f), std::forward<Args>(args)...);
    auto result = task->get_future();
    {
        std::lock_guard<std::mutex> lck(shared_src_->queue_mutex);
        shared_src_->queue.push([task]{
            (*task)();
            delete task;
        });
    }
    shared_src_->running_tasks_count.fetch_add(1);
    shared_src_->cv.notify_one();
    return result;
}

} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_IMPL_THREAD_STATIC_THREAD_POOL_
