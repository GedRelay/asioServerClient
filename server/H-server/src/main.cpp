#include <iostream>
#include <boost/asio.hpp>
#include "../include/CServer.h"
#include "../include/AsioIOContextPool.h"

int main(){
    try{
        unsigned short port = 10086;
        std::shared_ptr<AsioIOContextPool> io_contest_pool = AsioIOContextPool::GetInstance();
        boost::asio::io_context io_context;
        // 使用信号监听对象监听SIGINT（ctrl+c）和SIGTERM（kill）信号
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context, io_contest_pool](auto, auto) {
                io_context.stop();
                io_contest_pool->Stop();
            }
        );
        CServer server(io_context, port);
        std::cout << "Server Started" << std::endl;
        io_context.run();  // 程序在此阻塞，直到io_context.stop()被调用
        std::cout << "Server Closed" << std::endl;
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}