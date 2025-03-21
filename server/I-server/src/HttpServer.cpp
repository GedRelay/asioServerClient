#include "../include/HttpServer.h"

HttpServer::HttpServer(boost::asio::io_context &io_context, unsigned short port):
    _io_context(io_context),
    _acceptor(_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
    WaitAccept();
}


void HttpServer::WaitAccept(){
    std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_context);
    // 注册连接事件
    _acceptor.async_accept(
        *socket,
        [this, socket](const boost::system::error_code &error){
            if(!error){
                std::string connection_id = GenerateConnectionId();
                std::shared_ptr<HttpConnection> connection = std::make_shared<HttpConnection>(connection_id, socket, this, 5);
                _connections[connection_id] = connection;  // 将connection存入map
                std::cout << "New connection: " << connection_id << std::endl;
                connection->Start();
                WaitAccept();
            }
            else{
                std::cout << "Error: " << error.message() << std::endl;
            }
        }
    );
}


// 为每个连接生成一个唯一id
std::string HttpServer::GenerateConnectionId(){
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

void HttpServer::CloseConnection(const std::string &connection_id){
    _connections.erase(connection_id);
}