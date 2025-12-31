#pragma once
#include "core/protocol.hpp"
#include "core/thread_pool.hpp"

namespace minicachedb {

class TcpServer {
 public:
  TcpServer(int port, ThreadPool& pool, Protocol& protocol);
  ~TcpServer();

  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;

  void run();  // blocking

 private:
  int port_;
  ThreadPool& pool_;
  Protocol& protocol_;
  int listen_fd_ = -1;

  void setup_listener();
  void handle_client(int client_fd);
};

}  // namespace minicachedb
