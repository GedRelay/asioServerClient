#include "../include/CServer.h"


CServer::CServer(boost::asio::io_context &io_context, unsigned short port):
    _io_context(io_context),
    _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
    WaitAccept();
}


// 异步等待连接
void CServer::WaitAccept(){
    CSession * session = new CSession(_io_context);
    // 注册连接事件
    _acceptor.async_accept(
        session->GetSocket(), 
        [this, session](const boost::system::error_code &error){
            if(!error){
                std::cout << "new session connected" << std::endl;
                session->Start();
                WaitAccept();
            }
            else{
                delete session;
            }
        }
    );
}
