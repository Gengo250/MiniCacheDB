#include "core/thread_pool.hpp"

namespace minicachedb {

ThreadPool::ThreadPool(int workers) {
  if (workers < 1) workers = 1;
  threads_.reserve(static_cast<size_t>(workers));
  for (int i = 0; i < workers; i++) {
    threads_.emplace_back([this]() { worker_loop(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lk(m_);
    stop_ = true;
  }
  cv_.notify_all();
  for (auto& t : threads_) {
    if (t.joinable()) t.join();
  }
}

void ThreadPool::submit(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lk(m_);
    q_.push(std::move(task));
  }
  cv_.notify_one();
}

void ThreadPool::worker_loop() {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lk(m_);
      cv_.wait(lk, [&]() { return stop_ || !q_.empty(); });
      if (stop_ && q_.empty()) return;
      task = std::move(q_.front());
      q_.pop();
    }
    task();
  }
}

}  // namespace minicachedb
