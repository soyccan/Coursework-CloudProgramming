import socket
import os
import msgpackrpc
import time
import signal


def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=1)


is_leader = bool(os.environ.get("LEADER"))
self_ip = os.environ["CHORD_IP"]
initialized = False

while not initialized:
    time.sleep(1)

    try:
        if is_leader:
            new_client(self_ip, 5057).call("create")
            print(f"{self_ip}: created a ring")
        else:
            leader_ip = socket.gethostbyname("leader.chord")
            leader_node = new_client(leader_ip, 5057).call("get_info")
            new_client(self_ip, 5057).call("join", leader_node)
            print(f"{self_ip}: joined {leader_ip}")
    except:
        print(f"{self_ip}: Retry initialization")
        continue

    initialized = True
    open("init.done", "w") # expose to cluster

signal.pause()
