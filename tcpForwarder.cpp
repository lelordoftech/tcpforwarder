/*******************************************************************************
 * Copyright 2017 VuongLQ. All rights reserved.
 * ----------
 * Version details:
 * v1.1: Sep 13, 2017
 *  - Fix bug input argv
 *  - Optimize debug log format
 * v1.0: Sep 6, 2017
 *  - First completed
 ******************************************************************************/

#include "tcpForwarder.h"

int createListenSocket(const char* ip_p, int ip_len, int port)
{
  int socket_desc;
  struct sockaddr_in server;
  char ip[100];
  memcpy(ip, ip_p, ip_len);

  //Create socket for listening
  // AF_INET: IPv4 - AF_INET6: IPv6
  // SOCK_STREAM: TCP - SOCK_DGRAM: UDP
  // Protocol 0 [IPPROTO_IP]: IP protocol
  socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (socket_desc == -1)
  {
    printf("[createListenSocket] Could not create listening socket %s:%d\n", ip_p, port);
  }

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);

  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    printf("[createListenSocket] Bind %s:%d failed\n", ip_p, port);
    return 1;
  }
  printf("[createListenSocket] Bind %s:%d done\n", ip_p, port);

  //Listen
  listen(socket_desc , 3);

  return socket_desc;
}

int createSendSocket(const char* ip_p, int ip_len, int port)
{
  int socket_desc;
  struct sockaddr_in server;
  char ip[100];
  memcpy(ip, ip_p, ip_len);

  //Create socket for sending
  // AF_INET: IPv4 - AF_INET6: IPv6
  // SOCK_STREAM: TCP - SOCK_DGRAM: UDP
  // Protocol 0 [IPPROTO_IP]: IP protocol
  socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (socket_desc == -1)
  {
    printf("[createSendSocket] Could not create sending socket %s:%d", ip_p, port);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);

  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    printf("[createSendSocket] Connect %s:%d error \n", ip_p, port);
    return 1;
  }

  printf("[createSendSocket] Connected %s:%d\n", ip_p, port);

  return socket_desc;
}

void* connection_handler(void* par)
{
  //Get the socket descriptor
  struct sockaddr_in client;
  int client_len;
  socket_par* sock_par = (socket_par*)par;
  int sock_in = sock_par->soc_in;
  int sock_out = sock_par->soc_out;
  int read_size;
  unsigned char client_message[2000];

  //Receive a message from client
  while ((read_size = recv(sock_in, client_message, 2000, 0)) > 0)
  {
    // Debug
    printf("[Handler] Forward from %d to %d: ", sock_in, sock_out);
    for (int i = 0; i < read_size; i++)
    {
      printf("%2x ", (int)client_message[i]);
    }
    printf("\n");
    // Forward the message
    write(sock_out, client_message, read_size);
    memset(client_message, 0, 2000); // Reset client message
  }

  if (read_size == 0)
  {
    printf("[Handler] Client disconnected\n");
    // Close socket
    close(sock_in);
    printf("[Handler] Closed socket\n");
    fflush(stdout);
  }
  else if(read_size == -1)
  {
    printf("[Handler] socket %d ", sock_in);
    perror("recv failed\n");
  }

  return 0;
}

int main(int argc , char *argv[])
{
  int socket_listen;
  int socket_send;
  int new_socket, socklen;
  struct sockaddr_in client;

  if (argc == 5)
  {
    memset(in_ip, 0, sizeof(in_ip));
    memcpy(in_ip, argv[1], strlen(argv[1]));
    in_port = atoi(argv[2]);
    memset(out_ip, 0, sizeof(out_ip));
    memcpy(out_ip, argv[3], strlen(argv[3]));
    out_port = atoi(argv[4]);
  }
  printf("[Main] Forward from %s:%d to %s:%d\n", in_ip, in_port, out_ip, out_port);

  socket_listen = createListenSocket(in_ip, sizeof(in_ip), in_port);

  if (socket_listen > 1)
  {
    //Accept and incoming connection
    puts("[Main] Waiting for incoming connections...");
    socklen = sizeof(struct sockaddr_in);

    while ((new_socket = accept(socket_listen, (struct sockaddr *)&client, (socklen_t*)&socklen)))
    {
      char* client_ip = inet_ntoa(client.sin_addr);
      int client_port = ntohs(client.sin_port);

      printf("[Main] Connection accepted: ip %s : port %i\n", client_ip, client_port);

      socket_send = createSendSocket(out_ip, sizeof(out_ip), out_port);

      // Thread for sock_in
      pthread_t sniffer_thread_in;
      socket_par x;
      x.soc_in = new_socket;
      x.soc_out = socket_send;
      if (pthread_create(&sniffer_thread_in, NULL, connection_handler , (void*)&x) < 0)
      {
        perror("[Main] Could not create thread\n");
        return 1;
      }
      printf("[Main] Handler socket in %d\n", new_socket);

      // Thread for sock_out
      pthread_t sniffer_thread_out;
      socket_par y;
      y.soc_in = socket_send;
      y.soc_out = new_socket;
      if (pthread_create(&sniffer_thread_out, NULL, connection_handler , (void*)&y) < 0)
      {
        perror("[Main] Could not create thread\n");
        return 1;
      }
      printf("[Main] Handler socket out %d\n", socket_send);

      //Now join the thread , so that we dont terminate before the thread
      //pthread_join( sniffer_thread , NULL);
    }

    if (new_socket < 0)
    {
      perror("[Main] Accept failed\n");
      return 1;
    }

    return 0;
  }
  else
  {
    return 1;
  }
}
