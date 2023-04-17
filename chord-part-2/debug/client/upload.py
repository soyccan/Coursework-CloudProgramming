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

filepath = filename
slashs = [i for i, c in list(enumerate(filepath)) if c == '/']
if len(slashs) != 0:
	filename = filename[max(slashs) + 1:]

client = new_client(ip, 5057)
h = hash(filename)
print("Hash of {} is {}".format(filename, h))

node = client.call("find_successor", h)
node_ip = node[0].decode()

files = {
	'files': open(filepath, 'rb'),
}

print("Uploading file to http://{}".format(node_ip))
response = requests.post('http://{}:5058/upload'.format(node_ip), files=files)
