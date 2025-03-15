#include <iostream>
#include <boost/asio.hpp>
#include "../include/CServer.h"

int main(){
    std::cout << "Server Started" << std::endl;
    try{
        unsigned short port = 10086;
        boost::asio::io_context io_context;
        CServer server(io_context, port);
        io_context.run();
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}