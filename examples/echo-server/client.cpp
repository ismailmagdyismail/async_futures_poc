
#include <iostream>
#include <thread>
#include <chrono>

#include "TCPClientSocket.h"
#include "SocketReadFuture.h"
#include "SocketWriteFuture.h"
#include "DoneFuture.h"

int main()
{
    int port = 8080;
    RunTime runtime;
    TCPClientSocket socket(&runtime);

    unsigned int size = 1024;
    char buffer[size];
    std::string message = "Hello from client " + std::to_string(getpid());

    socket.Connect(port)->Then([&](SocketConnectFuture::DataType ){
        std::cerr <<"Client connect to server successfully " << std::endl;
        return socket.Write(message.data(), message.size());
    })
    ->Then([&](SocketWriteFuture::DataType data){
        std::cerr <<"Written bytes = "<< data <<std::endl;
        return socket.Read(buffer, size);
    })
    ->Then([&](SocketReadFuture::ReadResult data){
        std::cerr <<"Recieved message "<< std::string_view(data.buffer, data.bytesRead) << std::endl;
        runtime.Stop();
        return new Future<DoneFuture>(std::in_place, &runtime);
    });
    
    runtime.Run();

    return 0;
}
