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
        fatal("Usage: %s multicast_dotted_address local_port\n", argv[0]);
    char *multicast_dotted_address = argv[1];
    uint16_t port = read_port(argv[2]);

    /* otwarcie gniazda */
    int socket_fd = open_udp_socket();

    /* podłączenie do grupy rozsyłania (ang. multicast) */
    struct ip_mreq ip_mreq;
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(multicast_dotted_address, &ip_mreq.imr_multiaddr) == 0) {
        fatal("inet_aton - invalid multicast address\n");
    }

    CHECK_ERRNO(setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq));

    /* ustawienie adresu i portu lokalnego */
    bind_socket(socket_fd, port);

    /* czytanie tego, co odebrano */
    for (int i = 0; i < REPEAT_COUNT; ++i) {
        size_t received_length = receive_message(socket_fd, buffer, sizeof(buffer), NO_FLAGS);
        printf("read %zd bytes: %.*s\n", received_length, (int) received_length, buffer);
    }
    /* odłączenie od grupy rozsyłania */
    CHECK_ERRNO(setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &ip_mreq, sizeof(ip_mreq)));

    /* koniec */
    CHECK_ERRNO(close(socket_fd));
}
