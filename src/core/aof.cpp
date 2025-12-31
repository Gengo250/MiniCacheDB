#include "core/aof.hpp"

#include <fstream>

namespace minicachedb {

AOF::AOF(std::string path) : path_(std::move(path)) {}

void AOF::append_line(const std::string& line) {
  std::lock_guard<std::mutex> lk(m_);
  std::ofstream out(path_, std::ios::app);
  if (!out) return;  // best-effort
  out << line;
  if (line.empty() || line.back() != '\n') out << '\n';
}

std::vector<std::string> AOF::read_all_lines() const {
  std::lock_guard<std::mutex> lk(m_);
  std::ifstream in(path_);
  if (!in) return {};

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (!line.empty()) lines.push_back(line);
  }
  return lines;
}

}  // namespace minicachedb
