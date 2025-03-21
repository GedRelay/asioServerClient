#include "../include/HttpConnection.h"

size_t HttpConnection::RequestCount = 0;

std::string HttpConnection::GetNowString(){
    time_t now = time(0);
    char *dt = ctime(&now);
    return std::string(dt);
}

HttpConnection::HttpConnection(std::string connection_id, std::shared_ptr<boost::asio::ip::tcp::socket> socket, HttpServer* server, unsigned short timeout):
    _socket(socket),
    _connection_id(connection_id),
    _server(server),
    _deadline(_socket->get_executor(), std::chrono::seconds(timeout)){
}

HttpConnection::~HttpConnection(){
    std::cout << "Connection " << _connection_id << " destructed" << std::endl;
}

void HttpConnection::Start(){
    ReadRequest();
    CheckDeadline();
}

void HttpConnection::ReadRequest(){
    boost::beast::http::async_read(
        *_socket,
        _buffer,
        _request,
        [self = shared_from_this()](boost::beast::error_code error, size_t bytes_transferred){
            boost::ignore_unused(bytes_transferred);  // 将未使用的变量标记为已使用
            if(!error){
                HttpConnection::RequestCount++;  // 请求计数加1
                self->ProcessRequest();
            }
            else{
                self->_server->CloseConnection(self->_connection_id);
                self->_socket->close();
            }
        }
    );
}


// 对请求进行处理
void HttpConnection::ProcessRequest(){
    _response.version(_request.version());  // 设置响应版本为请求版本一致
    _response.keep_alive(false);  // 设置短连接

    switch (_request.method()){
    case boost::beast::http::verb::get:  // 处理GET请求
        _response.result(boost::beast::http::status::ok);
        _response.set(boost::beast::http::field::server, "Beast");
        CreateGetResponse();
        break;
    case boost::beast::http::verb::post:  // 处理POST请求
        _response.result(boost::beast::http::status::ok);
        _response.set(boost::beast::http::field::server, "Beast");
        CreatePostResponse();
        break;
    default:  // 处理其他请求
        _response.result(boost::beast::http::status::bad_request);
        _response.set(boost::beast::http::field::content_type, "text/plain");
        boost::beast::ostream(_response.body())
            << "Invalid request-method '"
            << std::string(_request.method_string())
            << "'";
        break;
    }
    // 处理完请求后，发送响应
    WriteResponse();
}


// 发送GET响应，根据请求的目标不同，返回不同的响应
void HttpConnection::CreateGetResponse(){
    if (_request.target() == "/count"){
        _response.set(boost::beast::http::field::content_type, "text/html");
        boost::beast::ostream(_response.body())
            << "<html>\n"
            << "<head><title>Request count</title></head>\n"
            << "<body>\n"
            << "<h1>Request count</h1>\n"
            << "<p>There have been "
            << HttpConnection::RequestCount
            << " requests so far.</p>\n"
            << "</body>\n"
            << "</html>\n";
    }
    else if (_request.target() == "/time"){
        _response.set(boost::beast::http::field::content_type, "text/html");
        boost::beast::ostream(_response.body())
            << "<html>\n"
            << "<head><title>Current time</title></head>\n"
            << "<body>\n"
            << "<h1>Current time</h1>\n"
            << "<p>The current time is "
            << HttpConnection::GetNowString()
            << " seconds since the epoch.</p>\n"
            << "</body>\n"
            << "</html>\n";
    }
    else{
        _response.result(boost::beast::http::status::not_found);
        _response.set(boost::beast::http::field::content_type, "text/plain");
        boost::beast::ostream(_response.body()) << "File not found\r\n";
    }
}


// 发送POST响应，根据请求的目标不同，返回不同的响应
void HttpConnection::CreatePostResponse(){
    if (_request.target() == "/email"){
        auto body_str = boost::beast::buffers_to_string(_request.body().data());  // 将请求体转为字符串
        std::cout << "receive body is " << body_str << std::endl;
        _response.set(boost::beast::http::field::content_type, "text/json");
        Json::Value response_json;
        Json::Reader reader;
        Json::Value request_json;
        bool parse_success = reader.parse(body_str, request_json);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            response_json["error"] = 1001;
            std::string jsonstr = response_json.toStyledString();
            boost::beast::ostream(_response.body()) << jsonstr;
            return ;
        }

        auto email = request_json["email"].asString();
        std::cout << "email is " << email << std::endl;

        response_json["error"] = 0;
        response_json["email"] = request_json["email"];
        response_json["msg"] = "recevie email post success";
        std::string jsonstr = response_json.toStyledString();
        boost::beast::ostream(_response.body()) << jsonstr;
    }
    else{
        _response.result(boost::beast::http::status::not_found);
        _response.set(boost::beast::http::field::content_type, "text/plain");
        boost::beast::ostream(_response.body()) << "File not found\r\n";
    }
}


// 发送响应
void HttpConnection::WriteResponse(){
    _response.content_length(_response.body().size());
    boost::beast::http::async_write(
        *_socket,
        _response,
        [self = shared_from_this()](boost::beast::error_code error, size_t bytes_transferred){
            if(!error){
                self->_server->CloseConnection(self->_connection_id);
                self->_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
                self->_deadline.cancel();
            }
            else{
                self->_server->CloseConnection(self->_connection_id);
                self->_socket->close();
            }
        }
    );
}


// 检查超时
void HttpConnection::CheckDeadline(){
    _deadline.async_wait(  // 当超时时，关闭socket
        [self = shared_from_this()](boost::beast::error_code error){
            if(!error){
                self->_server->CloseConnection(self->_connection_id);
                self->_socket->close();
            }
        }
    );
}