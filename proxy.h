//new connection initiation
typedef struct connect_packet
{
	int type:0;
	int payload:0;
	int seq_num:0;
	int ack_num:0;
};
//data packet
typedef struct data_packet
{
	int type:1;
	int payload:0;
	int seq_num:0;
	int ack_num:0;
};
//heartbeat packet
typdef struct heartbeat_packet
{
	int type:2;
	int payload:0;
	int seq_num:0;
	int ack_num:0;
};
