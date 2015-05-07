
#define BUF_SIZE 1024
#define CONNECT_P_TYPE 0
#define DATA_P_TYPE    1
#define HEART_P_TYPE   2

//new connection initiation
typedef struct connect_packet
{
        int type;
        int payload;
        int seq_num;
        int ack_num;
}connect_packet;
//data packet
typedef struct data_packet
{
        int type;
        int payload;
        int seq_num;
        int ack_num;
        char buf[BUF_SIZE];
        struct data_packet* next;
}data_packet;
//heartbeat packet
typedef struct heartbeat_packet
{
        int type;
        int payload;
        int seq_num;
        int ack_num;
}heartbeat_packet;

data_packet* to_s_packets = NULL;
data_packet* to_c_packets = NULL;
int s_pending;
int c_pending;

//HEARTBEAT CODE
int elapsed=1000000;
int timeout=3000000;
int check_elapsed(int t, int socket)
{
        elapsed = elapsed - (1000000 - t);
        timeout = timeout - (1000000 - t);
        if(timeout<0)
        {
                dead_connection(socket);
                return -1;
        }
        if(elapsed<0)
        {
                elapsed = 1000000;
                return 0;
        }
        else if(elapsed > 0)
        {
                return 1;
        }
}

int recvd_beat()
{
        timeout = 3000000;
        return 0;
}

int dead_connection(int socket)
{
        do{
                if(socket>=0)
                {
                        close(socket);
                        socket = -1;
                }
        }while(0)
        return 0;
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
        printf("Data = %s\n", myPacket -> buf);
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
