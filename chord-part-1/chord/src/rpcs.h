#ifndef RPCS_H
#define RPCS_H

#include "chord.h"
#include "rpc/client.h"

Node self, successor, predecessor;

Node get_info() { return self; } // Do not modify this line.

void create() {
  predecessor.ip = "";
  successor = self;
}

void join(Node n) {
  predecessor.ip = "";
  rpc::client client(n.ip, n.port);
  successor = client.call("find_successor", self.id).as<Node>();
}

Node find_successor(uint64_t id) {
  // TODO: implement your `find_successor` RPC
  return self;
}

void check_predecessor() {
  try {
    rpc::client client(predecessor.ip, predecessor.port);
    Node n = client.call("get_info").as<Node>();
  } catch (std::exception &e) {
    predecessor.ip = "";
  }
}

void register_rpcs() {
  add_rpc("get_info", &get_info); // Do not modify this line.
  add_rpc("create", &create);
  add_rpc("join", &join);
  add_rpc("find_successor", &find_successor);
}

void register_periodics() {
  add_periodic(check_predecessor);
}

#endif /* RPCS_H */
