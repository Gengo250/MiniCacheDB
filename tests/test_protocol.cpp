#include "core/aof.hpp"
#include "core/kv_store.hpp"
#include "core/protocol.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace minicachedb;

int main() {
  std::filesystem::path p = std::filesystem::temp_directory_path() / "minicachedb_test_aof.log";
  std::error_code ec;
  std::filesystem::remove(p, ec);

  KVStore store;
  AOF aof(p.string());
  Protocol proto(store, &aof);

  assert(proto.execute_line("PING") == "OK\n");
  assert(proto.execute_line("SET x hello") == "OK\n");
  assert(proto.execute_line("GET x") == "hello\n");
  assert(proto.execute_line("DEL x") == "OK\n");
  assert(proto.execute_line("GET x") == "(nil)\n");

  auto lines = aof.read_all_lines();
  assert(!lines.empty());

  std::filesystem::remove(p, ec);
  std::cout << "test_protocol OK\n";
  return 0;
}
