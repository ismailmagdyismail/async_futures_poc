#include <iostream>

//! Futures includes
#include "RunTime.h"
#include "SocketAcceptFuture.h"
#include "DoneFuture.h"
#include "SocketWriteFuture.h"
#include "TCPServerSocket.h"
#include "TCPClientSocket.h"

RunTime runtime;
TCPServerSocket server(&runtime);

Future<DoneFuture> *HandleNewConnection(SocketInfo *info)
{
    std::cerr << "New connection accepted!" << std::endl;
    
    //! Create connection
    auto clientConnection =  new TCPClientSocket(&runtime, info);
    

    //! Asynchronously echo back message
    clientConnection
    ->Read(new char[1024], 1024)
    ->Then([ptr = clientConnection](SocketReadFuture::DataType readResult){
        std::cerr << "Socket read completed, bytes read: " << readResult.bytesRead << std::endl;
        std::cerr << "Read bytes: " << std::string_view(readResult.buffer, readResult.bytesRead) << std::endl;
        return ptr->Write(readResult.buffer, readResult.bytesRead);
    })
    ->Then([&](SocketWriteFuture::DataType data){
        std::cerr << "Echoed back bytes count "<< data << std::endl;
        return new Future<DoneFuture>(std::in_place, &runtime);
    });


    //! keep chaining an accept future to handle new connections
    server.Accept()->Then([&](SocketInfo *info) { return HandleNewConnection(info); });
    return new Future<DoneFuture>(std::in_place, &runtime);
}

int main()
{

    if (!server.Listen(8080))
    {
        std::cerr << "Error Listening on Server Socket with port 8080" << std::endl;
        return 1;
    }

    server.Accept()
        ->Then([&](SocketInfo *info)
               { return HandleNewConnection(info); });

    runtime.Run();
}