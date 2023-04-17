"""
Debug utils.
Usage:
    $ kubectl exec <pod> -c init -it -- python
    >>> from _dbg import *
"""

import msgpackrpc
import socket
import hashlib


def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=1)


def hash(str):
    return int(hashlib.md5(str.encode()).hexdigest(), 16) & ((1 << 32) - 1)


def create(ip):
    new_client(ip, 5057).call("create")


def join(ip1, ip2):
    new_client(ip1, 5057).call("join", new_client(ip2, 5057).call("get_info"))


def kill(ip):
    new_client(ip, 5057).call("kill")


def find_successor(ip, id):
    return new_client(ip, 5057).call("find_successor", id)


def get_successor(ip, i):
    return new_client(ip, 5057).call("get_successor", i)


def get_predecessor(ip):
    return new_client(ip, 5057).call("get_predecessor")


def show_ring(ip):
    ip = socket.gethostbyname(ip)
    start = ip
    for _ in range(32):
        pred = new_client(ip, 5057).call("get_predecessor")[0].decode()
        nxt = new_client(ip, 5057).call("get_successor", 0)[0].decode()
        print(f"{pred} -> {ip} -> {nxt}")
        ip = nxt
        if nxt == start:
            break


def get_info(ip):
    return new_client(ip, 5057).call("get_info")
