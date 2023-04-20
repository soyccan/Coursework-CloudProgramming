import socket
import os
import msgpackrpc
import time


def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=1)


def in_ring(ip):
    try:
        return bool(new_client(ip, 5057).call("get_successor", 0)[0])
    except:
        return False


def create(ip):
    new_client(ip, 5057).call("create")
    print(f"{ip}: created a ring")


def join(ip, target_ip):
    target_node = new_client(target_ip, 5057).call("get_info")
    if not target_node[0]:
        raise Exception(f"{ip}: fail to ping {target_ip}")
    new_client(ip, 5057).call("join", target_node)
    print(f"{ip}: joined {target_ip}")


CHORD_LEADER_HOSTNAME = "leader.chord.chord.svc"

is_leader = bool(os.environ.get("LEADER"))
self_ip = os.environ["CHORD_IP"]

while True:
    # in case of chord restart, this loop will join a ring again
    time.sleep(1)

    if not in_ring(self_ip):
        try:
            if is_leader:
                create(self_ip)
            else:
                leader_ip = socket.gethostbyname(CHORD_LEADER_HOSTNAME)
                join(self_ip, leader_ip)
        except:
            print(f"{self_ip}: Retry initialization")
            continue

        # expose to cluster for readinessProbe
        open("init.done", "w")
