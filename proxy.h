
#define BUF_SIZE 1024

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
