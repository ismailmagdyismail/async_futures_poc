#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "Future.h"
#include "IncrementFuture.h"
#include "DecrementFuture.h"
#include "DoneFuture.h"
#include "SocketReadFuture.h"
#include "SocketAcceptFuture.h"
#include "FutureHandle.h"

int main()
{
    RunTime runtime;
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(serverFD, F_SETFL, O_NONBLOCK);
    sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    address.sin_family = AF_INET;

    int bindStatus = bind(serverFD, reinterpret_cast<sockaddr *>(&address), sizeof(address));
    if (bindStatus == -1)
    {
        std::cerr << "Error binding socket: with code " << errno << " and message: " << strerror(errno) << std::endl;
        return 1;
    }
    int listenStatus = listen(serverFD, 0);
    if (listenStatus != 0)
    {
        std::cerr << "Error Listening on Server Socket with port 8080" << std::endl;
        return 1;
    }

    Future<SocketAcceptFuture> *acceptFuture = new Future<SocketAcceptFuture>(std::in_place, &runtime, new SocketInfo{serverFD});
    acceptFuture
    ->Then([&](SocketInfo *info) { 
        return new Future<SocketReadFuture>(std::in_place, &runtime, 1024, info);
    })
    ->Then([&](SocketReadFuture::ReadResult readResult) {
        std::cerr << "Socket read completed, bytes read: " << readResult.bytesRead << std::endl;
        std::cerr << "Read bytes: " << std::string_view(readResult.buffer, readResult.bytesRead) << std::endl;
        return new Future<DoneFuture>(std::in_place, &runtime);
    });

    auto incFuture = new Future<IncrementFuture>(std::in_place, &runtime, 10, 0);
    incFuture
    ->Then([&](IncrementFuture::DataType value){
        std::cerr << " Increment future completed with value: " << value << std::endl;
        return new Future<DoneFuture>(std::in_place, &runtime); 
    })
    ->Then([&](DoneFuture::DataType value){
        std::cerr << " Done future completed with value: " << value << std::endl;
        return new Future<DecrementFuture>(std::in_place, &runtime, 0, 10); 
    })
    ->Then([&](DecrementFuture::DataType value){
        std::cerr << " Decrement future completed with value: " << value << std::endl;
        return new Future<DoneFuture>(std::in_place, &runtime);
    });
    runtime.Run();
}