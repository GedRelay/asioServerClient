#include "../include/CMessageNode.h"

CSendMessageNode::CSendMessageNode(char *msg, uint16_t msg_len){
    _msg_len = msg_len;
    _data = new char[_msg_len];
    memcpy(_data, msg, _msg_len);
}


CSendMessageNode::CSendMessageNode(std::string str){
    _msg_len = str.size();
    _data = new char[_msg_len];
    memcpy(_data, str.c_str(), _msg_len);
}


CRecvMessageNode::CRecvMessageNode(){
    _msg_len = 0;
    _data = new char[_MAX_MSG_LENGTH];
}