#include "../include/WebSocketServer.h"

WebSocketServer::WebSocketServer(boost::asio::io_context& io_context, unsigned short port):
	_io_context(io_context), 
	_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
	StartAccept();
}

void WebSocketServer::StartAccept(){
	auto connection = std::make_shared<Connection>(_io_context);
	_acceptor.async_accept(
		connection->GetSocket(), 
		[this, connection](boost::beast::error_code error){
			try{
				if (!error) {
					connection->AsyncAccept();
				}
				else {
					std::cout << "acceptor async_accept failed, err is " << error.what() << std::endl;
				}
				StartAccept();
			}
			catch (std::exception& e) {
				std::cout << "async_accept error is " << e.what() << std::endl;
			}
		}
	);
}