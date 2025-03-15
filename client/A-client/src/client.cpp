#include <iostream>
#include <cstring>
#include <boost/asio.hpp>

const size_t BUF_MAX_SIZE = 1024;

// 与服务器进行通信
void session(boost::asio::ip::tcp::socket &socket);

int main(){
    try{
        std::string target_ip = "127.0.0.1";
        unsigned short target_port = 10086;
        // 创建socket对象
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        // 构造连接端点
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(target_ip), target_port);
        // 连接服务器
        boost::system::error_code error = boost::asio::error::host_not_found;
        socket.connect(endpoint, error);
        if(error){
            std::cerr << error.what() << std::endl;
            return 0;
        }
        // 与服务器进行通信
        session(socket);
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

void session(boost::asio::ip::tcp::socket &socket){
    try{
        char buf[BUF_MAX_SIZE];
        while(true){
            // 准备发送数据（从终端读取）
            std::cout << "Enter message (q to quit): ";
            std::cin.getline(buf, BUF_MAX_SIZE);
            size_t msg_len = strlen(buf);
            if(msg_len == 1 && buf[0] == 'q') break;

            // 向服务器发送数据
            boost::asio::write(socket, boost::asio::buffer(buf, msg_len));

            // 从服务器接受数据
            msg_len = socket.read_some(boost::asio::buffer(buf));

            // 处理接收到的数据（进行打印）
            std::cout << "Received from server: ";
            std::cout.write(buf, msg_len);
            std::cout << std::endl;
        }
    }
    catch(std::exception &e){
        throw e;
    }
}