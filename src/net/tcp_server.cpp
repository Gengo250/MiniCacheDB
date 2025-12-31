#include "net/tcp_server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

namespace minicachedb {

static void die(const char* msg) {
  std::cerr << msg << ": " << std::strerror(errno) << "\n";
  std::exit(1);
}

TcpServer::TcpServer(int port, ThreadPool& pool, Protocol& protocol)
    : port_(port), pool_(pool), protocol_(protocol) {}

TcpServer::~TcpServer() {
  if (listen_fd_ != -1) ::close(listen_fd_);
}

void TcpServer::setup_listener() {
  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  addrinfo* res = nullptr;
  std::string port_str = std::to_string(port_);
  int rc = ::getaddrinfo(nullptr, port_str.c_str(), &hints, &res);
  if (rc != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rc) << "\n";
    std::exit(1);
  }

  for (auto* p = res; p != nullptr; p = p->ai_next) {
    listen_fd_ = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listen_fd_ == -1) continue;

    int yes = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (::bind(listen_fd_, p->ai_addr, p->ai_addrlen) == 0) break;

    ::close(listen_fd_);
    listen_fd_ = -1;
  }

  ::freeaddrinfo(res);

  if (listen_fd_ == -1) die("bind");
  if (::listen(listen_fd_, 128) != 0) die("listen");
}

static bool write_all(int fd, const std::string& s) {
  const char* buf = s.data();
  size_t left = s.size();
  while (left > 0) {
    ssize_t n = ::send(fd, buf, left, 0);
    if (n < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    buf += n;
    left -= static_cast<size_t>(n);
  }
  return true;
}

void TcpServer::handle_client(int client_fd) {
  std::string buffer;
  buffer.reserve(4096);

  char tmp[4096];
  while (true) {
    ssize_t n = ::recv(client_fd, tmp, sizeof(tmp), 0);
    if (n == 0) break;
    if (n < 0) {
      if (errno == EINTR) continue;
      break;
    }

    buffer.append(tmp, tmp + n);

    size_t pos = 0;
    while (true) {
      size_t nl = buffer.find('\n', pos);
      if (nl == std::string::npos) {
        buffer.erase(0, pos);
        break;
      }

      std::string line = buffer.substr(pos, nl - pos);
      pos = nl + 1;

      std::string resp = protocol_.execute_line(line);
      if (!write_all(client_fd, resp)) {
        ::close(client_fd);
        return;
      }

      auto t = Protocol::split_ws(line);
      if (!t.empty()) {
        std::string cmd = t[0];
        for (auto& c : cmd) c = (char)std::toupper((unsigned char)c);
        if (cmd == "QUIT") {
          ::close(client_fd);
          return;
        }
      }
    }
  }

  ::close(client_fd);
}

void TcpServer::run() {
  setup_listener();

  while (true) {
    sockaddr_storage addr{};
    socklen_t len = sizeof(addr);
    int client_fd = ::accept(listen_fd_, (sockaddr*)&addr, &len);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      die("accept");
    }

    // Note: one connection occupies a worker thread (good enough for MVP).
    pool_.submit([this, client_fd]() { handle_client(client_fd); });
  }
}

}  // namespace minicachedb
