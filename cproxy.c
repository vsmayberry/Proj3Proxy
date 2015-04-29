/*
   Virgil Mayberry
   Nick Dejaco
   CSc 425 Project 3
   Simple Network Proxy
   University of Arizona
   Due Friday May 8, 2014
   Uses code provided as reference on https://d2l.arizona.edu/content/enforced/408520-765-2151-1CSC425001/tcpclient.c
   */

//take input cproxy w.x.y.z
//process IP
//telnet localhost 5200
//TCP connection to Server port 6200 at w.x.y.z
//calls select() on both sockets
//if data then read from that socket and write to the other socket
//if heartbeat update timer
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define BUF_SIZE 1024

#define SHUT_FD1 do {                                \
        if (fd1 >= 0) {                 \
                shutdown(fd1, SHUT_RDWR);   \
                close(fd1);                 \
                fd1 = -1;                   \
        }                               \
} while (0)

#define SHUT_FD2 do {                                \
        if (fd2 >= 0) {                 \
                shutdown(fd2, SHUT_RDWR);   \
                close(fd2);                 \
                fd2 = -1;                   \
        }                               \
} while (0)


static int forward_port;

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

static int listen_socket(int listen_port)
{
        char local_ip[] = "127.0.0.1";

        struct sockaddr_in a;
        int s;
        int yes;

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) {
                perror("socket");
                return -1;
        }
        yes = 1;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                                &yes, sizeof(yes)) == -1) {
                perror("setsockopt");
                close(s);
                return -1;
        }
        memset(&a, 0, sizeof(a));
        a.sin_port = htons(listen_port);
        a.sin_family = AF_INET;
        if (bind(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
                perror("bind");
                close(s);
                return -1;
        }
        printf("accepting connections on port %d\n", listen_port);
        listen(s, 10);
        return s;
}


static int connect_socket(int connect_port, char *address)
{
        struct sockaddr_in a;
        int s;

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) {
                perror("socket");
                close(s);
                return -1;
        }

        memset(&a, 0, sizeof(a));
        a.sin_port = htons(connect_port);
        a.sin_family = AF_INET;

        if (!inet_aton(address, (struct in_addr *) &a.sin_addr.s_addr)) {
                perror("bad IP address format");
                close(s);
                return -1;
        }

        if (connect(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
                perror("connect()");
                shutdown(s, SHUT_RDWR);
                close(s);
                return -1;
        }
        return s;
}

int main(int argc, char *argv[])
{
        int h;
        int fd1 = -1, fd2 = -1;
        char buf1[BUF_SIZE], buf2[BUF_SIZE];
        int buf1_avail, buf1_written;
        int buf2_avail, buf2_written;
        int forward_port = 6200;
        int listen_port = 5200;

        if (argc != 2) {
                fprintf(stderr, "Usage\n\tcproxy <forward-to-ip-address>\n");
                exit(EXIT_FAILURE);
        }

        //TODO
        signal(SIGPIPE, SIG_IGN);


        h = listen_socket(listen_port);
        if (h == -1)
                exit(EXIT_FAILURE);

        for (;;) {
                int r, nfds = 0;
                fd_set rd, wr, er;

                FD_ZERO(&rd);
                FD_ZERO(&wr);
                FD_ZERO(&er);
                FD_SET(h, &rd);
                nfds = max(nfds, h);

                //if file descriptor 1 is ready and queued packets
                if (fd1 > 0 && buf1_avail < BUF_SIZE)
                {
                        FD_SET(fd1, &rd);
                        nfds = max(nfds, fd1);
                }
                //if file descriptor 2 is ready and queued packets
                if (fd2 > 0 && buf2_avail < BUF_SIZE)
                {
                        FD_SET(fd2, &rd);
                        nfds = max(nfds, fd2);
                }
                //if file descriptor 1 is ready and available > written
                if (fd1 > 0 && buf2_avail - buf2_written > 0)
                {
                        FD_SET(fd1, &wr);
                        nfds = max(nfds, fd1);
                }
                if (fd2 > 0 && buf1_avail - buf1_written > 0)
                {
                        FD_SET(fd2, &wr);
                        nfds = max(nfds, fd2);
                }
                r = select(nfds + 1, &rd, &wr, &er, NULL);

                if (r == -1 && errno == EINTR)
                        continue;

                if (r == -1) {
                        perror("select()");
                        exit(EXIT_FAILURE);
                }

                if(FD_ISSET(h, &rd))
                {
                        int length;
                        struct sockaddr_in server_address;
                        memset(&server_address, 0, length = sizeof(server_address));
                        r = accept(h,(struct sockaddr*) &server_address, &length );
                        if(r==-1)
                        {
                                perror("accept()");
                        }
                        else
                        {

                                fd1 = r;
                                fd2 = connect_socket(forward_port,argv[1]);
                                if(fd2==-1)
                                {
                                        SHUT_FD1;
                                }
                                else
                                        printf("connect from %s\n", inet_ntoa(server_address.sin_addr));
                        }
                }
                if(fd1>0)
                {
                        if(FD_ISSET(fd1, &rd))
                        {
                                r = read(fd1, buf1, BUF_SIZE);
                                if (r < 1)
                                        SHUT_FD1;
                        }
                }

                if(fd2>0)
                {
                        if(FD_ISSET(fd2, &rd))
                        {
                                r = read(fd2, buf2, BUF_SIZE);
                                if (r < 1)
                                        SHUT_FD2;
                        }
                }

                if (fd1 > 0)
                {
                        if (FD_ISSET(fd1, &wr)) {
                                r = write(fd1, buf2 + buf2_written,
                                                buf2_avail - buf2_written);
                                if (r < 1)
                                        SHUT_FD1;
                        }
                }
                if (fd2 > 0)
                {
                        if (FD_ISSET(fd2, &wr)) {
                                r = write(fd2, buf1 + buf1_written,
                                                buf1_avail - buf1_written);
                                if (r < 1)
                                        SHUT_FD2;
                        }
                }

        }
        exit(EXIT_SUCCESS);
}
