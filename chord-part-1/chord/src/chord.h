#ifndef CHORD_H
#define CHORD_H

#include <rpc/server.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

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
std::atomic_bool terminated{false}, ready_to_exit{false};
std::atomic_int64_t rpc_count{0};
std::mutex mutex;
std::condition_variable condvar;
uint64_t interval = 2000;

template <typename F> void add_periodic(F func) {
  periodics.emplace_back([func = func]() {
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

// for func being a lambda closure
template <typename F> void add_rpc(std::string const &name, F func) {
  add_rpc(name, &F::operator());
}

template <typename R, typename... Args>
void add_rpc(std::string const &name, R (*func)(Args...)) {
  server_p->bind(name, [func = func](Args... args) {
    rpc_count++;
    return func(std::forward<Args...>(args)...);
  });
}

#endif /* CHORD_H */
