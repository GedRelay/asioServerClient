#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "../include/HttpServer.h"

int main(){
    try{
        unsigned short port = 8080;  // 127.0.0.1:8080/time
        boost::asio::io_context io_context {1};
        // 使用信号监听对象监听SIGINT（ctrl+c）和SIGTERM（kill）信号
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context](auto, auto) {
                io_context.stop();
            }
        );
        
        HttpServer server(io_context, port);
        std::cout << "Server Started" << std::endl;
        io_context.run();  // 程序在此阻塞，直到io_context.stop()被调用
        std::cout << "Server Closed" << std::endl;
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}