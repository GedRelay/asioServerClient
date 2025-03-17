#include "../include/CLogicSystem.h"


CLogicNode::CLogicNode(std::shared_ptr<CSession> session, std::shared_ptr<CRecvMessageNode> recv_msg_node):
    _session(session),
    _recv_msg_node(recv_msg_node){

}


std::shared_ptr<CSession> CLogicNode::GetSession(){
    return _session;
}


std::shared_ptr<CRecvMessageNode> CLogicNode::GetCRecvMessageNode(){
    return _recv_msg_node;
}


// 线程开始不断处理数据
CLogicSystem::CLogicSystem():
    _is_server_shutdown(false){
    RegisterHandlerFunc();
    _queue_pop_thread = std::thread(&CLogicSystem::DealMsg, this);
}


// 服务器关闭则将逻辑结点队列中所有结点处理完后再析构
CLogicSystem::~CLogicSystem(){
    _is_server_shutdown = true;
    _consume.notify_one();
    _queue_pop_thread.join();
}


// 注册消息处理函数
void CLogicSystem::RegisterHandlerFunc(){
    _handler_functions[1001] = MsgHandler_1001;
    _handler_functions[901] = MsgHandler_901;
}


// 处理消息
void CLogicSystem::DealMsg(){
    while(true){
        std::unique_lock<std::mutex> lock(_logic_queue_mutex);
        // 判断队列为空则用条件变量阻塞等待，并释放锁
        while(_logic_queue.empty() && !_is_server_shutdown){
            _consume.wait(lock);
        }
        // 如果服务器关闭，将队列里的消息处理完后break出循环
        if(_is_server_shutdown){
            while(!_logic_queue.empty()){
                DealOneMsg();
            }
            break;
        }
        // 服务器没有关闭，且队列中有数据
        DealOneMsg();
    }
}


// 处理队列中的一条消息（处理失败返回false）
void CLogicSystem::DealOneMsg(){
    std::shared_ptr<CLogicNode> logic_node = _logic_queue.front();
    // 如果处理函数中没有与msg_id对应的处理函数，直接弹出队列不进行处理
    uint16_t msg_id = logic_node->GetCRecvMessageNode()->GetMsgId();
    if(!_handler_functions.count(msg_id)){
        _logic_queue.pop();
        return;
    }
    // 调用处理函数
    _handler_functions[msg_id](
        logic_node->GetSession(),
        msg_id,
        logic_node->GetCRecvMessageNode()->GetMsgData(),
        logic_node->GetCRecvMessageNode()->GetMsgLen()
    );
    // 调用完后将逻辑结点弹出
    _logic_queue.pop();
}


// 将新消息投递进逻辑队列
void CLogicSystem::PostMsgToQue(std::shared_ptr<CSession> session, std::shared_ptr<CRecvMessageNode> recv_msg_node){
    std::unique_lock<std::mutex> lock(_logic_queue_mutex);
    // 将消息结点构造为逻辑结点后加入逻辑队列（进行深拷贝）
    std::shared_ptr<CRecvMessageNode> recv_msg_node_copy = std::make_shared<CRecvMessageNode>(*recv_msg_node);
    auto logic_node = std::make_shared<CLogicNode>(session, recv_msg_node_copy);
    _logic_queue.push(logic_node);
    if(_logic_queue.size() == 1){
        lock.unlock();
        _consume.notify_one();
    }
}


// 处理1001消息
void MsgHandler_1001(std::shared_ptr<CSession> session, const uint16_t &msg_id, char *msg, uint32_t msg_len){
    // 1. 打印数据
    Json::Value root;
    Json::Reader reader;
    reader.parse(msg, msg + msg_len, root);
    std::cout << session->GetSessionId() << " (msg_id=";
    std::cout << root["msg_id"];
    std::cout << ",msg_len=";
    std::cout << msg_len;
    std::cout << ",user_name=";
    std::cout << root["user_name"];
    std::cout << "): ";
    std::cout << root["msg"];
    std::cout << std::endl;

    // 2.构造新消息进行发送
    root["user_name"] = "server";
    root["msg"] = "ok! " + root["msg"].asString() + "!";
    std::string msg_str = root.toStyledString();  // 序列化
    session->SendMsg(1001, msg_str);
}


// 处理901消息
void MsgHandler_901(std::shared_ptr<CSession> session, const uint16_t &msg_id, char *msg, uint32_t msg_len){
    // 1. 打印消息
    Msg901 msg901;
    msg901.ParseFromArray(msg, msg_len);
    std::cout << session->GetSessionId() << " (msg_id=";
    std::cout << msg901.msg_id();
    std::cout << ",msg_len=";
    std::cout << msg_len;
    std::cout << ",machine_id=";
    std::cout << msg901.machine_id();
    std::cout << "): ";
    std::cout << msg901.msg();
    std::cout << std::endl;

    // 2.构造新消息进行发送
    msg901.set_machine_id(1);
    msg901.set_msg("ok! " + msg901.msg() + "!");
    std::string msg_str;
    msg901.SerializeToString(&msg_str);  // 序列化
    session->SendMsg(901, msg_str);
}