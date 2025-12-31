#include "core/kv_store.hpp"
#include <cassert>
#include <chrono>
#include <iostream>

using minicachedb::KVStore;

int main() {
  KVStore::TimePoint now = KVStore::Clock::now();
  KVStore store([&]() { return now; });

  store.set("a", "1");
  assert(store.get("a").has_value());
  assert(*store.get("a") == "1");

  assert(store.del("a") == true);
  assert(!store.get("a").has_value());

  store.set("k", "v", 1);
  assert(store.get("k").has_value());

  now += std::chrono::seconds(2);
  assert(!store.get("k").has_value());

  std::cout << "test_kv_store OK\n";
  return 0;
}
