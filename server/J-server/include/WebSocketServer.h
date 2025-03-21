#pragma once
#include "ConnectionManager.h"

class WebSocketServer
{
public:
	WebSocketServer(const WebSocketServer&) = delete;
	WebSocketServer& operator = (const WebSocketServer&) = delete;
	
	WebSocketServer(boost::asio::io_context& io_context, unsigned short port);
	void StartAccept();
private:
	boost::asio::ip::tcp::acceptor _acceptor;
	boost::asio::io_context & _io_context;
};