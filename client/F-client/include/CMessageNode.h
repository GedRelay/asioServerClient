#pragma once
#include <iostream>
#include <cstring>

class CMessageNode {
public:
    virtual ~CMessageNode() { delete[] _data; }

    static const uint16_t _HEAD_ID_LEN = 2;  // 消息头中消息id的长度（字节）
    static const uint16_t _HEAD_LEN_LEN = 2;  // 消息头中消息长度的长度（字节）
    static const uint16_t _HEAD_LEN = _HEAD_ID_LEN + _HEAD_LEN_LEN;  // 消息头部总长度（字节）

    char *GetData() { return _data; }
    char *GetIdData() { return _data; }
    char *GetLenData() { return _data + _HEAD_ID_LEN; }
    char *GetMsgData() { return _data + _HEAD_LEN; }

    uint32_t GetDataLen() { return _data_len; }
    uint16_t GetMsgId() { return _msg_id; }
    uint16_t GetMsgLen() { return _msg_len; }
protected:
    CMessageNode() = default;  // 阻止基类被实例化
    char *_data;
    uint16_t _msg_id;  // 消息类型
    uint16_t _msg_len;  // 消息长度(字节)
    uint32_t _data_len;  // 数据总长度(字节)
};


// 用于存放发送的消息的节点
class CSendMessageNode : public CMessageNode {
public:
    CSendMessageNode() = delete;
    CSendMessageNode(uint16_t msg_id, char *msg, uint16_t msg_len);
    CSendMessageNode(uint16_t msg_id, std::string str);
};


// 用于存放接收的消息的节点
class CRecvMessageNode : public CMessageNode {
public:
    CRecvMessageNode();
    enum { _MAX_MSG_LENGTH = 2048 };  // 最大消息长度，消息超过该长度则断开连接
    void Reset();  // 逻辑复位
    void AppendData(char *data, uint32_t data_len);  // 追加数据
private:
    bool _msg_id_has_been_parsed;
    bool _msg_len_has_been_parsed;
};