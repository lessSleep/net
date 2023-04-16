#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio> //printf
#include <arpa/inet.h> //inet_addr
int main() {
    // 创建 socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 连接服务器
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(12345);
    connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 发送消息给服务器
    const char* message = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);

    // 接收服务器返回的消息
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    // 关闭 socket
    close(clientSocket);

    return 0;
}