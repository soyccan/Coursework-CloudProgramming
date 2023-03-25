#include "chord.h"

#include <iostream>

#include "rpc/client.h"
#include "rpc/server.h"
#include "rpcs.h"

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
      terminated = true;
      while (terminated)
        ;
    }
    for (auto &p : periodics) {
      p.join();
    }
    ready_to_exit = true;
  });

  while (!ready_to_exit)
    ;

  std::this_thread::sleep_until(std::chrono::steady_clock::now() +
                                std::chrono::milliseconds(3000));
  return 0;
}
