#include "../include/CServer.h"


CServer::CServer(boost::asio::io_context &io_context, unsigned short port):
    _io_context(io_context),
    _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
    WaitAccept();
}


// 异步等待连接
void CServer::WaitAccept(){
    auto &io_context = AsioIOContextPool::GetInstance()->GetIOContext();  // 使用IOContextPool里的io_context
    std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    // 注册连接事件
    _acceptor.async_accept(
        *socket,
        [this, socket](const boost::system::error_code &error){
            if(!error){
                std::string session_id = GenerateSessionId();
                std::shared_ptr<CSession> session = std::make_shared<CSession>(socket, session_id, this);
                _sessions[session_id] = session;  // 将session存入map
                std::cout << "New session: " << session_id << std::endl;
                session->Start();
                WaitAccept();
            }
            else{
                std::cout << "Error: " << error.message() << std::endl;
            }
        }
    );
}


// 为每个Session生成一个唯一id
std::string CServer::GenerateSessionId(){
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}


// 清除指定id的session
void CServer::ClearSession(const std::string &session_id) {
    _sessions.erase(session_id);
}