#pragma once
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace minicachedb {

class KVStore {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using NowFn = std::function<TimePoint()>;

  explicit KVStore(NowFn now = [] { return Clock::now(); });

  void set(std::string key, std::string value, std::optional<int> ttl_seconds = std::nullopt);
  std::optional<std::string> get(const std::string& key);
  bool del(const std::string& key);

  size_t size() const;

 private:
  struct Entry {
    std::string value;
    std::optional<TimePoint> expires_at;
  };

  bool is_expired(const Entry& e, TimePoint now) const;
  void maybe_purge_expired_locked(TimePoint now, const std::string& key);

  NowFn now_;
  mutable std::mutex m_;
  std::unordered_map<std::string, Entry> map_;
};

}  // namespace minicachedb
