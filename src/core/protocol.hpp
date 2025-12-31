#pragma once
#include "core/aof.hpp"
#include "core/kv_store.hpp"

#include <optional>
#include <string>
#include <vector>

namespace minicachedb {

class Protocol {
 public:
  Protocol(KVStore& store, AOF* aof);

  std::string execute_line(const std::string& line, bool append_to_aof = true);

  // Exposed for server-side QUIT detection (simple MVP convenience).
  static std::vector<std::string> split_ws(const std::string& s);

 private:
  KVStore& store_;
  AOF* aof_;

  static std::string join(const std::vector<std::string>& v, size_t from, size_t to_exclusive);

  std::string cmd_ping();
  std::string cmd_get(const std::vector<std::string>& t);
  std::string cmd_del(const std::vector<std::string>& t);
  std::string cmd_set(const std::vector<std::string>& t, const std::string& original_line, bool append_to_aof);
};

}  // namespace minicachedb
