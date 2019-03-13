import socket
import datetime
import time

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

serverSocket.bind(("224.0.0.1", 9099))

print('Start!')

while True:
    buff = serverSocket.recv(1024)
    log_time = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
    print('['+log_time+']' + buff)