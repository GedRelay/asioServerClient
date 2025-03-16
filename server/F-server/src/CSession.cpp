#include "../include/CSession.h"

CSession::CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string session_id, CServer *server):
    _socket(socket),
    _session_id(session_id),
    _server(server){
    _cur_recv_msg = std::make_shared<CRecvMessageNode>();
}


CSession::~CSession(){
    std::cout << "Session " << _session_id << " destructed" << std::endl;
}


// 开始异步读，由Server调用
void CSession::Start(){
    // 注册读事件
    boost::asio::async_read(
        *_socket,
        boost::asio::buffer(_recv_pack_data, CRecvMessageNode::_HEAD_LEN),
        [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t recv_pack_size){
            HandleReadHead(error, recv_pack_size, self_ptr);
        }
    );
}


// 发送消息
void CSession::SendMsg(uint16_t msg_id, char *msg, uint16_t msg_len){
    std::lock_guard<std::mutex> lock(_msg_queue_mutex);
    bool is_sending = !_sending_msg_queue.empty();  // 如果发送信息队列里还有消息说明有消息正在发送
    _sending_msg_queue.push(std::make_shared<CSendMessageNode>(msg_id, msg, msg_len));  // 将消息打包为发送结点后加入发送消息队列
    if(is_sending) return;
    // 如果没有消息正在发送，则注册发送消息事件
    boost::asio::async_write(
        *_socket,
        boost::asio::buffer(_sending_msg_queue.front()->GetData(), _sending_msg_queue.front()->GetDataLen()),
        [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t send_pack_size) {
            HandleWrite(error, self_ptr);
        }
    );
}


// 读取完消息头后的回调函数
void CSession::HandleReadHead(const boost::system::error_code &error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr){
    if(!error){
        _cur_recv_msg->AppendData(_recv_pack_data, recv_pack_size);
        uint16_t msg_id = _cur_recv_msg->GetMsgId();
        uint16_t msg_len = _cur_recv_msg->GetMsgLen();
        if(msg_id != 901 && msg_id != 1001){  // 消息id不是901且不是1001，直接断开连接
            std::cerr << "Invalid message id" << std::endl;
            _server->ClearSession(_session_id);
            return;
        }
        if(msg_len > CRecvMessageNode::_MAX_MSG_LENGTH){  // 消息长度超过消息最大长度，直接断开连接
            std::cerr << "Message length exceeds the maximum length" << std::endl;
            _server->ClearSession(_session_id);
            return;
        }
        if(msg_len == 0){  // 消息长度为0，直接断开连接
            std::cerr << "Message length is zero" << std::endl;
            _server->ClearSession(_session_id);
            return;
        }
        // 接收到的消息头正常，继续注册读事件读取消息体
        boost::asio::async_read(
            *_socket,
            boost::asio::buffer(_recv_pack_data, msg_len),
            [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t recv_pack_size){
                HandleReadMsg(error, recv_pack_size, self_ptr);
            }
        );
    }
    else{
        std::cerr << "Read head error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}


// 读取完消息体后的回调函数
void CSession::HandleReadMsg(const boost::system::error_code &error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr){
    if(!error){
        _cur_recv_msg->AppendData(_recv_pack_data, recv_pack_size);
        // 处理接收到的数据（打印消息内容，然后将其返回给客户端）
        uint16_t msg_id = _cur_recv_msg->GetMsgId();
        uint16_t msg_len = _cur_recv_msg->GetMsgLen();
        if(msg_id == 1001){
            Json::Value root;
            Json::Reader reader;
            reader.parse(_cur_recv_msg->GetMsgData(), _cur_recv_msg->GetMsgData() + _cur_recv_msg->GetMsgLen(), root);
            std::cout << _session_id.substr(24) << " (msg_id=";
            std::cout << root["msg_id"];
            std::cout << ",msg_len=";
            std::cout << msg_len;
            std::cout << ",user_name=";
            std::cout << root["user_name"];
            std::cout << "): ";
            std::cout << root["msg"];
            std::cout << std::endl;
        }
        else if(msg_id == 901){
            Msg901 msg901;
            msg901.ParseFromArray(_cur_recv_msg->GetMsgData(), _cur_recv_msg->GetMsgLen());
            std::cout << _session_id.substr(24) << " (msg_id=";
            std::cout << msg901.msg_id();
            std::cout << ",msg_len=";
            std::cout << msg_len;
            std::cout << ",machine_id=";
            std::cout << msg901.machine_id();
            std::cout << "): ";
            std::cout << msg901.msg();
            std::cout << std::endl;
        }

        // 准备发送给对端的数据（直接将原消息发回）
        SendMsg(msg_id, _recv_pack_data, recv_pack_size);

        // 处理完这条消息继续监听消息头
        _cur_recv_msg->Reset();
        boost::asio::async_read(
            *_socket,
            boost::asio::buffer(_recv_pack_data, CRecvMessageNode::_HEAD_LEN),
            [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t recv_pack_size){
                HandleReadHead(error, recv_pack_size, self_ptr);
            }
        );
    }
    else{
        std::cerr << "Read msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}


// 写操作完成后的回调函数
void CSession::HandleWrite(const boost::system::error_code &error, std::shared_ptr<CSession> self_ptr){
    if(!error){
        std::lock_guard<std::mutex> lock(_msg_queue_mutex);
        _sending_msg_queue.pop();  // 发送完成后将消息从队列中删除

        // 如果队列中还有消息，则继续注册写事件
        if(!_sending_msg_queue.empty()){
            auto &send_msg_node = _sending_msg_queue.front();
            boost::asio::async_write(
                *_socket,
                boost::asio::buffer(send_msg_node->GetData(), send_msg_node->GetDataLen()),
                [this, self_ptr](const boost::system::error_code &error, size_t send_pack_size) {
                    HandleWrite(error, self_ptr);
                }
            );
        }
    }
    else{
        std::cerr << "Send msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}