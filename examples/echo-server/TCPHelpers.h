#pragma once

#include <fcntl.h>
#include <arpa/inet.h>

void MakeNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}


sockaddr_in CreateLocalAddress(unsigned int port)
{
    sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    address.sin_family = AF_INET;

    return address;
}