#ifndef UDPSENDER_HPP
#define UDPSENDER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string>

class UdpSender
{
private:
    struct addrinfo *result;
public:
    UdpSender(std::string hostname, std::string service);
    ~UdpSender();
    void send(int data1);
};

#endif // UDPSENDER_HPP
