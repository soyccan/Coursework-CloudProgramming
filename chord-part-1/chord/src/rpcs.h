#ifndef RPCS_H
#define RPCS_H

#include "chord.h"

#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include <array>
#include <chrono>
#include <iostream>
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

const size_t FINGER_TABLE_SIZE = 4;

Node self, successor, predecessor;
std::array<Node, FINGER_TABLE_SIZE> finger_table;
size_t next_finger = 0;

std::ostream &operator<<(std::ostream &os, const Node &n) {
  os << "Node(" << n.ip << ", " << n.port << ", " << n.id << ")";
  return os;
}

bool operator==(const Node &a, const Node &b) { return a.id == b.id; }

// return if n (whose predecessor is pred) is the successor of id
static inline bool is_successor_of(uint64_t n, uint64_t id, uint64_t pred) {
  return n && id && pred &&
         (pred == n                              // only one node
          || (pred > n && (id > pred || id < n)) // last interval
          || (pred < id && id <= n));
}

// return if n (whose predecessor is pred) is the successor of id
static bool is_successor_of(const Node &n, uint64_t id, const Node &pred) {
  return id && !pred.ip.empty() && !n.ip.empty() &&
         is_successor_of(n.id, id, pred.id);
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

static Node closest_preceding_node(uint64_t id) {
  for (auto n = finger_table.crbegin(); n != finger_table.crend(); ++n) {
    if (!n->ip.empty() && is_successor_of(id, n->id, self.id))
      return *n;
  }
  return self; // TODO: return successor in this case?
}

Node find_successor(uint64_t id) {
  _LOG_DEBUG << self << ": find successor of " << id << std::endl;

  if (is_successor_of(successor, id, self)) {
    _LOG_DEBUG << self << ": successor of " << id << " is " << successor
               << std::endl;
    return successor;
  }

  try {
    Node closest = closest_preceding_node(id);
    if (closest == self)
      // avoid infinite recursion
      return {};

    Node suc = rpc::client(closest.ip, closest.port)
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

// periodically refresh finger table entries
// next_finger stores the index of the next finger to fix
void fix_fingers() {
  next_finger =
      (next_finger + 1) & -(size_t)(next_finger != finger_table.size() - 1);

  assert(next_finger < finger_table.size() && next_finger < 64);

  _LOG_DEBUG << self << ": fix fingers[" << next_finger << "]" << std::endl;

  Node suc = find_successor(self.id + (1 << next_finger));
  if (!suc.ip.empty()) {
    finger_table.at(next_finger) = suc;

    _LOG_DEBUG << self << ": fix fingers[" << next_finger << "] = " << suc
               << std::endl;
  }
}

// periodically check aliveness of the predecessor
void check_predecessor() {
  if (predecessor.ip.empty())
    return;

  _LOG_DEBUG << self << ": check predecessor" << std::endl;

  try {
    rpc::client(predecessor.ip, predecessor.port).call("get_info");

  } catch (rpc::rpc_error &e) {
    predecessor.ip.clear();

    std::cout << self << ": Error when checking predecessor: " << e.what()
              << ". In function " << e.get_function_name() << std::endl;
  }
}

#ifndef NDEBUG
int get_rpc_count() { return rpc_count; }
#endif

void register_rpcs() {
  add_rpc("get_info", &get_info); // Do not modify this line.
  add_rpc("get_neighbors", &get_neighbors);
  add_rpc("create", &create);
  add_rpc("join", &join);
  add_rpc("find_successor", &find_successor);
  add_rpc("notify", &notify);

#ifndef NDEBUG
  add_rpc("get_rpc_count", &get_rpc_count);
#endif
}

void register_periodics() {
  add_periodic(check_predecessor);
  add_periodic(stabilize);
  add_periodic(fix_fingers);
}

#endif /* RPCS_H */
