"""
Debug utils.
Usage:
    $ kubectl exec <pod> -c init -it -- python
    >>> from dbgutils import *
"""

import msgpackrpc
import socket


def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=1)


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
