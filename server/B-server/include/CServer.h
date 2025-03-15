#pragma once
#include <boost/asio.hpp>
#include "CSession.h"


class CServer{
public:
    CServer(boost::asio::io_context &io_context, unsigned short port);
private:
    void WaitAccept();

    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::io_context &_io_context;
};