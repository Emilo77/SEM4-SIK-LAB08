#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "err.h"

#define BSIZE 256
#define HOPS_VALUE 4
#define REPEAT_COUNT 3
#define SLEEP_TIME 1
#define CONNECTIONS 2
#define TIMEOUT 3000
#define TEMP_BUFF_SIZE 64

char buffer[BSIZE];

void send_GET_TIME(int sockfd, int counter, struct sockaddr_in6 *dest_addr) {
  char *message = "GET TIME";
  strcpy(buffer, message);
  send_message_address(sockfd, buffer, strlen(message), NO_FLAGS,
                       (struct sockaddr *)dest_addr, sizeof(*dest_addr));
  printf("Sending request [%d]\n", counter + 1);
}

void receive_GET_TIME(int sockfd) {
  char temp_buffer[TEMP_BUFF_SIZE];
  struct sockaddr server_addr;
  socklen_t add_len;
  size_t received_length = receive_message_address(
      sockfd, buffer, sizeof(buffer), NO_FLAGS, &server_addr, &add_len);
  if (received_length < 0) {
    PRINT_ERRNO();
  }
  printf("Response from: %s\n",
         inet_ntop(AF_INET6, &server_addr, temp_buffer, sizeof(temp_buffer)));
  printf("Received time: %.*s\n", (int)received_length, buffer);
}

int main(int argc, char *argv[]) {
  struct sockaddr_in6 remote_address;
  struct pollfd descriptor[CONNECTIONS];

  /* parsowanie argumentów programu */
  if (argc != 3)
    fatal("Usage: %s remote_address remote_port\n", argv[0]);
  char *remote_colon_address = argv[1];
  uint16_t remote_port = read_port(argv[2]);

  for (int i = 0; i < CONNECTIONS; ++i) {
    descriptor[i].fd = -1;
    descriptor[i].events = POLLIN;
    descriptor[i].revents = 0;
  }

  descriptor[0].fd = open_udp_ip6_socket();

  /* ustawienie liczby skoków dla datagramów rozsyłanych do grupy */
  int optval = HOPS_VALUE;
  CHECK_ERRNO(setsockopt(descriptor[0].fd, SOL_IPV6, IPV6_MULTICAST_HOPS,
                         (void *)&optval, sizeof optval));

  /* związanie z gniazdem adresu i portu odbiorcy, aby móc użyć write zamiast
   * sendto */
  remote_address.sin6_family = AF_INET6;
  remote_address.sin6_port = htons(remote_port);
  CHECK_ERRNO(
      inet_pton(AF_INET6, remote_colon_address, &remote_address.sin6_addr));

  //  connect_socket_ip6(descriptor[0].fd, &remote_address);

  int counter = 0;
  do {
    send_GET_TIME(descriptor[0].fd, counter, &remote_address);

    for (int i = 0; i < CONNECTIONS; ++i) {
      descriptor[i].revents = 0;
    }

    int poll_status = poll(descriptor, CONNECTIONS, TIMEOUT);
    if (poll_status == -1) {
      PRINT_ERRNO();
    } else if (poll_status > 0) {
      if (descriptor[0].revents & POLLIN) {
        receive_GET_TIME(descriptor[0].fd);
        break;
      }
    }
    counter++;
    if (counter + 1 > 3) {
      printf("Timeout: unable to receive response.\n");
      break;
    }
  } while (1);

  /* koniec */
  CHECK_ERRNO(close(descriptor[0].fd));
}
