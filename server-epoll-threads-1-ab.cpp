/* conclusion, in this style, the connecting event been sent to one thread */

/* gcc -lpthread epoll-demo.c */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sd = 0;
int efd;

typedef void *(*thread_fp)(void *);

/* return 0 for success */
int ss_pthread_create(pthread_t *thread, thread_fp func, void *arg) {
  int r;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  r = pthread_create(thread, &attr, func, arg);
  return r;
}

void *thread_func(void *p) {
  int r;
  int cfd;
  int connect_fd;
  struct epoll_event e = {0};
  struct sockaddr client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  while (1) {
    r = epoll_wait(efd, &e, 1, -1);
    if (-1 == r) {
      if (EINTR == errno) {
        continue;
      } else {
        printf("epoll wait error \n");
        break;
      }
    }

    printf("epoll get connecting event(%p)\n", (void *)pthread_self());

    cfd = accept(sd, &client_addr, &addrlen);
    if (-1 == cfd) {
      printf("connect error \n");
      return NULL;
    }

    shutdown(cfd, SHUT_RDWR);

    sleep(1);

    printf("connection has been closed(%p)\n", (void *)pthread_self());
  }

  return NULL;
}

int main(void) {
  int r;
  struct epoll_event e = {0};
  int optval = SO_REUSEADDR;
  struct sockaddr_in ss_addr;

  /* open socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sd) {
    printf("failed to open socket\n");
    return -1;
  }

  r = fcntl(sd, F_GETFL);
  if (r < 0) {
    printf("fcntl 1 error \n");
    return -1;
  }

  r |= O_NONBLOCK;
  r = fcntl(sd, F_SETFL, r);
  if (r < 0) {
    printf("fcntl 2 error \n");
    return -1;
  }

  r = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (r) {
    printf("setsockopt error \n");
    return -1;
  }

  ss_addr.sin_family = AF_INET;
  ss_addr.sin_port = htons(8080);
  ss_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  r = bind(sd, (struct sockaddr *)(&ss_addr), sizeof(ss_addr));
  if (-1 == r) {
    printf("bind error \n");
    return -1;
  }

  r = listen(sd, 100);
  if (-1 == r) {
    printf("listen error \n");
    return -1;
  }

  efd = epoll_create(10);
  if (-1 == efd) {
    printf("epoll create error \n");
    return -1;
  }

#define EPOLLFLAGS (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLET)

  e.events = EPOLLFLAGS;
  e.data.u32 = 1;
  r = epoll_ctl(efd, EPOLL_CTL_ADD, sd, &e);
  if (-1 == r) {
    printf("epoll add socket error \n");
    return -1;
  }

  pthread_t thread;

  ss_pthread_create(&thread, thread_func, NULL);
  ss_pthread_create(&thread, thread_func, NULL);
  ss_pthread_create(&thread, thread_func, NULL);
  ss_pthread_create(&thread, thread_func, NULL);

  thread_func(NULL);

  return 0;
}