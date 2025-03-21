#pragma once
#include <iostream>
#include "../include/HttpConnection.h"
#include <map>
#include <memory>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>

class HttpConnection;
class HttpServer{
public:
    HttpServer(boost::asio::io_context &io_context, unsigned short port);
    void CloseConnection(const std::string &connection_id);
private:
    void WaitAccept();
    std::string GenerateConnectionId();

    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<HttpConnection>> _connections;
};