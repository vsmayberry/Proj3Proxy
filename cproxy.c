//take input cproxy w.x.y.z
//process IP
//telnet localhost 5200
//TCP connection to Server port 6200 at w.x.y.z
//calls select() on both sockets
//if data then read from that socket and write to the other socket
//if heartbeat update timer
