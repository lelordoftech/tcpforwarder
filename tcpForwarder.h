#ifndef TCP_FORWARDER_H
#define TCP_FORWARDER_H

#include<stdio.h> // printf
#include<string.h> // memcpy, memset
#include<stdlib.h> // atoi
#include<sys/socket.h>
#include<arpa/inet.h> // inet_addr
#include<unistd.h> // write, close socket
#include<pthread.h> //for threading , link with -lpthread

struct socket_par
{
  int soc_in;
  int soc_out;
};

char in_ip[15] = "192.168.1.100";
int in_port = 44405;
char out_ip[15] = "192.168.1.100";
int out_port = 44406;

/** \brief Create a socket for listening
 * 
 *  @param ip_p IP of socket
 *  @param ip_len length of IP
 *  @param port Port of socket
 *  @return A socket id
 */
int createListenSocket(const char* ip_p, int ip_len, int port);

/** \brief Create a socket for sending
 * 
 *  @param ip_p IP of socket
 *  @param ip_len length of IP
 *  @param port Port of socket
 *  @return A socket id
 */
int createSendSocket(const char* ip_p, int ip_len, int port);

/** \brief Handle connection for each client
 * 
 *  @param par Parameter of handler
 *  @return A pointer to the created algorithm
 */
void* connection_handler(void* par);

#endif