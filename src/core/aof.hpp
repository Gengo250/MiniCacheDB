#pragma once
#include <mutex>
#include <string>
#include <vector>

namespace minicachedb {

class AOF {
 public:
  explicit AOF(std::string path);

  void append_line(const std::string& line);
  std::vector<std::string> read_all_lines() const;

  const std::string& path() const { return path_; }

 private:
  std::string path_;
  mutable std::mutex m_;
};

}  // namespace minicachedb
