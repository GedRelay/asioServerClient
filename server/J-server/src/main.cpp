#include "../include/WebSocketServer.h"

int main(){
	try{
		unsigned short port = 10086;
		boost::asio::io_context io_context;

		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait(
			[&io_context](auto, auto) {
				io_context.stop();
			}
		);
		
		WebSocketServer server(io_context, port);
		
		std::cout << "Server Started on port " << port << std::endl;
		io_context.run();
		std::cout << " Server Closed" << std::endl;
	}
	catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}