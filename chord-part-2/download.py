#!/usr/bin/python3

import sys
import requests
import msgpackrpc
import hashlib

def new_client(ip, port):
	return msgpackrpc.Client(msgpackrpc.Address(ip, port))

def hash(str):
	return int(hashlib.md5(str.encode()).hexdigest(), 16) & ((1 << 32) - 1)

filename = sys.argv[1]
ip = sys.argv[2]

client = new_client(ip, 5057)
h = hash(filename)
print("Hash of {} is {}".format(filename, h))

node = client.call("find_successor", h)
node_ip = node[0].decode()

print("Downloading file from http://{}".format(node_ip))
response = requests.get("http://{}:5058/{}".format(node_ip, filename))

with open(filename, "wb") as f:
	f.write(response.content)
