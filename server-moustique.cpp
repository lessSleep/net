#include "moustique.hh"
#define BUFFER "HTTP/1.1 200 OK\r\nContent-Length: 102\r\nConnection: close\r\nContent-Type: text/html;charset=utf-8\r\n\r\n <html> 响应数据 <head> <title>HTTP响应示例<title> </head>  <body> Hello HTTP! </body> </html>"
int main()
{
    moustique_listen(12345, // Port number
                     SOCK_STREAM, // TCP socket
                     4, // number of threads
         [] (int fd, auto read, auto write) {

           //printf("new connection: %i\n", fd);

           char buf[1024];
           int received;
           
           //while (received = read(buf, sizeof(buf))) // Suspend until new bytes
                                                     // are available for reading.
            received = read(buf, sizeof(buf));
            //  if (!write(BUFFER, strlen(BUFFER))) // Suspend until the socket is ready for a write.
            //    break;
            write(BUFFER, strlen(BUFFER));
            //close(fd);



           //printf("end of connection: %i\n", fd);
         }
    );
}