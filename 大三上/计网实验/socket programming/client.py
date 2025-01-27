from socket import *
serverName = '172.26.43.224'
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName,serverPort))
print("[连接到服务器] 你可以开始发送消息了(输入'exit' 退出)")
while True:
    message = input()
    if message.lower() == 'exit':
        print("[连接断开] 客户端已断开连接")
        clientSocket.close()
        break
    # 发送消息
    clientSocket.send(message.encode('utf-8'))
    print(f"你：{message}")
    #modifiedSentence = clientSocket.recv(1024) 
    