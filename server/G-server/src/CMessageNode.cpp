#include "../include/CMessageNode.h"
#include <boost/asio.hpp>

CSendMessageNode::CSendMessageNode(uint16_t msg_id, char *msg, uint16_t msg_len){
    _msg_id = msg_id;
    _msg_len = msg_len;
    _data_len = _HEAD_LEN + _msg_len;
    _data = new char[_HEAD_LEN + _msg_len];
    // 将msg_id和msg_len转换为网络字节序后拷贝到_data中
    uint16_t network_msg_id = boost::asio::detail::socket_ops::host_to_network_short(_msg_id);
    uint16_t network_msg_len = boost::asio::detail::socket_ops::host_to_network_short(_msg_len);
    memcpy(_data, &network_msg_id, _HEAD_ID_LEN);
    memcpy(_data + _HEAD_ID_LEN, &network_msg_len, _HEAD_LEN_LEN);
    memcpy(_data + _HEAD_LEN, msg, _msg_len);
}


CSendMessageNode::CSendMessageNode(uint16_t msg_id, std::string str){
    _msg_id = msg_id;
    _msg_len = str.size();
    _data_len = _HEAD_LEN + _msg_len;
    _data = new char[_HEAD_LEN + _msg_len];
    // 将msg_id和msg_len转换为网络字节序后拷贝到_data中
    uint16_t network_msg_id = boost::asio::detail::socket_ops::host_to_network_short(_msg_id);
    uint16_t network_msg_len = boost::asio::detail::socket_ops::host_to_network_short(_msg_len);
    memcpy(_data, &network_msg_id, _HEAD_ID_LEN);
    memcpy(_data + _HEAD_ID_LEN, &network_msg_len, _HEAD_LEN_LEN);
    memcpy(_data + _HEAD_LEN, str.c_str(), _msg_len);
}


CRecvMessageNode::CRecvMessageNode(){
    _msg_id = 0;
    _msg_len = 0;
    _data_len = 0;
    _msg_id_has_been_parsed = false;
    _msg_len_has_been_parsed = false;
    _data = new char[_HEAD_LEN + _MAX_MSG_LENGTH];
}


CRecvMessageNode::CRecvMessageNode(const CRecvMessageNode& recv_msg_node){
    _msg_id = recv_msg_node._msg_id;
    _msg_len = recv_msg_node._msg_len;
    _data_len = recv_msg_node._data_len;
    _msg_id_has_been_parsed = recv_msg_node._msg_id_has_been_parsed;
    _msg_len_has_been_parsed = recv_msg_node._msg_len_has_been_parsed;
    _data = new char[_data_len];
    memcpy(_data, recv_msg_node._data, _data_len);
}

// 逻辑复位
void CRecvMessageNode::Reset() {
    _msg_id = 0;
    _msg_len = 0;
    _data_len = 0;
    _msg_id_has_been_parsed = false;
    _msg_len_has_been_parsed = false;
}


// 追加数据
void CRecvMessageNode::AppendData(char *data, uint32_t data_len) {
    memcpy(_data + _data_len, data, data_len);
    _data_len += data_len;
    // 已得到_msg_id的数据，_msg_id还没被解析，则进行解析
    if(_msg_id_has_been_parsed == false && _data_len >= _HEAD_ID_LEN){
        uint16_t msg_id = 0;
        memcpy(&msg_id, _data, _HEAD_ID_LEN);
        _msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
        _msg_id_has_been_parsed = true;
    }
    // 已得到_msg_len的数据，_msg_len还没被解析，则进行解析
    if(_msg_len_has_been_parsed == false && _data_len >= _HEAD_LEN){
        uint16_t msg_len = 0;
        memcpy(&msg_len, _data + _HEAD_ID_LEN, _HEAD_LEN_LEN);
        _msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
        _msg_len_has_been_parsed = true;
    }
}