#pragma once
#include <iostream>
#include <cstring>

class CMessageNode {
public:
    CMessageNode() = default;
    virtual ~CMessageNode() { delete[] _data; }
    char* GetData() { return _data; }
    uint32_t GetMsgLen() { return _msg_len; }
protected:
    char *_data;
    uint16_t _msg_len;  // 消息长度（字节）
};


// 用于存放发送的消息的节点
class CSendMessageNode : public CMessageNode {
public:
    CSendMessageNode() = delete;
    CSendMessageNode(char *msg, uint16_t msg_len);
    CSendMessageNode(std::string str);
};


// 用于存放接收的消息的节点
class CRecvMessageNode : public CMessageNode {
public:
    CRecvMessageNode();
    enum { _MAX_MSG_LENGTH = 2048 };  // 最大消息长度，消息超过该长度则断开连接
};