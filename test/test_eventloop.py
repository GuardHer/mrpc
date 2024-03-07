import socket
import threading

HOST = '127.0.0.1'
PORT = 12345
NUM_CLIENTS = 10000

sockets = []

def connect_to_server():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.setblocking(0)  # 设置为非阻塞模式
    try:
        client_socket.connect((HOST, PORT))
    except Exception as e:
        pass  # 连接失败，继续执行
    finally:
        client_socket.close()
        # sockets.append(client_socket)

# 创建多个线程来模拟连接
threads = []
for _ in range(NUM_CLIENTS):
    t = threading.Thread(target=connect_to_server)
    threads.append(t)
    t.start()
    
# 等待所有线程完成
for t in threads:
    t.join()

