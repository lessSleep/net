#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345
#define MAXFDS 256
#define EVENTSIZE 100

#define BUFFER "HTTP/1.1 200 OK\r\nContent-Length: 102\r\nConnection: close\r\nContent-Type: text/html;charset=utf-8\r\n\r\n <html> 响应数据 <head> <title>HTTP响应示例<title> </head>  <body> Hello HTTP! </body> </html>"

int epfd;
void *serv_epoll(void *p);
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

int main(int argc, char *argv[]) {
  int fd, cfd, opt = 1;
  struct epoll_event ev;
  struct sockaddr_in sin, cin;
  socklen_t sin_len = sizeof(struct sockaddr_in);
  pthread_t tid;
  pthread_attr_t attr;

  epfd = epoll_create(MAXFDS);
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
    fprintf(stderr, "socket failed/n");
    return -1;
  }

// 设置socket，不用看的代码
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons((short)(PORT));
  sin.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
    fprintf(stderr, "bind failed/n");
    return -1;
  }
  if (listen(fd, 32) != 0) {
    fprintf(stderr, "listen failed/n");
    return -1;
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&tid, &attr, serv_epoll, NULL) != 0) {
    fprintf(stderr, "pthread_create failed/n");
    return -1;
  }

  while ((cfd = accept(fd, (struct sockaddr *)&cin, &sin_len)) > 0) {
    setnonblocking(cfd);
    ev.data.fd = cfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    // printf("connect from %s/n",inet_ntoa(cin.sin_addr));
    // printf("cfd=%d/n",cfd);
  }

  if (fd > 0)
    close(fd);
  return 0;
}

void *serv_epoll(void *p) {
  int i, ret, cfd, nfds;
  ;
  struct epoll_event ev, events[EVENTSIZE];
  char buffer[512];

  while (1) {
    nfds = epoll_wait(epfd, events, EVENTSIZE, -1);
    // printf("nfds ........... %d/n",nfds);
    for (i = 0; i < nfds; i++) {
      if (events[i].events & EPOLLIN) {
        cfd = events[i].data.fd;
        ret = recv(cfd, buffer, sizeof(buffer), 0);
        // printf("read ret..........= %d/n",ret);

        ev.data.fd = cfd;
        ev.events = EPOLLOUT | EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_MOD, cfd, &ev);
      } else if (events[i].events & EPOLLOUT) {
        cfd = events[i].data.fd;
        ret = send(cfd, BUFFER, strlen(BUFFER), 0);
        // printf("send ret...........= %d/n", ret);

        ev.data.fd = cfd;
        epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
        // shutdown(cfd, 1);
        close(cfd);
      }
    }
  }
  return NULL;
}
// g++ server-epoll-ab.cpp -O3 -lpthread
// Concurrency Level:      4
// Time taken for tests:   5.017 seconds
// Complete requests:      50000
// Failed requests:        0
// Total transferred:      10000000 bytes
// HTML transferred:       5100000 bytes
// Requests per second:    9965.29 [#/sec] (mean)
// Time per request:       0.401 [ms] (mean)
// Time per request:       0.100 [ms] (mean, across all concurrent requests)
// Transfer rate:          1946.35 [Kbytes/sec] received

// Concurrency Level:      4
// Time taken for tests:   4.699 seconds
// Complete requests:      50000
// Failed requests:        0
// Total transferred:      10000000 bytes
// HTML transferred:       5100000 bytes
// Requests per second:    10640.99 [#/sec] (mean)
// Time per request:       0.376 [ms] (mean)
// Time per request:       0.094 [ms] (mean, across all concurrent requests)
// Transfer rate:          2078.32 [Kbytes/sec] received