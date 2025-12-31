#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace minicachedb {

class ThreadPool {
 public:
  explicit ThreadPool(int workers);
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  void submit(std::function<void()> task);

 private:
  void worker_loop();

  std::mutex m_;
  std::condition_variable cv_;
  bool stop_ = false;
  std::queue<std::function<void()>> q_;
  std::vector<std::thread> threads_;
};

}  // namespace minicachedb
