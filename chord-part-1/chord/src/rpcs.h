#ifndef RPCS_H
#define RPCS_H

#include "chord.h"

#include "rpc/client.h"
#include "rpc/rpc_error.h"

#include <iostream>
#include <tuple>
#include <utility>

#ifndef NDEBUG
#define _LOG_DEBUG                                                             \
  ([]() -> auto & {                                                            \
    const std::time_t tm = std::chrono::system_clock::to_time_t(               \
        std::chrono::system_clock::now());                                     \
    return std::cout << "[" << std::put_time(std::localtime(&tm), "%T")        \
                     << "] ";                                                  \
  })()
#else
#define _LOG_DEBUG                                                             \
  if (1)                                                                       \
    ;                                                                          \
  else                                                                         \
    std::cout
#endif

// {successor, predecessor}
using NodePair = std::pair<Node, Node>;

Node self, successor, predecessor;

std::ostream &operator<<(std::ostream &os, const Node &n) {
  os << "Node(" << n.ip << ", " << n.port << ", " << n.id << ")";
  return os;
}

// return if n (whose predecessor is pred) is the successor of id
bool is_successor_of(const Node &n, uint64_t id, const Node &pred) {
  return id && !pred.ip.empty() && !n.ip.empty() &&
         (pred.id == n.id                                    // only one node
          || (pred.id > n.id && (id > pred.id || id < n.id)) // last interval
          || (pred.id < id && id <= n.id));
}

Node get_info() { return self; } // Do not modify this line.

NodePair get_neighbors() { return {successor, predecessor}; }

// create a new ring
void create() {
  _LOG_DEBUG << self << ": create ring" << std::endl;

  predecessor.ip.clear();
  successor = self;
}

// join the ring at the ring member
void join(Node member) {
  _LOG_DEBUG << self << ": join at " << member << std::endl;

  try {
    predecessor.ip.clear();
    successor = rpc::client(member.ip, member.port)
                    .call("find_successor", self.id)
                    .as<Node>();

    _LOG_DEBUG << self << ": join and set successor to " << successor
               << std::endl;

  } catch (rpc::rpc_error &e) {
    std::cout << self << ": Error when joining to " << member << ": "
              << e.what() << ". In function " << e.get_function_name()
              << std::endl;
  }
}

Node find_successor(uint64_t id) {
  _LOG_DEBUG << self << ": find successor of " << id << std::endl;

  if (is_successor_of(successor, id, self)) {
    _LOG_DEBUG << self << ": successor of " << id << " is " << successor
               << std::endl;
    return successor;
  }

  try {
    Node suc = rpc::client(successor.ip, successor.port)
                   .call("find_successor", id)
                   .as<Node>();

    _LOG_DEBUG << self << ": successor of " << id << " is " << suc << std::endl;

    return suc;

  } catch (rpc::rpc_error &e) {
    std::cout << self << ": Error when finding successor of id " << id << ": "
              << e.what() << ". In function " << e.get_function_name()
              << std::endl;
    return {};
  }
}

// regular stabilization
// if there is a node between self & successor, let it be the new successor
// also, notify the successor to update its predecessor in case it's missing
// (maybe because its predecessor has left) or need update
void stabilize() {
  if (successor.ip.empty())
    return;

  _LOG_DEBUG << self << ": stabilize" << std::endl;

  try {
    rpc::client succli(successor.ip, successor.port);

    Node sucpred = succli.call("get_neighbors").as<NodePair>().second;
    if (!sucpred.ip.empty() && is_successor_of(successor, sucpred.id, self)) {

      _LOG_DEBUG << self << ": stabilize and set successor to " << sucpred
                 << std::endl;

      rpc::client(sucpred.ip, sucpred.port).call("notify", self);
      successor = std::move(sucpred);

    } else {
      succli.call("notify", self);
    }

  } catch (rpc::rpc_error &e) {
    std::cout << self << ": Error when stabilizing: " << e.what()
              << ". In function " << e.get_function_name() << std::endl;
  }
}

// notify that sender may be its predecessor
void notify(Node sender) {
  _LOG_DEBUG << self << ": notified by " << sender << std::endl;

  assert(!sender.ip.empty());

  if (sender.id == self.id)
    // forbid the predecessor to be self
    // TODO: can we forbid the predecessor to be self here?
    return;

  if (predecessor.ip.empty() || is_successor_of(self, sender.id, predecessor)) {

    _LOG_DEBUG << self << ": notified and set predecessor to " << sender
               << std::endl;

    predecessor = sender;
  }
}

void check_predecessor() {
  if (predecessor.ip.empty())
    return;

  _LOG_DEBUG << self << ": check predecessor" << std::endl;

  try {
    rpc::client client(predecessor.ip, predecessor.port);
    Node n = client.call("get_info").as<Node>();

  } catch (rpc::rpc_error &e) {
    predecessor.ip.clear();

    std::cout << self << ": Error when checking predecessor: " << e.what()
              << ". In function " << e.get_function_name() << std::endl;
  }
}

void register_rpcs() {
  add_rpc("get_info", &get_info); // Do not modify this line.
  add_rpc("get_neighbors", &get_neighbors);
  add_rpc("create", &create);
  add_rpc("join", &join);
  add_rpc("find_successor", &find_successor);
  add_rpc("stabilize", &stabilize);
  add_rpc("notify", &notify);
}

void register_periodics() {
  add_periodic(check_predecessor);
  add_periodic(stabilize);
}

#endif /* RPCS_H */
