#include "net/tcp_server.hpp"
#include "core/aof.hpp"
#include "core/kv_store.hpp"
#include "core/protocol.hpp"
#include "core/thread_pool.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

struct Args {
  int port = 6379;
  std::string aof_path = "data/aof.log";
  int workers = 4;
};

static void usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [--port N] [--aof PATH] [--workers N]\n";
}

static Args parse_args(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; i++) {
    std::string s = argv[i];
    auto need = [&](const char* flag) {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for " << flag << "\n";
        usage(argv[0]);
        std::exit(2);
      }
      return std::string(argv[++i]);
    };

    if (s == "--port") {
      a.port = std::stoi(need("--port"));
    } else if (s == "--aof") {
      a.aof_path = need("--aof");
    } else if (s == "--workers") {
      a.workers = std::stoi(need("--workers"));
    } else if (s == "--help" || s == "-h") {
      usage(argv[0]);
      std::exit(0);
    } else {
      std::cerr << "Unknown arg: " << s << "\n";
      usage(argv[0]);
      std::exit(2);
    }
  }
  return a;
}

int main(int argc, char** argv) {
  Args args = parse_args(argc, argv);

  std::filesystem::create_directories(std::filesystem::path(args.aof_path).parent_path());

  minicachedb::KVStore store;
  minicachedb::AOF aof(args.aof_path);

  // Replay AOF on boot (best-effort)
  {
    auto lines = aof.read_all_lines();
    minicachedb::Protocol proto(store, &aof);
    for (const auto& line : lines) {
      proto.execute_line(line, /*append_to_aof=*/false);
    }
    std::cout << "[boot] replayed " << lines.size() << " AOF entries\n";
  }

  minicachedb::ThreadPool pool(args.workers);
  minicachedb::Protocol protocol(store, &aof);

  minicachedb::TcpServer server(args.port, pool, protocol);
  std::cout << "MiniCacheDB listening on 0.0.0.0:" << args.port << "\n";
  std::cout << "Workers: " << args.workers << " | AOF: " << args.aof_path << "\n";

  server.run();
  return 0;
}
