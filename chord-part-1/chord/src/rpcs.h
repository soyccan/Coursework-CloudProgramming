#ifndef RPCS_H
#define RPCS_H

#include "chord.h"

#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include <array>
#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <thread>
#include <type_traits>
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
  if (true)                                                                    \
    ;                                                                          \
  else                                                                         \
    std::cout
#endif

// {successor, predecessor}
using NodePair = std::pair<Node, Node>;

constexpr size_t N_FIND_RETRIAL = 2;
constexpr size_t ID_NBITS = 32;
constexpr size_t FINGER_TABLE_SIZE = 4;

// TODO: add a mutex to protect sucessor, predecessor & finger table
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

  predecessor.ip.clear();

  try {
    auto suc = rpc::client(member.ip, member.port)
                   .call("find_successor", self.id)
                   .as<Node>();
    if (suc.ip.empty())
      return;

    successor = std::move(suc);

    _LOG_DEBUG << self << ": join and set successor to " << successor
               << std::endl;

  } catch (const rpc::rpc_error &e) {
    std::cout << self << ": Error when joining to " << member << ": "
              << e.what() << ". In function " << e.get_function_name()
              << std::endl;

  } catch (const std::exception &e) {
    std::cout << self << ": Error when joining to " << member << ": "
              << e.what() << std::endl;
  }
}

static Node &closest_preceding_node(uint64_t id) {
  for (auto n = finger_table.rbegin(); n != finger_table.rend(); ++n) {
    if (!n->ip.empty() && is_successor_of(id, n->id, self.id))
      return *n;
  }
  return successor.ip.empty() ? self : successor;
}

Node find_successor(uint64_t id);

static void handle_node_dead(Node &node, const std::exception &error) {
  try {
    auto &e = dynamic_cast<const std::system_error &>(error);
    if (e.code().value() == ECONNREFUSED) {
      // if connection is refused, we think the node is dead
      // TODO: the error code may depend on operating system?
      _LOG_DEBUG << self << ": " << node << " seems to be dead. Removing."
                 << std::endl;
      if (node == successor) {
        // if successor is dead, we need to find a new one
        // & remove relevant finger table entries
        successor = self; // should not set to none, or may be seen
                          // uninitialized (not in the ring)
        for (auto &entry : finger_table)
          if (entry == node)
            entry.ip.clear();

        if (auto suc = find_successor(self.id); !suc.ip.empty())
          successor = std::move(suc);
      } else {
        node.ip.clear();
      }
    }
  } catch (const std::bad_cast &) {
  }
}

Node find_successor(uint64_t id) {
  _LOG_DEBUG << self << ": find successor of " << id << std::endl;

  for (size_t tries = 0; tries < N_FIND_RETRIAL; tries++) {
    if (is_successor_of(successor, id, self)) {
      _LOG_DEBUG << self << ": successor of " << id << " is " << successor
                 << std::endl;
      return successor;
    }

    for (size_t i = 0; i < finger_table.size(); i++) {
      Node &closest = closest_preceding_node(id);

      _LOG_DEBUG << self << ": closest node of " << id << " is " << closest
                 << std::endl;

      if (closest == self)
        // avoid infinite recursion
        break;

      try {
        Node suc = rpc::client(closest.ip, closest.port)
                       .call("find_successor", id)
                       .as<Node>();

        _LOG_DEBUG << self << ": successor of " << id << " is " << suc
                   << std::endl;

        return suc;

      } catch (const rpc::rpc_error &e) {
        std::cout << self << ": Error when finding successor of id " << id
                  << ": " << e.what() << ". In function "
                  << e.get_function_name() << std::endl;
        return {};

      } catch (const std::exception &e) {
        std::cout << self << ": Error when finding successor of id " << id
                  << ": " << e.what() << std::endl;
        handle_node_dead(closest, e); // remove the entry
        continue;                     // try another entry
      }
    }
    // retry in case of successor dead
    _LOG_DEBUG << self << ": find successor of " << id << " retry #" << tries
               << std::endl;
  }
  return {};
}

// regular stabilization
// if there is a node between self & successor, let it be the new successor
// also, notify the successor to update its predecessor in case it's missing
// (maybe because its predecessor has left) or need update
void stabilize() {
  if (successor.ip.empty())
    return;

  _LOG_DEBUG << self << ": stabilize" << std::endl;

  std::unique_ptr<rpc::client> succli;
  try {
    succli = std::make_unique<rpc::client>(successor.ip, successor.port);

  } catch (const rpc::rpc_error &e) {
    std::cout << self << ": Error when stabilizing: " << e.what()
              << ". In function " << e.get_function_name() << std::endl;
    return;

  } catch (const std::exception &e) {
    std::cout << self << ": Error when stabilizing: " << e.what() << std::endl;
    handle_node_dead(successor, e);
    return;
  }

  try {
    Node sucpred = succli->call("get_neighbors").as<NodePair>().second;
    if (!sucpred.ip.empty() && is_successor_of(successor, sucpred.id, self)) {

      _LOG_DEBUG << self << ": stabilize and set successor to " << sucpred
                 << std::endl;

      rpc::client(sucpred.ip, sucpred.port).call("notify", self);
      successor = std::move(sucpred);

    } else {
      succli->call("notify", self);
    }

  } catch (const rpc::rpc_error &e) {
    std::cout << self << ": Error when stabilizing : " << e.what()
              << ". In function " << e.get_function_name() << std::endl;

  } catch (const std::exception &e) {
    std::cout << self << ": Error when stabilizing : " << e.what() << std::endl;
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

static auto increment_with_wrap = [](auto &value, auto maximum) {
  // branchless equivalence of:
  //   return value = value == maximum ? 0 : value + 1;
  return value = (value + 1) &
                 -static_cast<std::remove_reference_t<decltype(value)>>(
                     value != maximum);
};

// periodically refresh finger table entries
// next_finger stores the index of the next finger to fix
void fix_fingers() {
  increment_with_wrap(next_finger, finger_table.size() - 1);
  assert(next_finger < finger_table.size());

  _LOG_DEBUG << self << ": fix fingers[" << next_finger << "]" << std::endl;

  // a finger table step is 2^ID_NBITS / 2^FINGER_TABLE_SIZE
  constexpr size_t finger_table_step_log = ID_NBITS - finger_table.size();
  assert(next_finger + finger_table_step_log <
         std::numeric_limits<size_t>::digits);
  Node suc =
      find_successor(self.id + (static_cast<size_t>(1)
                                << (next_finger + finger_table_step_log)));
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

  } catch (const rpc::rpc_error &e) {
    std::cout << self << ": Error when checking predecessor: " << e.what()
              << ". In function " << e.get_function_name() << std::endl;

  } catch (const std::exception &e) {
    std::cout << self << ": Error when checking predecessor: " << e.what()
              << std::endl;
    handle_node_dead(predecessor, e);
  }
}

void register_rpcs() {
  add_rpc("get_info", &get_info); // Do not modify this line.
  add_rpc("get_neighbors", &get_neighbors);
  add_rpc("create", &create);
  add_rpc("join", &join);
  add_rpc("find_successor", &find_successor);
  add_rpc("notify", &notify);
}

void register_periodics() {
  add_periodic(check_predecessor);
  add_periodic(stabilize);
  add_periodic(fix_fingers);
}

#endif /* RPCS_H */
