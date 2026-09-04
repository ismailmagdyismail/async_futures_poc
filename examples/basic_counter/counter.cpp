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