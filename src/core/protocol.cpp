#include "core/protocol.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace minicachedb {

Protocol::Protocol(KVStore& store, AOF* aof) : store_(store), aof_(aof) {}

static std::string upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return (char)std::toupper(c); });
  return s;
}

std::vector<std::string> Protocol::split_ws(const std::string& s) {
  std::istringstream iss(s);
  std::vector<std::string> out;
  std::string tok;
  while (iss >> tok) out.push_back(tok);
  return out;
}

std::string Protocol::join(const std::vector<std::string>& v, size_t from, size_t to_exclusive) {
  std::string out;
  for (size_t i = from; i < to_exclusive; i++) {
    if (i > from) out.push_back(' ');
    out += v[i];
  }
  return out;
}

std::string Protocol::cmd_ping() { return "OK\n"; }

std::string Protocol::cmd_get(const std::vector<std::string>& t) {
  if (t.size() != 2) return "ERR usage: GET <key>\n";
  auto v = store_.get(t[1]);
  if (!v.has_value()) return "(nil)\n";
  return *v + "\n";
}

std::string Protocol::cmd_del(const std::vector<std::string>& t) {
  if (t.size() != 2) return "ERR usage: DEL <key>\n";
  bool ok = store_.del(t[1]);
  return ok ? "OK\n" : "(nil)\n";
}

std::string Protocol::cmd_set(const std::vector<std::string>& t, const std::string& original_line,
                              bool append_to_aof) {
  if (t.size() < 3) return "ERR usage: SET <key> <value...> [EX <seconds>]\n";

  std::optional<int> ttl = std::nullopt;
  size_t value_end = t.size();

  // Support: SET key value... EX N (value may contain spaces)
  if (t.size() >= 5 && upper(t[t.size() - 2]) == "EX") {
    try {
      ttl = std::stoi(t[t.size() - 1]);
      value_end = t.size() - 2;
    } catch (...) {
      return "ERR invalid EX seconds\n";
    }
  }

  std::string key = t[1];
  std::string value = join(t, 2, value_end);

  store_.set(std::move(key), std::move(value), ttl);

  if (append_to_aof && aof_) aof_->append_line(original_line);
  return "OK\n";
}

std::string Protocol::execute_line(const std::string& line, bool append_to_aof) {
  std::string s = line;
  if (!s.empty() && s.back() == '\r') s.pop_back();

  auto t = split_ws(s);
  if (t.empty()) return "ERR empty\n";

  auto c = upper(t[0]);

  if (c == "PING") return cmd_ping();
  if (c == "GET") return cmd_get(t);
  if (c == "DEL") return cmd_del(t);
  if (c == "SET") return cmd_set(t, s, append_to_aof);
  if (c == "QUIT") return "OK\n";

  return "ERR unknown command\n";
}

}  // namespace minicachedb
