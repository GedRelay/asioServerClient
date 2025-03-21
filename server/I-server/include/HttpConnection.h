#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include "../include/HttpServer.h"


class HttpServer;
class HttpConnection: public std::enable_shared_from_this<HttpConnection>{
public:
    HttpConnection(std::string connection_id, std::shared_ptr<boost::asio::ip::tcp::socket> socket, HttpServer* server, unsigned short timeout);
    ~HttpConnection();
    void Start();

    static size_t RequestCount;
    static std::string GetNowString();

private:
    void ReadRequest();
    void CheckDeadline();
    void ProcessRequest();
    void CreateGetResponse();
    void CreatePostResponse();
    void WriteResponse();

    HttpServer* _server;
    std::string _connection_id;
    boost::beast::flat_buffer _buffer{ 8192 };
    boost::beast::http::request<boost::beast::http::dynamic_body> _request;
    boost::beast::http::response<boost::beast::http::dynamic_body> _response;
    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::steady_timer _deadline;
};
