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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/*
 Prints error message and exits.
 */
void error(char *msg){
  perror(msg);
  exit(1);
}

int main(int argc, char * argv[]){
  // port number to listen on
  int portnum = 6200;
  // socket file descriptors, bytes sent and received 
  int sockfd, session, bytes_sent, bytes_received;
  // stores the length of client's sockaddr struct for accept function
  socklen_t clilen;
  // receive buffer
  char buffer[256];
  // structure that contains info about internet addr
  struct sockaddr_in serv_addr, cli_addr;
  // server's reply
  char * reply = "Hello client";

  // create a stream socket (TCP)
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error ("ERROR opening socket");
  // 0 out the struct
  memset (&serv_addr, 0, sizeof(serv_addr));
  // domain is Internet
  serv_addr.sin_family = AF_INET;
  // don't need a specific address, any of the network interface will do
//TODO
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  // set port number, and stores it in network byte order
  serv_addr.sin_port = htons (portnum);

  // bind socket to an address
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) 
    error("ERROR on binding");

  // listen to socket, set maximum pending connection is 5
  listen (sockfd, 5);
  // set length of client address
  clilen = sizeof (cli_addr);
  // accept first incoming connection on the pending queue, returns a new file descriptor
  session = accept (sockfd, (struct sockaddr *) &cli_addr, &clilen);

  if (session < 0)
    error("ERROR on accept");
  // 0 out the recieve buffer
  memset (buffer, 0, sizeof (buffer));
  // receive data from client and puts it in buffer
//TODO
  bytes_received = recv (session, buffer, sizeof (buffer), 0);
  if (bytes_received < 0)
    error("ERROR reading from socket");
  
  printf ("Client said: %s\n",buffer);
  // send client a reply
  bytes_sent = send (session, reply, sizeof (char) * strlen (reply), 0);
  if (bytes_sent < 0)
    error("ERROR writing to socket");

  close (session);
  close (sockfd); 
//TODO
  return 0; 
}
