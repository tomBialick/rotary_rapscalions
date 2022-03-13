import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5005
MESSAGE = b"Hello, World!"

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

sockSend = socket.socket(socket.AF_INET, # Internet
                       socket.SOCK_DGRAM) # UDP
sockSend.sendto(MESSAGE, (UDP_IP, UDP_PORT))

UDP_IP_REC = "127.0.0.1"
UDP_PORT_REC = 5005

sockRec = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sockRec.bind((UDP_IP_REC, UDP_PORT_REC))

while True:
    data, addr = sockRec.recv(1024) # buffer size is 1024 bytes
    print("received message: %s" % data)
