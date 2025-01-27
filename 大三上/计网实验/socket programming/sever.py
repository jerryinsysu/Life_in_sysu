from socket import *
from threading import Thread

class TcpSever(object):
    def __init__(self,ip,port):
          self.code_mode = 'utf-8'
          self.server_socket = socket(AF_INET, SOCK_STREAM) # 创建TCP socket并设置为TCP连接，并且是IPv4网络格式
          self.server_socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1) # 设置端口复用
          self.server_socket.bind((ip, port)) # 绑定端口
          self.server_socket.listen(10) # 设置最大的连接数
          print("[服务器启动] 等待客户端连接……")  # 服务器启动后打印此消息

    def run(self):
          while True:
              client_socket, client_address = self.server_socket.accept() # 等待客户端进行连接
              print("[连接建立] 客户端 ({},{}) 已连接".format(client_address[0], client_address[1])) # 连接建立后打印此消息
              client_thread = Thread(target=self.recv, args=(client_socket, client_address)) # 创建线程
              client_thread.start() # 启动线程
          self.server_socket.close() # 关闭服务器(可写可不写，因为是不会执行的)

    def recv(self, client_socket, client_address):
         while True:
              try:
                    message = client_socket.recv(1024).decode(self.code_mode)
                    if not message:
                         break
                    else:
                         print("[来自({},{})的消息] {}".format(client_address[0], client_address[1], message))
                         client_socket.send(message.encode(self.code_mode))
              except ConnectionResetError:
                    break
         print("[连接断开] 客户端 ({},{}) 已断开连接".format(client_address[0], client_address[1]))
         client_socket.close()


def main():
     ip = '172.26.43.224'
     port = 12000
     my_server = TcpSever(ip,port)
     my_server.run()

if __name__ == "__main__":
    main()