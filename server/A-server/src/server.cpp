#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
#include <thread>

using socket_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;
std::set<std::shared_ptr<std::thread>> thread_set;
const size_t BUF_MAX_SIZE = 1024;  // 接收缓冲区最大大小

void session(socket_ptr sock);

int main(){
    std::cout << "Server Started" << std::endl;
    unsigned short port = 10086;
    try{
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        boost::asio::ip::tcp::acceptor acceptor(io_context, endpoint);
        while(true){
            // 创建socket对象
            socket_ptr sock = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
            // 等待接受新的连接
            acceptor.accept(*sock);
            std::cout << "Connection from: " << sock->remote_endpoint().address().to_string() << std::endl;
            // 为每个连接创建一个线程，在新线程中处理连接
            auto t = std::make_shared<std::thread>(session, sock);
            thread_set.insert(t);
        }
        for(auto &t : thread_set){
            t->join();
        }
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

// 处理单个连接
void session(socket_ptr sock){
    // 准备用于接受消息的缓冲区
    char buf[BUF_MAX_SIZE];
    boost::system::error_code error;
    try{
        while(true){
            // 从客户端接收消息
            size_t msg_len = sock->read_some(boost::asio::buffer(buf), error);
            if(error){
                if(error == boost::asio::error::eof){
                    std::cout << "Connection closed by peer." << std::endl;
                    break;
                }
                throw boost::system::system_error(error);
            }
            // 处理接收到的消息
            std::cout << "Received from " << sock->remote_endpoint().address().to_string() << ": ";
            std::cout.write(buf, msg_len);
            std::cout << std::endl;
            // 向客户端发送消息(原样发回)
            boost::asio::write(*sock, boost::asio::buffer(buf, msg_len));
        }
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
}