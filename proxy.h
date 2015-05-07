
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


int dead_connection(int socket)
{
        do{
                if(socket>=0)
                {
                        close(socket);
                        socket = -1;
                }
        }while(0);
        return 0;
}


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
