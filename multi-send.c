#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "err.h"
#include "common.h"

#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  3
#define SLEEP_TIME    1

int main(int argc, char *argv[]) {

    /* zmienne obsługujące komunikację */
    char buffer[BSIZE];
    time_t time_buffer;

    /* parsowanie argumentów programu */
    if (argc != 3)
        fatal("Usage: %s remote_address remote_port\n", argv[0]);
    char *remote_dotted_address = argv[1];
    uint16_t remote_port = read_port(argv[2]);

    int socket_fd = open_udp_socket();

    /* uaktywnienie rozgłaszania (ang. broadcast) */
    int optval = 1;
    CHECK_ERRNO(setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval));

    /* ustawienie TTL dla datagramów rozsyłanych do grupy */
    optval = TTL_VALUE;
    CHECK_ERRNO(setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval));

    /* ustawienie adresu i portu odbiorcy */
    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(remote_port);
    if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0) {
        fprintf(stderr, "ERROR: inet_aton - invalid multicast address\n");
        exit(EXIT_FAILURE);
    }

    /* związanie z gniazdem adresu i portu odbiorcy, aby móc użyć write zamiast sendto */
    connect_socket(socket_fd, &remote_address);

    /* radosne rozgłaszanie czasu */
    for (int i = 0; i < REPEAT_COUNT; ++i) {
        time(&time_buffer);
        strncpy(buffer, ctime(&time_buffer), BSIZE);
        size_t length = strnlen(buffer, BSIZE);
        send_message(socket_fd, buffer, length, NO_FLAGS);
        sleep(SLEEP_TIME);
    }

    /* koniec */
    CHECK_ERRNO(close(socket_fd));
}
