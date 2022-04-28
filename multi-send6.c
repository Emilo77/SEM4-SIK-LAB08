#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "err.h"
#include "common.h"

#define BSIZE         256
#define HOPS_VALUE    4
#define REPEAT_COUNT  3
#define SLEEP_TIME    1

int main(int argc, char *argv[]) {
    struct sockaddr_in6 remote_address;

    /* zmienne obsługujące komunikację */
    char buffer[BSIZE];
    time_t time_buffer;

    /* parsowanie argumentów programu */
    if (argc != 3)
        fatal("Usage: %s remote_address remote_port\n", argv[0]);
    char *remote_colon_address = argv[1];
    uint16_t remote_port = read_port(argv[2]);

    int socket_fd = open_udp_ip6_socket();

    /* ustawienie liczby skoków dla datagramów rozsyłanych do grupy */
    int optval = HOPS_VALUE;
    CHECK_ERRNO(setsockopt(socket_fd, SOL_IPV6, IPV6_MULTICAST_HOPS, (void *) &optval, sizeof optval));

    /* związanie z gniazdem adresu i portu odbiorcy, aby móc użyć write zamiast sendto */
    remote_address.sin6_family = AF_INET6;
    remote_address.sin6_port = htons(remote_port);
    CHECK_ERRNO(inet_pton(AF_INET6, remote_colon_address, &remote_address.sin6_addr));

    connect_socket_ip6(socket_fd, &remote_address);

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
