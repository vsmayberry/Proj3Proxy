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
int checkPacket(char* buffer);

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
        int h, connected;
        int fd1 = -1, fd2 = -1;
        int forward_port = 23;
        int listen_port = 6200;
        struct timeval timeout;
        s_pending=0;
        c_pending=0;
        char local_ip[] = "127.0.0.1";
        connected = 0;

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
        int ack_number = 0;
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
                //printf("ack %d, seq %d\n", Uack_num, sequence_number);
                int t = (int) timeout.tv_usec;

                elapsed = elapsed - (1000000 - t);
                timed = timed - (1000000 - t);
                if(timed<0 && connected)
                {
                        //dead_connection(fd2);
                        if(fd2>=0)
                        {
                                printf("DEAD CONNECTION\n");
                                shutdown(fd2, SHUT_RDWR);
                                close(fd2);
                                fd2 = -1;
                                connected = 0;
                        }
                }
                if(elapsed<=0)
                {
                        send_heartbeat = 1;
                        elapsed = 1000000;

                }

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
                                {

                                        timed = 3000000;
                                        connected = 1;
                                        printf("Connection from %s to %s\n", inet_ntoa(client_address.sin_addr), argv[1]);
                                }
                        }
                }
                //if the accept returned something
                if(fd1>0)
                {
                        //if the fd1 socket is ready
                        //create a new packet to store in the queue
                        if(FD_ISSET(fd1, &rd))
                        {
                                char temp[BUF_SIZE + 8];
                                memset(temp, 0, BUF_SIZE + 8);
                                r = read(fd1, temp, BUF_SIZE + 8);


                                struct data_packet data2;
                                struct data_packet* data;

                                if (r >= 1 && strlen(temp) == 0) {
                                        if (checkPacket(temp) == HEART_P_TYPE)
                                        {
                                                timed = 3000000;
                                                //printf("RECEIVED HEART BEAT\n");
                                        }
                                        else if (checkPacket(temp) == CONNECT_P_TYPE)
                                                printf("RECEIVED CONNECTION INITIATION\n");
                                        else {
                                                data2 = unpackPacket(temp);
                                                data = &data2;
                                                ack_number = data -> seq_num;
                                                printf("Ack number = %d\n", data -> ack_num);
                                                printf("Seq number = %d\n", data -> seq_num);



                                                if (caching != NULL)  {
                                                      if (data -> ack_num > 0) {
                                                         while (caching -> seq_num <= data -> ack_num) {
                                                                data_packet* delete = caching;
                                                                printf("Deleted %d\n", caching -> seq_num);
                                                                if (caching -> next != NULL) {
                                                                       caching = caching -> next;
                                                                       free(delete);
                                                                }
                                                                else {
                                                                       caching = NULL;
                                                                       free(delete);
                                                                       break;
                                                                }
                                                         }
                                                      }
                                               }
 
                                        }
                                }


                                if (r < 1)
                                {
                                        printf("here");
                                        SHUT_FD1;

                                }
                                else
                                {
                                        if (checkPacket(temp) == DATA_P_TYPE) {
                                                data_packet* temp2;
                                                temp2 = to_s_packets;
                                                if(temp2==NULL)
                                                {
                                                        to_s_packets = data;
                                                        data->next = NULL;
                                                }
                                                s_pending+=1;
                                        }
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
                                data = malloc(sizeof(*data));
                                data->type = DATA_P_TYPE;
                                memset(&data->buf, 0, sizeof(BUF_SIZE));
                                r = read(fd2, data->buf, BUF_SIZE);
                                data->payload = r;
                                data -> seq_num = sequence_number++;
                                data -> ack_num = ack_number;
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

                                        data_packet* temp5;
                                        temp5 = caching;
                                        if (temp5 == NULL) {
                                             struct data_packet temp4 = *data;
                                             caching = malloc(sizeof(temp4));
                                             caching -> type = temp4.type;
                                             caching -> payload = temp4.payload;
                                             caching -> seq_num = temp4.seq_num;
                                             caching -> ack_num = temp4.ack_num;
                                             strcpy(caching -> buf, temp4.buf);
                                             caching -> next = NULL;
                                       }

                                       else
                                          {
                                                while(temp5->next!=NULL)     {
                                                        temp5 = temp5->next;
                                                }
                                                struct data_packet temp6 = *data;
                                                temp5->next = malloc(sizeof(temp6));
                                                temp5 -> next -> type = temp6.type;
                                                temp5 -> next -> payload = temp6.payload;
                                                temp5 -> next -> seq_num = temp6.seq_num;
                                                temp5 -> next -> ack_num = temp6.ack_num;
                                                strcpy(temp5 -> next -> buf, temp6.buf);
                                                temp5 -> next -> next = NULL;

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



                                if(send_heartbeat == 1)
                                {
                                        heartbeat_packet* hb_packet = malloc(sizeof(hb_packet));
                                        char buf[16];
                                        memset(buf, 0, 16);
                                        hb_packet->type =HEART_P_TYPE;
                                        hb_packet->payload = 0;
                                        hb_packet->seq_num = 0;
                                        hb_packet->ack_num = 0;
                                        pack_hb_packet(hb_packet, buf);
                                        if (checkPacket(buf) == HEART_P_TYPE)
                                                r = write(fd1, buf, sizeof(buf));
                                        send_heartbeat = 0;

                                }



                                if(to_c_packets!=NULL)
                                {
                                        data_packet* data = to_c_packets;
                                        to_c_packets = to_c_packets->next;
                                        char buffer[BUF_SIZE + 8];
                                        data->ack_num = ack_number;
                                        packPacket(data, buffer);
                                        to_c_packets = NULL;
                                        if (checkPacket(buffer) == DATA_P_TYPE)
                                                r = write(fd1, buffer, sizeof(buffer));
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

int checkPacket(char* buffer) {
        int  a, b;
        memcpy((char *) &a, buffer, 2);
        return ntohs(a);
}

