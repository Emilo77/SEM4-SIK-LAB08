#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "err.h"

#define BSIZE         256
#define HOPS_VALUE    4
#define REPEAT_COUNT  3
#define SLEEP_TIME    1

int main (int argc, char *argv[]) {
  /* argumenty wywołania programu */
  char *remote_colon_address;
  in_port_t remote_port;

  /* zmienne i struktury opisujące gniazda */
  int sock, optval;
  struct sockaddr_in6 local_address;
  struct sockaddr_in6 remote_address;

  /* zmienne obsługujące komunikację */
  char buffer[BSIZE];
  size_t length;
  time_t time_buffer;
  int i;

  /* parsowanie argumentów programu */
  if (argc != 3)
    fatal("Usage: %s remote_address remote_port\n", argv[0]);
  remote_colon_address = argv[1];
  remote_port = (in_port_t)atoi(argv[2]);

  /* otwarcie gniazda */
  sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket");

  /* ustawienie liczby skoków  dla datagramów rozsyłanych do grupy */
  optval = HOPS_VALUE;
  if (setsockopt(sock, SOL_IPV6, IPV6_MULTICAST_HOPS, (void*)&optval, sizeof optval) < 0)
    syserr("setsockopt multicast hops");

  /* związanie z gniazdem adresu i portu odbiorcy, aby móc użyć write zamiast sendto */
  remote_address.sin6_family = AF_INET6;
  remote_address.sin6_port = htons(remote_port);
  if (inet_pton(AF_INET6, remote_colon_address, &remote_address.sin6_addr) < 0)
    syserr("inet_pton");
  if (connect(sock, (struct sockaddr *)&remote_address, sizeof remote_address) < 0)
    syserr("connect");

  /* radosne rozgłaszanie czasu */
  for (i = 0; i < REPEAT_COUNT; ++i) {
    time(&time_buffer);
    strncpy(buffer, ctime(&time_buffer), BSIZE);
    length = strnlen(buffer, BSIZE);
    if (write(sock, buffer, length) != length)
      syserr("write");
    sleep(SLEEP_TIME);
  }

  /* koniec */
  close(sock);
  exit(EXIT_SUCCESS);
}
