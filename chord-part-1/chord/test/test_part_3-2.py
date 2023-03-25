#!/usr/bin/python3

import msgpackrpc
import time

ids = []
find_successor_req = 0
incorrect = 0
t = 2

def add_id(id):
	if id not in ids:
		ids.append(id)
		ids.sort()

def new_client(ip, port):
	return msgpackrpc.Client(msgpackrpc.Address(ip, port))

def get_id(port):
	client = new_client("127.0.0.1", port)
	return client.call("get_info")[2]

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

create(5057)
wait(t)

join(5058, 5057)
wait(t)
join(5059, 5058)
wait(t)
join(5060, 5059)
wait(t)
join(5061, 5060)
wait(t)
join(5062, 5061)
wait(t)
join(5063, 5062)
wait(t)
join(5064, 5063)
wait(10 * t)

kill(5062)
kill(5058)
wait(20 * t)

stride = (1 << 32) // 128
testcases = [39, 51, 77, 108]

for case in testcases:
	id = stride * case

	for port in [5057, 5059, 5061, 5064, 5063, 5060]:
		verify(port, id)
		wait(t)

kill(5063)
kill(5064)
wait(20 * t)

stride = (1 << 32) // 128
testcases = [1, 2, 111, 112]

for case in testcases:
	id = stride * case

	for port in [5057, 5060, 5061, 5059]:
		verify(port, id)
		wait(t)

print("{} find successor requests, ".format(find_successor_req), end="")
if (incorrect == 0):
	print("All correct.")
else:
	print("{} incorrect response(s).".format(incorrect))

print("Do not forget to terminate your Chord nodes!")
