#!/usr/bin/env python3
import hashlib
import select
import socket
import struct
import subprocess
import sys
import time

GROUP = "224.0.0.69"
PORT = 4403

if len(sys.argv) < 3:
    raise SystemExit(f"Usage: {sys.argv[0]} <iface1> <iface2> [iface3] [iface4]")
if len(sys.argv) > 5:
    raise SystemExit(
        f"Too many interfaces (max 4). Usage: {sys.argv[0]} <iface1> <iface2> [iface3] [iface4]"
    )

IFACES = sys.argv[1:]


def get_ip(iface):
    out = subprocess.check_output(f"ip -4 addr show dev {iface}", shell=True).decode()
    for line in out.splitlines():
        if "inet " in line:
            return line.split()[1].split("/")[0]
    raise Exception(f"No IPv4 on {iface}")


iface_ips = {iface: get_ip(iface) for iface in IFACES}


def make_recv_sock(iface, iface_ip):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Crucial: bind this socket to a specific interface
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, iface.encode())

    # Bind to group:port
    s.bind((GROUP, PORT))

    # Join multicast group on the specific interface
    mreq = struct.pack("4s4s", socket.inet_aton(GROUP), socket.inet_aton(iface_ip))
    s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    return s


def make_send_sock(iface_ip):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

    # Send interface selection
    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(iface_ip))

    # Do NOT loop back to sender
    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 0)

    return s


print("Binding receivers:")
for iface in IFACES:
    print(f"  • {iface} ({iface_ips[iface]})")

recv_socks = {iface: make_recv_sock(iface, iface_ips[iface]) for iface in IFACES}
send_socks = {iface: make_send_sock(iface_ips[iface]) for iface in IFACES}
sock_to_iface = {sock: iface for iface, sock in recv_socks.items()}

recent = {}
TTL = 0.300

print(f"Multicast bridge active: {' ↔ '.join(IFACES)}")

while True:
    readable, _, _ = select.select(list(recv_socks.values()), [], [])
    for sock in readable:
        data, addr = sock.recvfrom(65535)
        h = hashlib.sha1(data).hexdigest()

        now = time.time()
        if h in recent and (now - recent[h]) < TTL:
            continue
        recent[h] = now

        src_iface = sock_to_iface.get(sock)
        for dst_iface, out_sock in send_socks.items():
            if dst_iface == src_iface:
                continue
            out_sock.sendto(data, (GROUP, PORT))
