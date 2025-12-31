#include "core/kv_store.hpp"

namespace minicachedb {

KVStore::KVStore(NowFn now) : now_(std::move(now)) {}

bool KVStore::is_expired(const Entry& e, TimePoint now) const {
  return e.expires_at.has_value() && now >= *e.expires_at;
}

void KVStore::maybe_purge_expired_locked(TimePoint now, const std::string& key) {
  auto it = map_.find(key);
  if (it == map_.end()) return;
  if (is_expired(it->second, now)) map_.erase(it);
}

void KVStore::set(std::string key, std::string value, std::optional<int> ttl_seconds) {
  auto now = now_();
  Entry e;
  e.value = std::move(value);
  if (ttl_seconds.has_value()) e.expires_at = now + std::chrono::seconds(*ttl_seconds);

  std::lock_guard<std::mutex> lk(m_);
  map_[std::move(key)] = std::move(e);
}

std::optional<std::string> KVStore::get(const std::string& key) {
  auto now = now_();
  std::lock_guard<std::mutex> lk(m_);
  maybe_purge_expired_locked(now, key);
  auto it = map_.find(key);
  if (it == map_.end()) return std::nullopt;
  return it->second.value;
}

bool KVStore::del(const std::string& key) {
  std::lock_guard<std::mutex> lk(m_);
  return map_.erase(key) > 0;
}

size_t KVStore::size() const {
  std::lock_guard<std::mutex> lk(m_);
  return map_.size();
}

}  // namespace minicachedb
