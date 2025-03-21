#include "../include/Connection.h"
#include "../include/ConnectionManager.h"
Connection::Connection(boost::asio::io_context& io_context):
	_io_context(io_context),
	_ws_ptr(std::make_unique<boost::beast::websocket::stream<boost::beast::tcp_stream>>(make_strand(io_context))){
	// 生成随机的 UUID
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	_uuid = boost::uuids::to_string(uuid);
}

std::string Connection::GetUid(){
	return _uuid;
}

boost::asio::ip::tcp::socket& Connection::GetSocket(){
	auto &socket = boost::beast::get_lowest_layer(*_ws_ptr).socket();
	return socket;
}

void Connection::AsyncAccept(){
	_ws_ptr->async_accept(
		[self = shared_from_this()](boost::beast::error_code error) {
			try {
				if (!error) {
					ConnectionManager::GetInstance().AddConnection(self);
					self->Start();
				}
				else {
					std::cout << "websocket accept failed, err is " << error.what() << std::endl;
				}
			}
			catch (std::exception& e) {
				std::cout << "websocket async accept exception is " << e.what();
			}
		}
	);
}

void Connection::Start(){
	_ws_ptr->async_read(
		_recv_buffer, 
		[self = shared_from_this()](boost::beast::error_code  error, std::size_t  buffer_bytes) {
			try {
				if (error) {	
					std::cout << "websocket async read error is " << error.what() << std::endl;
					ConnectionManager::GetInstance().RmvConnection(self->GetUid());
					return;
				}
				self->_ws_ptr->text(self->_ws_ptr->got_text());
				std::string recv_data = boost::beast::buffers_to_string(self->_recv_buffer.data());
				self->_recv_buffer.consume(self->_recv_buffer.size());
				std::cout << "websocket receive msg is " << recv_data << std::endl;
				self->AsyncSend(std::move(recv_data));
				self->Start();
			}
			catch (std::exception& e) {
				std::cout << "exception is " << e.what() << std::endl;
				ConnectionManager::GetInstance().RmvConnection(self->GetUid());
			}
		}
	);
}

void Connection::AsyncSend(std::string msg){
	{
		std::lock_guard<std::mutex> lck_gurad(_send_mtx);
		int que_len = _send_queue.size();
		_send_queue.push(msg);
		if (que_len > 0) {
			return;
		}
	}
	SendCallBack(std::move(msg));
}


void Connection::SendCallBack(std::string msg){
	_ws_ptr->async_write(
		boost::asio::buffer(msg.c_str(), msg.length()),
		[self = shared_from_this()](boost::beast::error_code  err, std::size_t  nsize) {
			try {
				if (err) {
					std::cout << "async send err is " << err.what() << std::endl;
					ConnectionManager::GetInstance().RmvConnection(self->_uuid);
					return;
				}
				std::string send_msg;
				{
					std::lock_guard<std::mutex> lck_gurad(self->_send_mtx);
					self->_send_queue.pop();
					if (self->_send_queue.empty()) {
						return;
					}

					send_msg = self->_send_queue.front();
				}

				self->SendCallBack(std::move(send_msg));
			}
			catch (std::exception& exp) {
				std::cout << "async send exception is " << exp.what() << std::endl;
				ConnectionManager::GetInstance().RmvConnection(self->_uuid);
			}
		}
	);
}
