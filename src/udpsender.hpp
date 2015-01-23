#ifndef UDPSENDER_HPP
#define UDPSENDER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string>

#include "yellow.hpp"

class UdpSender
{
private:
    struct addrinfo *result;
public:
    UdpSender(std::string hostname, std::string service);
    ~UdpSender();
    void send(Game_Piece piece);
};

#endif // UDPSENDER_HPP
