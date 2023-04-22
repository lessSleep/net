#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 5000
#define BUF_SIZE 1024
#define MAXFDS 5000

#define BUFFER                                                                  \
  "HTTP/1.1 200 OK\r\nContent-Length: 102\r\nConnection: "                      \
  "close\r\nContent-Type: text/html;charset=utf-8\r\n\r\n <html> 响应数据 " \
  "<head> <title>HTTP响应示例<title> </head>  <body> Hello HTTP! </body> "  \
  "</html>"

void setnonblocking(int fd) {
  int opts;
  opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    fprintf(stderr, "fcntl failed/n");
    return;
  }
  opts = opts | O_NONBLOCK;
  if (fcntl(fd, F_SETFL, opts) < 0) {
    fprintf(stderr, "fcntl failed/n");
    return;
  }
  return;
}

int main() {
  int server_fd, client_fd, epoll_fd, n;
  struct sockaddr_in server_addr {}, client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  char buf[BUF_SIZE];

  // create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create socket\n";
    return 1;
  }

  // zl add
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt,
             sizeof(opt));
  memset(&server_addr, 0, sizeof(struct sockaddr_in));

  // set server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(12345);

  // bind socket to address
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    std::cerr << "Failed to bind socket to address\n";
    return 1;
  }

  // listen for incoming connections
  if (listen(server_fd, 5) < 0) {
    std::cerr << "Failed to listen for incoming connections\n";
    return 1;
  }

  // create epoll instance
  // epoll_fd = epoll_create1(0);
  epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    std::cerr << "Failed to create epoll instance\n";
    return 1;
  }

  // add server_fd to epoll instance
  struct epoll_event event {}, events[MAX_EVENTS];
  event.events = EPOLLIN;
  event.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
    std::cerr << "Failed to add server_fd to epoll instance\n";
    return 1;
  }

  while (true) {
    // wait for events
    n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (n < 0) {
      std::cerr << "Failed to wait for events\n";
      return 1;
    }

    // handle events
    for (int i = 0; i < n; i++) {

      if (events[i].data.fd == server_fd) {

        // handle new connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           &client_addr_len);
        if (client_fd < 0) {
          std::cerr << "Failed to accept new connection\n";
          return 1;
        }

        // set client_fd to non-blocking mode
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        // add client_fd to epoll instance
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = client_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
          std::cerr << "Failed to add client_fd to epoll instance\n";
          return 1;
        }

      } else {

        struct epoll_event ev;
        int client_fd = events[i].data.fd;
        if (events[i].events & EPOLLIN) {
          // handle data from client

          int len = read(client_fd, buf, BUF_SIZE);

          if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              // no more data to read
              continue;
            } else {
              std::cerr << "Failed to read data from client\n";
              return 1;
            }
          } else if (len == 0) {
            // client closed connection
            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr) < 0) {
              std::cerr << "Failed to remove client_fd from epoll instance\n";
              return 1;
            }
            close(client_fd);
          } else {
            ev.data.fd = client_fd;
            ev.events = EPOLLOUT | EPOLLET;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
            // std::cout <<  " event \n";
          }

        } else if (events[i].events & EPOLLOUT) {
          // echo data back to client
          if (write(client_fd, BUFFER, strlen(BUFFER)) < 0) {
            std::cerr << "Failed to write data to client\n";
            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr) < 0) {
              std::cerr << "Failed to remove client_fd from epoll instance\n";
              return 1;
            }
            return 1;
          }
          close(client_fd);
        }
      }
    }
  }

  return 0;
}

// g++ server-epoll-main-ab.cpp -O3
// ab -n 1 -t 10 -c 1  http://127.0.0.1:12345/index.html
// Concurrency Level:      1
// Time taken for tests:   6.497 seconds
// Complete requests:      50000
// Failed requests:        0
// Total transferred:      10000000 bytes
// HTML transferred:       5100000 bytes
// Requests per second:    7696.02 [#/sec] (mean)
// Time per request:       0.130 [ms] (mean)
// Time per request:       0.130 [ms] (mean, across all concurrent requests)
// Transfer rate:          1503.13 [Kbytes/sec] received

// Concurrency Level:      4
// Time taken for tests:   4.266 seconds
// Complete requests:      50000
// Failed requests:        0
// Total transferred:      10000000 bytes
// HTML transferred:       5100000 bytes
// Requests per second:    11719.42 [#/sec] (mean)
// Time per request:       0.341 [ms] (mean)
// Time per request:       0.085 [ms] (mean, across all concurrent requests)
// Transfer rate:          2288.95 [Kbytes/sec] received