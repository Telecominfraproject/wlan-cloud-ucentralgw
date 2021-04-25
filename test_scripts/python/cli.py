#!/usr/bin/env python3

import socket
import sys
import os

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

server_address = '/tmp/app.ucentralgw'

try:
    sock.connect(server_address)
except socket.error:
    print >> sys.stderr, 'Could not connect'
    sys.exit(1)

try:
    message = 'set loglevel notice WebSocket'
    sock.sendall(message.encode('utf-8'))

finally:
    sock.close()

