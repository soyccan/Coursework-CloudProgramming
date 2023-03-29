#!/usr/bin/python3

import random
import msgpackrpc
import time
import sys
import io

ids = []
find_successor_req = 0
incorrect = 0
t = 2

def add_id(id):
	if id not in ids:
		ids.append(id)
		ids.sort()

def new_client(ip, port):
	return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=2)

def get_id(port):
	client = new_client("127.0.0.1", port)
	return client.call("get_info")[2]

def get_neighbors(port):
	client = new_client("127.0.0.1", port)
	return client.call("get_neighbors")

def create(port):
	client = new_client("127.0.0.1", port)
	client.call("create")
	add_id(get_id(port))
	print("node {} created a chord ring".format(port))

def join(port1, port2):
	client1 = new_client("127.0.0.1", port1)
	client2 = new_client("127.0.0.1", port2)
	client1.call("join", client2.call("get_info"))
	add_id(get_id(port1))
	print("node {} joined node {}".format(port1, port2))

def kill(port):
	id = get_id(port)
	ids.remove(id)
	client = new_client("127.0.0.1", port)
	client.call("kill")
	print("node {} killed".format(port))

def get_ans(id):
	if id > ids[-1]:
		return ids[0]
	i = 0
	while ids[i] < id:
		i += 1
	return ids[i]

def find_successor(port, id):
	client = new_client("127.0.0.1", port)
	return client.call("find_successor", id)[2]

def verify(port, id):
	global find_successor_req, incorrect
	find_successor_req += 1
	get = find_successor(port, id)
	if get == get_ans(id):
		print("find_successor({}, {}) correct.".format(port, id))
	else:
		print("find_successor({}, {}) incorrect, ans: {}, get: {}.".format(port, id, get_ans(id), get))
		incorrect += 1

def wait(t):
	print("wait {} sec...".format(t))
	time.sleep(t)

def show_ring():
	for i in range(5057, 5073):
		print(f'neighbors of {i}:', get_neighbors(i))

def get_system_rpc_count():
	res = 0
	for port in range(5057, 5073):
		res += new_client("127.0.0.1", port).call("get_rpc_count")
	return res

# unbuffered
sys.stdout = io.TextIOWrapper(open(sys.stdout.fileno(), 'wb', 0), write_through=True)

create(5057)
wait(t)
join(5059, 5057)
wait(t)
join(5060, 5057)
wait(t)
join(5063, 5059)
wait(t)
join(5062, 5059)
wait(t)
join(5061, 5063)
wait(t)
join(5064, 5061)
wait(t)
join(5058, 5062)
wait(t)
join(5065, 5064)
wait(t)
join(5066, 5060)
wait(t)
join(5067, 5064)
wait(t)
join(5068, 5059)
wait(t)
join(5069, 5063)
wait(t)
join(5070, 5067)
wait(t)
join(5071, 5066)
wait(t)
join(5072, 5071)
wait(10 * t)

show_ring()

rpccnt_init = get_system_rpc_count()
time.sleep(60)
rpccnt_after = get_system_rpc_count()
rpc_per_sec_idle = (rpccnt_after - rpccnt_init) / 60
print(f'Average RPC count per sec during 60s: {rpc_per_sec_idle}')

time_before = time.time()
rpccnt_before = get_system_rpc_count()
for i in range(256):
	port = random.randint(5057, 5072)
	id = random.randint(0, 0xffffffff)
	verify(port, id)
rpccnt_after = get_system_rpc_count()
time_after = time.time()
rpc_per_find = (rpccnt_after - rpccnt_before - rpc_per_sec_idle * (time_after - time_before)) / 256
print(f'Average RPC per find: {rpc_per_find}')

print("{} find successor requests, ".format(find_successor_req), end="")
if (incorrect == 0):
	print("All correct.")
else:
	print("{} incorrect response(s).".format(incorrect))

print("Do not forget to terminate your Chord nodes!")
