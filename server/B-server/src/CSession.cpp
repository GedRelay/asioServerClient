#include "../include/CSession.h"

CSession::CSession(boost::asio::io_context &io_context):
    _socket(io_context){
}

boost::asio::ip::tcp::socket& CSession::GetSocket(){
    return _socket;
}

// 开始异步读，由Server调用
void CSession::Start(){
    // 注册读事件
    _socket.async_read_some(
        boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
        [this](const boost::system::error_code &error, size_t recv_pack_size){
            HandleRead(error, recv_pack_size);
        }
    );
}

// 读操作完成后的回调函数
void CSession::HandleRead(const boost::system::error_code &error, size_t recv_pack_size){
    if(!error){
        // 对读取到的消息进行处理（进行打印）
        std::cout << "receive message: ";
        std::cout.write(_recv_pack_data, recv_pack_size);
        std::cout << std::endl;

        // 准备发送给客户端的数据（收到的消息后面加一个!号作为发送的信息)
        _recv_pack_data[recv_pack_size] = '~';

        // 注册写事件
        boost::asio::async_write(
            _socket,
            boost::asio::buffer(_recv_pack_data, recv_pack_size + 1),
            [this](const boost::system::error_code &error, size_t send_pack_size){
                HandleWrite(error);
            }
        );
    }
    else{
        std::cerr << "Read msg error: " << error.message() << std::endl;
        delete this;
    }
}

// 写操作完成后的回调函数
void CSession::HandleWrite(const boost::system::error_code &error){
    if(!error){
        // 注册读事件
        _socket.async_read_some(
            boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
            [this](const boost::system::error_code &error, size_t recv_pack_size){
                HandleRead(error, recv_pack_size);
            }
        );
    }
    else{
        std::cerr << "Send msg error: " << error.message() << std::endl;
        delete this;
    }
}