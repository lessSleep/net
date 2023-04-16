#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include <sys/poll.h>
#include <cstdio>

#define BUFFER "HTTP/1.1 200 OK\r\nContent-Length: 102\r\nConnection: close\r\nContent-Type: text/html;charset=utf-8\r\n\r\n <html> 响应数据 <head> <title>HTTP响应示例<title> </head>  <body> Hello HTTP! </body> </html>"
int main() {
    // 创建 socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定 socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(12345);
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 监听 socket
    listen(serverSocket, 10);

    // 接收 socket 请求
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    struct pollfd fds[1];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;
    while (true) {
        poll(fds, 1, -1);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);


        // 接收客户端发送的数据
        char buffer[1024];
        recv(clientSocket, buffer, sizeof(buffer), 0);
        //printf("%s\n", buffer);

        // 返回给客户端一个字符串
        // const char* message = "Hello, client!";
        send(clientSocket, BUFFER, strlen(BUFFER), 0);

        // 关闭 socket
        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}
