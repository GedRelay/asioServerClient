#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "CServer.h"

class CServer;
class CSession: public std::enable_shared_from_this<CSession>{  // 继承自enable_shared_from_this，用于在回调函数中获取this的shared_ptr
public:
    CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string session_id, CServer *server);
    ~CSession();
    void Start();
private:
    void HandleRead(const boost::system::error_code &error, size_t pack_size, std::shared_ptr<CSession> self_ptr);  // 读操作完成后的回调函数
    void HandleWrite(const boost::system::error_code &error, std::shared_ptr<CSession> self_ptr);  // 写操作完成后的回调函数
    
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    enum { _PACK_MAX_SIZE = 2048 };
    char _recv_pack_data[_PACK_MAX_SIZE];  // 用于存放接收到的数据
    CServer* _server;  // 保存server指针，用于清除session
    std::string _session_id;  // 该session的id
};