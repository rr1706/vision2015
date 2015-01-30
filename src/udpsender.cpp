#include <stdexcept>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include "udpsender.hpp"

UdpSender::UdpSender(std::string hostname, std::string service)
{
    struct addrinfo hints;
    int s;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &this->result);
    if (s != 0) {
        throw std::runtime_error(gai_strerror(s));
    }
}

UdpSender::~UdpSender()
{
    freeaddrinfo(this->result);
}

void UdpSender::send(Game_Piece piece)
{
    struct addrinfo *rp;
    int sock;
    char buf[255];

    sprintf(buf, "%f %f %f %d %d %d\n",
            piece.get_xrot(),       // same as in ultimate ascent
            piece.get_distance(),   // distance away in centimeters
            piece.get_rotation(),   // square-up rotation
            piece.get_green_bin(),  // 1 - has green bin on top, 0 - no green bin on top
            piece.get_piece_type(), // 0 - unknown, 1 - gray, 2 - yellow, 3 - green bin solo
            piece.get_totes_high()  // 0 - 6, totes in stack
            );
    printf("[UDP] %s\n", buf);

    for (rp = this->result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        if (sendto(sock, buf, strlen(buf), 0, rp->ai_addr, rp->ai_addrlen) == -1) {
            throw std::runtime_error("Sending UDP message to RoboRIO failed: " + std::string(strerror(errno)));
        }

        close(sock);
    }
}
