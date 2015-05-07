/*
Virgil Mayberry
Nick Dejaco
University of Arizona
CSc 425 Project 3
Implementation of a network proxy
Due Friday May 8, 2015
The sproxy should listen to TCP port 6200 on the Server, and takes no command-line
argument.
Uses code provided on  https://d2l.arizona.edu/content/enforced/408520-765-2151-1CSC425001/tcpserver.c
*/
/*
  It listens on port
  8888, receives 1 short message from a client, then replies to client and exits.
 */


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
#include "proxy.h"
#define BUF_SIZE 1024

//remove some code snippets to clean up code
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
void packPacket(struct data_packet* myPacket, char* buffer);
struct data_packet unpackPacket(char* buffer);
#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

//function to establish a socket on localhost and start listening to it
static int listen_socket(int listen_port)
{

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

//function to connect to the server
static int connect_socket(int connect_port, char *address)
{
        struct sockaddr_in a;
        int s;

        printf("connection to socket port = %d\n", connect_port);
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
        int buf1_avail, buf1_written;
        int buf2_avail, buf2_written;
        int forward_port = 23;
        int listen_port = 6200;
        struct timeval timeout;
        s_pending=0;
        c_pending=0;
        char local_ip[] = "127.0.0.1";

        //test for to many or to few args
        if (argc != 1) {
                fprintf(stderr, "Usage\n\tsproxy\n");
                exit(EXIT_FAILURE);
        }

        //set signal so we can use write instead of send just a semantic
        signal(SIGPIPE, SIG_IGN);


        //open the port 5200 on local host
        //set h as the socket
        h = listen_socket(listen_port);
        if (h == -1)
                exit(EXIT_FAILURE);

        //main loop that contains the select call
        //essentially a timer loop that recvs stores and sends packets
        //also this loop monitors the connection to server
        for (;;) {
                timeout.tv_sec = 2;
                timeout.tv_usec = 0;
                //printf("FOR LOOP\n\n");
                //nfds is one of the args for the select function
                //r is used to catch sockets errors from read write
                int r, nfds = 0;
                //sets of file descriptors used by select to monitor when a socket is ready
                fd_set rd, wr, er;

                //zero out the sets that select is monitoring
                FD_ZERO(&rd);
                FD_ZERO(&wr);
                FD_ZERO(&er);
                //set the socket that listens on 5200 local host into the read set
                FD_SET(h, &rd);
                //taking care of nfds which should be one more that the largest integer in select
                nfds = max(nfds, h);

                //TODO
                FD_SET(fd1, &rd);
                nfds = max(nfds, fd1);
                FD_SET(fd2, &rd);
                nfds = max(nfds, fd2);
                FD_SET(fd1, &wr);
                nfds = max(nfds, fd1);
                FD_SET(fd2, &wr);
                nfds = max(nfds, fd2);

                //call select which can monitor time and whic sockets are ready
                r = select(nfds + 1, &rd, &wr, &er, &timeout);
                //printf("Select returned %d\n\n", r);

                //if it caught a signal ie select returned without a fd or timeout
                if (r == -1 && errno == EINTR)
                        continue;

                //select errored out
                if (r == -1) {
                        perror("select()");
                        exit(EXIT_FAILURE);
                }
                //if the localhost port is ready to read
                if(FD_ISSET(h, &rd))
                {
                        //set up and accept data from that socket
                        int length;
                        struct sockaddr_in client_address;
                        memset(&client_address, 0, length = sizeof(client_address));
                        r = accept(h,(struct sockaddr*) &client_address, &length );
                        if(r==-1)
                        {
                                perror("accept()");
                        }
                        else //accept had no problems
                        {
                                SHUT_FD1;
                                SHUT_FD2;
                                //set file descriptor 1 to the value returned by accept
                                fd1 = r;
                                //set fd2 to where we are trying to send to
                                fd2 = connect_socket(forward_port,local_ip);
                                //if fd2 is a bad connection then close it using the shut macro
                                if(fd2==-1)
                                {
                                        SHUT_FD1;
                                }
                                else
                                        printf("Connection from %s to %s\n", inet_ntoa(client_address.sin_addr), argv[1]);
                        }
                }
                //if the accept returned something
                if(fd1>0)
                {
                        //if the fd1 socket is ready
                        //create a new packet to store in the queue
                        if(FD_ISSET(fd1, &rd))
                        {
                                data_packet* data;
                                data = malloc(sizeof(data));
                                data->type = DATA_P_TYPE;
                                memset(&data->buf, 0, sizeof(BUF_SIZE));
                                r = read(fd1, data->buf, BUF_SIZE);
                                data->payload = r;
                                printf("data = %s\n", data -> buf);
                                if (r < 1)
                                {
                                        printf("here");
                                        SHUT_FD1;
                                }
                                else
                                {
                                        data_packet* temp;
                                        temp = to_s_packets;
                                        if(temp==NULL)
                                        {
                                                to_s_packets = data;
                                                data->next = NULL;
                                        }


                                        s_pending+=1;
                                }
                        }
                }

                //if the outgoing port is functioning
                if(fd2>0)
                {
                        //if the server side port has something to read
                        //read the packet and save it to the queue
                        if(FD_ISSET(fd2, &rd))
                        {
                                data_packet* data;
                                data = malloc(sizeof(data));
                                data->type = DATA_P_TYPE;
                                memset(&data->buf, 0, sizeof(BUF_SIZE));
                                r = read(fd2, data->buf, BUF_SIZE);
                                data->payload = r;
                                if (r < 1)
                                {
                                        printf("here2");
                                        SHUT_FD2;
                                }
                                else
                                {
                                        data_packet* temp;
                                        temp = to_c_packets;
                                        if(temp==NULL)
                                        {
                                                to_c_packets = data;
                                                data->next=NULL;
                                        }



                                        c_pending+=1;
                                }
                        }
                }

                //if client side is working
                if (fd1 > 0)
                {
                        //if client side is ready to write
                        //check if the queue is empty
                        //if not then send a packet
                        if (FD_ISSET(fd1, &wr)) {
                                if(to_c_packets!=NULL)
                                {
                                        data_packet* data = to_c_packets;
                                        to_c_packets = to_c_packets->next;
                                        char buffer[BUF_SIZE + 8];
                                        data -> seq_num = 0;
                                        data -> ack_num = 0;
                                        packPacket(data, buffer);
                                        to_c_packets = NULL;
                                        r = write(fd1, buffer, sizeof(buffer));

 
                                        //r = write(fd1, data->buf, data->payload);
                                        if (r < 1)
                                                SHUT_FD1;
                                        //free(data);
                                }
                                c_pending-=1;
                        }
                }
                //if server side is functioning
                if (fd2 > 0)
                {
                        //if the server side is ready to write
                        //send any pending packets
                        if (FD_ISSET(fd2, &wr)) {
                                if(to_s_packets!=NULL)
                                {
                                        data_packet* data = to_s_packets;
                                        to_s_packets = to_s_packets->next;
                                        r = write(fd2, data->buf, data->payload);
                                        if (r < 1)
                                                SHUT_FD2;
                                        //free(data);
                                }
                                s_pending-=1;
                        }
                }

        }
        exit(EXIT_SUCCESS);
}


void packPacket(struct data_packet* myPacket, char* buffer) {

    // Packs message into a packet with structure described in assignment spec using htons(l)
    // The buffer contains the time in seconds, the time in msec, the message lengh, and the actual message.

    int a, b, c, d;
    a =  htons(myPacket -> type);
    memcpy(buffer, (char *) &a, 2);
    b = htons(myPacket -> payload);
    memcpy(buffer+2, (char*) &b, 2);
    c = htons(myPacket -> seq_num);
    memcpy(buffer+4, (char*) &c, 2);
    d =  htons(myPacket -> ack_num);
    memcpy(buffer+6, (char *) &d, 2);
    memcpy(buffer+8, (char *) myPacket->buf, myPacket -> payload);
//    printf("Data = %s\n", myPacket -> buf);
   }


struct data_packet unpackPacket(char* buffer) {

   // Uses memcpy to copy out parts of the buffer back into their individual values in the packet struct.
   // Dynamically allocates space into for the message based on message length in header.

   struct data_packet tempPacket;
   int  a, b, c, d, e, f, g, h;
   memcpy((char *) &a, buffer, 2);
   b = ntohs(a);
   tempPacket.type = b;
   memcpy((char *) &c, buffer+2, 2);
   d = ntohs(c);
   tempPacket.payload = d;
   memcpy((char *) &e, buffer+4, 2);
   f = ntohs(e);
   tempPacket.seq_num = f;
   memcpy((char *) &g, buffer+6, 2);
   h = ntohs(g);
   tempPacket.ack_num = h;
   memcpy((char *) &tempPacket.buf, buffer + 8, tempPacket.payload + 1);
   return tempPacket;
}
