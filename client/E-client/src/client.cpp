#include <iostream>
#include <cstring>
#include <boost/asio.hpp>
#include "../include/CMessageNode.h"

const size_t BUF_MAX_SIZE = 4096;

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
            
            // 将发送数据封装成 CSendMessageNode
            CSendMessageNode send_msg_node(1001, buf, msg_len);

            // 向服务器发送数据
            boost::asio::write(socket, boost::asio::buffer(send_msg_node.GetData(), send_msg_node.GetDataLen()));

            // 从服务器接受数据
            // 1. 接收消息头
            CRecvMessageNode recv_msg_node;
            boost::asio::read(socket, boost::asio::buffer(buf, CRecvMessageNode::_HEAD_LEN));
            recv_msg_node.AppendData(buf, CRecvMessageNode::_HEAD_LEN);
            // 2. 接收消息体
            std::cout << "msg_len" << msg_len << std::endl;
            msg_len = recv_msg_node.GetMsgLen();
            boost::asio::read(socket, boost::asio::buffer(buf, msg_len));
            recv_msg_node.AppendData(buf, msg_len);

            // 处理接收到的数据（进行打印）
            std::cout << "Received from server (msg_id=";
            std::cout << recv_msg_node.GetMsgId();
            std::cout << ",msg_len=";
            std::cout << recv_msg_node.GetMsgLen();
            std::cout << "): ";
            std::cout.write(recv_msg_node.GetMsgData(), recv_msg_node.GetMsgLen());
            std::cout << std::endl;
        }
    }
    catch(std::exception &e){
        throw e;
    }
}