#ifndef CHORD_H
#define CHORD_H

#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include "rpc/client.h"
#include "rpc/server.h"

struct Node {
  std::string ip;
  uint32_t port;
  uint64_t id;
  MSGPACK_DEFINE_ARRAY(ip, port, id)
};

uint64_t hash(Node const &n) {
  uint64_t h1 = std::hash<std::string>{}(n.ip);
  uint64_t h2 = std::hash<std::string>{}(std::to_string(n.port));
  return (h1 ^ h2) & ((1UL << 32) - 1);
}

std::vector<std::thread> periodics;
std::atomic_bool terminated = false, ready_to_exit = false;
uint64_t interval = 2000;

void add_periodic(std::function<void(void)> func) {
  periodics.emplace_back([func]() {
    while (true) {
      if (terminated) {
        terminated = false;
        break;
      }
      auto x = std::chrono::steady_clock::now() +
               std::chrono::milliseconds(interval);
      func();
      std::this_thread::sleep_until(x);
    }
  });
}

std::unique_ptr<rpc::server> server_p;

template <typename F>
void add_rpc(std::string const &name, F func) {
  server_p->bind(name, func);
}

#endif /* CHORD_H */
