#pragma once
#include <iostream>
#include <boost/asio.hpp>

class CSession{
public:
    CSession(boost::asio::io_context &io_context);
    boost::asio::ip::tcp::socket& GetSocket();
    void Start();
private:
    void HandleRead(const boost::system::error_code &error, size_t pack_size);
    void HandleWrite(const boost::system::error_code &error);
    
    boost::asio::ip::tcp::socket _socket;
    enum { _PACK_MAX_SIZE = 1024};
    char _recv_pack_data[_PACK_MAX_SIZE];
};