#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "err.h"

#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  3
#define SLEEP_TIME    1

int main (int argc, char *argv[]) {
  /* argumenty wywołania programu */
  char *remote_dotted_address;
  in_port_t remote_port;

  /* zmienne i struktury opisujące gniazda */
  int sock, optval;
  struct sockaddr_in remote_address;

  /* zmienne obsługujące komunikację */
  char buffer[BSIZE];
  size_t length;
  time_t time_buffer;
  int i;

  /* parsowanie argumentów programu */
  if (argc != 3)
    fatal("Usage: %s remote_address remote_port\n", argv[0]);
  remote_dotted_address = argv[1];
  remote_port = (in_port_t)atoi(argv[2]);

  /* otwarcie gniazda */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket");

  /* uaktywnienie rozgłaszania (ang. broadcast) */ 
  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
    syserr("setsockopt broadcast"); 

  /* ustawienie TTL dla datagramów rozsyłanych do grupy */ 
  optval = TTL_VALUE;
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
    syserr("setsockopt multicast ttl");

  /* ustawienie adresu i portu odbiorcy */
  remote_address.sin_family = AF_INET;
  remote_address.sin_port = htons(remote_port);
  if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0) {
    fprintf(stderr, "ERROR: inet_aton - invalid multicast address\n");
    exit(EXIT_FAILURE);
  } 

  /* związanie z gniazdem adresu i portu odbiorcy, aby móc użyć write zamiast sendto */
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
