#pragma once
#include "Singleton.h"
#include <queue>
#include "CMessageNode.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "../proto/Msg901.pb.h"
#include "CSession.h"


class CSession;
using HandlerFunc = std::function<void(std::shared_ptr<CSession> session, const uint16_t &msg_id, char *msg, uint32_t msg_len)>;


// 逻辑结点
class CLogicNode{
public:
    CLogicNode(std::shared_ptr<CSession> session, std::shared_ptr<CRecvMessageNode> recv_msg_node);
    std::shared_ptr<CSession> GetSession();
    std::shared_ptr<CRecvMessageNode> GetCRecvMessageNode();
private:
    std::shared_ptr<CSession> _session;
    std::shared_ptr<CRecvMessageNode> _recv_msg_node;
};


// 逻辑系统
class CLogicSystem: public Singleton<CLogicSystem>{
    friend class Singleton<CLogicSystem>;
public:
    ~CLogicSystem();
    void PostMsgToQue(std::shared_ptr<CSession> session, std::shared_ptr<CRecvMessageNode> recv_msg_node);

private:
    CLogicSystem();
    void DealMsg();  // 处理消息
    void DealOneMsg();  // 处理队列中的一条消息
    void RegisterHandlerFunc();  // 注册消息的处理函数

    std::unordered_map<uint16_t, HandlerFunc> _handler_functions;  // 保存消息处理函数，键为消息类型，值为对应的处理函数
    bool _is_server_shutdown;  // 服务器是否关闭，如果关闭则将逻辑结点队列中所有结点处理完后再析构
    std::queue<std::shared_ptr<CLogicNode>> _logic_queue;  // 逻辑结点队列
    std::mutex _logic_queue_mutex;
    std::condition_variable _consume;
    std::thread _queue_pop_thread;  // 不断弹出队列元素进行处理的线程
};


void MsgHandler_901(std::shared_ptr<CSession> session, const uint16_t &msg_id, char *msg, uint32_t msg_len);
void MsgHandler_1001(std::shared_ptr<CSession> session, const uint16_t &msg_id, char *msg, uint32_t msg_len);