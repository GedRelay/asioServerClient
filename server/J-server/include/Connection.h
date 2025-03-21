#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <mutex>

class Connection:public std::enable_shared_from_this<Connection>{
public:
	Connection(boost::asio::io_context& io_context);
	std::string  GetUid();
	boost::asio::ip::tcp::socket& GetSocket();
	void AsyncAccept();
	void Start();
	void AsyncSend(std::string msg);
	void SendCallBack(std::string msg);
private:
	std::unique_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> _ws_ptr;
	std::string  _uuid;
	boost::asio::io_context& _io_context;
	boost::beast::flat_buffer _recv_buffer;
	std::queue<std::string> _send_queue;
	std::mutex _send_mtx;
};

