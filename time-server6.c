#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "err.h"

#define BSIZE 1024
#define REPEAT_COUNT 30
char buffer[BSIZE];

void send_time(int sockfd, struct sockaddr *client_addr,
               socklen_t client_addr_len) {
  char buf[64];
  printf("Request from: %s\n",
         inet_ntop(AF_INET6, &client_addr, buf, sizeof(buf)));
  time_t time_buffer;
  time(&time_buffer);
  strncpy(buffer, ctime(&time_buffer), BSIZE);
  size_t length = strnlen(buffer, BSIZE);
  sendto(sockfd, buffer, length, NO_FLAGS, client_addr, client_addr_len);
}

int main(int argc, char *argv[]) {
  /* parsowanie argumentów programu */
  if (argc != 3)
    fatal("Usage: %s multicast_colon_address local_port\n", argv[0]);
  char *multicast_colon_address = argv[1];
  uint16_t port = read_port(argv[2]);

  /* otwarcie gniazda */
  int socket_fd = open_udp_ip6_socket();

  /* podłączenie do grupy rozsyłania (ang. multicast) */
  struct ipv6_mreq ipv6_mreq;
  ipv6_mreq.ipv6mr_interface = 0;
  CHECK_ERRNO(inet_pton(AF_INET6, multicast_colon_address,
                        &ipv6_mreq.ipv6mr_multiaddr));
  CHECK_ERRNO(setsockopt(socket_fd, SOL_IPV6, IPV6_ADD_MEMBERSHIP,
                         (void *)&ipv6_mreq, sizeof ipv6_mreq));

  /* ustawienie adresu i portu lokalnego */
  bind_ip6_socket(socket_fd, port);
  printf("Listening on %s %d\n", argv[1], port);

  /* czytanie tego, co odebrano */
  while (1) {
    struct sockaddr client_address;
    socklen_t add_len;
    size_t received_length = receive_message_address(
        socket_fd, buffer, sizeof(buffer), NO_FLAGS, &client_address, &add_len);
    if (strncmp(buffer, "GET TIME", received_length) == 0) {
      send_time(socket_fd, &client_address, add_len);
    } else {
      printf("Received unknown command: %.*s\n", (int)received_length, buffer);
    }
  }
  /* odłączenie od grupy rozsyłania */
  CHECK_ERRNO(setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                         (void *)&ipv6_mreq, sizeof(ipv6_mreq)));

  /* koniec */
  CHECK_ERRNO(close(socket_fd));
}
