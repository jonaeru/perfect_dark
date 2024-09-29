#!/usr/bin/env python3

# simple client for server status queries

import sys, os, socket, struct, selectors

DEFAULT_PORT = 27100
QUERY_MAGIC = b"PDQM\x01"
MAX_WAIT = 5.0

def checksum(data):
  crc = 0xFFFF
  for b in data[:-2]:
    x = crc >> 8 ^ b
    x ^= x >> 4
    crc += (crc << 8) ^ (x << 12) ^ (x << 5) ^ x
    crc &= 0xFFFF
  return crc

def eat_string(data):
  strlen = struct.unpack_from("<H", data)[0]
  strdata = data[2:strlen + 2]
  return strdata[:-1].decode(encoding='utf-8'), data[2 + strlen:]

if len(sys.argv) < 2:
  print("usage: query <address>[:<port>]")
  sys.exit(1)

addrstr = sys.argv[1].strip()
host = addrstr
port = None

if addrstr.startswith('[') and ("]:" in addrstr):
  # [ipv6]:port
  host, sep, port = addrstr.rpartition(':')
elif addrstr.count(':') == 1:
  # possibly ipv4:port or hostname:port
  host, sep, port = addrstr.rpartition(':')

host = host.strip('[]')

if port != None:
  port = int(port)
else:
  port = DEFAULT_PORT

sockfam = socket.AF_INET
if ':' in host:
  sockfam = socket.AF_INET6

sel = selectors.DefaultSelector()
sock = socket.socket(sockfam, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
sock.settimeout(MAX_WAIT)
sel.register(sock, selectors.EVENT_READ, None)

# send query magic to server(s)
sock.sendto(QUERY_MAGIC, (host, port))

while True:
  # wait for response
  events = sel.select(1.0)  # timeout in seconds
  if not events:
    break

  for (key, mask) in events:
    data, from_addr = sock.recvfrom(256)

    # check magic
    if data[:5] != QUERY_MAGIC:
      print("invalid magic: expected", QUERY_MAGIC, "got", data[:5])
      sys.exit(1)

    # check checksum
    chkremote = struct.unpack("<H", data[-2:])[0]
    chklocal = checksum(data)
    if chkremote != chklocal:
      print("invalid checksum: expected", chklocal, "got", chkremote)
      sys.exit(1)

    # check size
    datalen = struct.unpack("<H", data[5:7])[0]
    if datalen > len(data):
      print("invalid size: expected", len(data), "got", datalen)
      sys.exit(1)

    data = data[7:datalen - 2]

    # unpack fixed size part of the response
    msgdata = struct.unpack_from("<LBBBBB", data)
    # unpack strings from the end of the response
    hostname, data = eat_string(data[9:])
    romname, data = eat_string(data)
    moddir, data = eat_string(data)

    print("address:", from_addr)
    print("protocol ver:", msgdata[0])
    print("in progress:", msgdata[1])
    print("clients: {0}/{1}".format(msgdata[2], msgdata[3]))
    print("stage num:", hex(msgdata[4]))
    print("scenario:", msgdata[5])
    print("host name:", hostname)
    print("rom name:", romname)
    print("mod dir:", moddir)
    print("-"*40)

