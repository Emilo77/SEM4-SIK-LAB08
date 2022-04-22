#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "err.h"
#include "common.h"

#define BSIZE         1024
#define REPEAT_COUNT  30

int main(int argc, char *argv[]) {
    /* zmienne obsługujące komunikację */
    char buffer[BSIZE];

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
    CHECK_ERRNO(inet_pton(AF_INET6, multicast_colon_address, &ipv6_mreq.ipv6mr_multiaddr));
    CHECK_ERRNO(setsockopt(socket_fd, SOL_IPV6, IPV6_ADD_MEMBERSHIP, (void *) &ipv6_mreq, sizeof ipv6_mreq));

    /* ustawienie adresu i portu lokalnego */
    bind_ip6_socket(socket_fd, port);

    /* czytanie tego, co odebrano */
    for (int i = 0; i < REPEAT_COUNT; ++i) {
        size_t received_length = receive_message(socket_fd, buffer, sizeof(buffer), NO_FLAGS);
        printf("read %zd bytes: %.*s\n", received_length, (int) received_length, buffer);
    }
    /* odłączenie od grupy rozsyłania */
    CHECK_ERRNO(setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &ipv6_mreq, sizeof(ipv6_mreq)));

    /* koniec */
    CHECK_ERRNO(close(socket_fd));
}
