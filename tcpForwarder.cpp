/*******************************************************************************
 * Copyright 2017 VuongLQ. All rights reserved.
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
    printf("Could not create listening socket\n");
  }

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);

  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    puts("bind failed\n");
    return 1;
  }
  puts("bind done\n");

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
    printf("Could not create sending socket");
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);

  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    printf("connect error\n");
    return 1;
  }

  printf("Connected\n");

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
    printf("Forward from %d to %d: ", sock_in, sock_out);
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
    printf("Client disconnected\n");
    // Close socket
    close(sock_in);
    printf("Closed socket\n");
    fflush(stdout);
  }
  else if(read_size == -1)
  {
    printf("soc %d ", sock_in);
    perror("recv failed\n");
  }

  return 0;
}

int main(int argc , char *argv[])
{
  int socket_listen;
  int socket_send;
  int new_socket, socklen;//, *new_sock;
  struct sockaddr_in client;

  if (argc == 5)
  {
    memcpy(in_ip, argv[1], sizeof(argv[1]));
    in_port = atoi(argv[2]);
    memcpy(out_ip, argv[3], sizeof(argv[3]));
    out_port = atoi(argv[4]);
  }
  printf("Forward from %s:%d to %s:%d\n", in_ip, in_port, out_ip, out_port);

  socket_listen = createListenSocket(in_ip, sizeof(in_ip), in_port);

  if (socket_listen > 1)
  {
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    socklen = sizeof(struct sockaddr_in);

    while ((new_socket = accept(socket_listen, (struct sockaddr *)&client, (socklen_t*)&socklen)))
    {
      char* client_ip = inet_ntoa(client.sin_addr);
      int client_port = ntohs(client.sin_port);

      printf("Connection accepted: ip %s : port %i\n", client_ip, client_port);

      socket_send = createSendSocket(out_ip, sizeof(out_ip), out_port);

      // Thread for sock_in
      pthread_t sniffer_thread_in;
      socket_par x;
      x.soc_in = new_socket;
      x.soc_out = socket_send;
      if (pthread_create(&sniffer_thread_in, NULL, connection_handler , (void*)&x) < 0)
      {
        perror("could not create thread\n");
        return 1;
      }
      printf("Handler socket_in %d\n", new_socket);

      // Thread for sock_out
      pthread_t sniffer_thread_out;
      socket_par y;
      y.soc_in = socket_send;
      y.soc_out = new_socket;
      if (pthread_create(&sniffer_thread_out, NULL, connection_handler , (void*)&y) < 0)
      {
        perror("could not create thread\n");
        return 1;
      }
      printf("Handler socket_out %d\n", socket_send);

      //Now join the thread , so that we dont terminate before the thread
      //pthread_join( sniffer_thread , NULL);
    }

    if (new_socket < 0)
    {
      perror("accept failed\n");
      return 1;
    }

    return 0;
  }
  else
  {
    return 1;
  }
}
