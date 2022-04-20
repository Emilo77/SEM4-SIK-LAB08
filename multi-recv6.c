#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "err.h"

#define BSIZE         1024
#define REPEAT_COUNT  30

int main (int argc, char *argv[]) {

  /* argumenty wywołania programu */
  char *multicast_colon_address;
  in_port_t local_port;

  /* zmienne i struktury opisujące gniazda */
  int sock;
  struct sockaddr_in6 local_address;
  struct ipv6_mreq ipv6_mreq;

  /* zmienne obsługujące komunikację */
  char buffer[BSIZE];
  ssize_t rcv_len;
  int i;

  /* parsowanie argumentów programu */
  if (argc != 3)
    fatal("Usage: %s multicast_colon_address local_port\n", argv[0]);
  multicast_colon_address = argv[1];
  local_port = (in_port_t)atoi(argv[2]);

  /* otwarcie gniazda */
  sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket");

  /* podłączenie do grupy rozsyłania (ang. multicast) */
  ipv6_mreq.ipv6mr_interface = 0;
  if (inet_pton(AF_INET6, multicast_colon_address, &ipv6_mreq.ipv6mr_multiaddr) < 0)
    syserr("inet_pton");
  if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, (void*)&ipv6_mreq, sizeof ipv6_mreq) < 0)
    syserr("setsockopt");

  /* związanie z gniazdem lokalnego adresu i portu */
  local_address.sin6_family = AF_INET6;
  local_address.sin6_flowinfo = 0;
  local_address.sin6_addr = in6addr_any;
  local_address.sin6_port = htons(local_port);
  if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
    syserr("bind");

  /* czytanie tego, co odebrano */
  for (i = 0; i < REPEAT_COUNT; ++i) {
    rcv_len = read(sock, buffer, sizeof buffer);
    if (rcv_len < 0)
      syserr("read");
    else
      printf("read %zd bytes: %.*s\n", rcv_len, (int)rcv_len, buffer);
  }

  /* koniec */
  close(sock);
  exit(EXIT_SUCCESS);
}
