同步客户端
在E-client基础上的修改
1. 引入protobuf和jsoncpp库对消息进行序列化和反序列化处理
    - 对于消息id为1001的消息，表示json格式的普通消息，对其做json解析。字段为msg_id, user_name, msg
    - 对于消息id为901的消息，表示protobuf格式的普通消息，对其做protobuf解析。字段为folat msg_id, uint32 machine_id, string msg