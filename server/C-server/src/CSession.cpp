#include "../include/CSession.h"

CSession::CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string session_id, CServer *server):
    _socket(socket),
    _session_id(session_id),
    _server(server){
}

CSession::~CSession(){
    std::cout << "Session " << _session_id << " destructed" << std::endl;
}

// 开始异步读，由Server调用
void CSession::Start(){
    // 注册读事件
    _socket->async_read_some(
        boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
        [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t recv_pack_size){
            HandleRead(error, recv_pack_size, self_ptr);
        }
    );
}

// 读操作完成后的回调函数
void CSession::HandleRead(const boost::system::error_code &error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr){
    if(!error){
        // 对读取到的消息进行处理（进行打印）
        std::cout << _session_id.substr(24) << ": ";
        std::cout.write(_recv_pack_data, recv_pack_size);
        std::cout << std::endl;

        // 准备发送给客户端的数据（收到的消息后面加一个!号作为发送的信息)
        _recv_pack_data[recv_pack_size] = '~';

        // 注册写事件
        boost::asio::async_write(
            *_socket,
            boost::asio::buffer(_recv_pack_data, recv_pack_size + 1),
            [this, self_ptr](const boost::system::error_code &error, size_t send_pack_size){
                HandleWrite(error, self_ptr);
            }
        );
    }
    else{
        std::cerr << "Read msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}

// 写操作完成后的回调函数
void CSession::HandleWrite(const boost::system::error_code &error, std::shared_ptr<CSession> self_ptr){
    if(!error){
        // 注册读事件
        _socket->async_read_some(
            boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
            [this, self_ptr](const boost::system::error_code &error, size_t recv_pack_size){
                HandleRead(error, recv_pack_size, self_ptr);
            }
        );
    }
    else{
        std::cerr << "Send msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}