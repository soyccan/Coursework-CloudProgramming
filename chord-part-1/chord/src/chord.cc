#include "chord.h"

#include "rpcs.h"

#include <rpc/server.h>

int main(int argc, char *argv[]) {
  std::string ip;
  uint32_t port;

  if (argc >= 3) {
    ip = std::string(argv[1]);
    port = atoi(argv[2]);
    if (argc == 4) {
      interval = atol(argv[3]);
    }
  } else {
    std::cout << "usage: ./chord <ip> <port>\n";
    return 1;
  }

  self.ip = ip;
  self.port = port;
  self.id = hash(self);
  assert(self.id == hash(self));

  server_p = std::make_unique<rpc::server>(port);

  register_rpcs();
  server_p->async_run(1);
  register_periodics();

  server_p->bind("kill", []() {
    for (size_t i = 0; i < periodics.size(); i++) {
      _LOG_DEBUG << "sending termination signal to periodic #" << i << std::endl;
      terminated.store(true);

      _LOG_DEBUG << "waiting for termination of periodic #" << i << std::endl;
      terminated.wait(true);
    }
    for (auto &p : periodics) {
      p.join();
    }
    ready_to_exit.test_and_set();
    ready_to_exit.notify_one();
  });

  server_p->bind("get_rpc_count", []() { return rpc_count.load(); });

  ready_to_exit.wait(false);

  std::this_thread::sleep_until(std::chrono::steady_clock::now() +
                                std::chrono::milliseconds(3000));
  return 0;
}
