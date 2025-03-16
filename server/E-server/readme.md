异步服务器
在D-server基础上的修改
1. 将消息改为TLV结构（类型msg_id，长度msg_len，内容msg）
    - 通过标明消息的类型，后期可以根据不同的消息id对消息进行不同处理
    - 通过在消息中指定消息长度，可以解决粘包问题
    - 为了保证在不同机器中数据解析正确，在一条消息中，类型msg_id和长度msg_len在发送时需要将其转换为网络字节序，在接收时需要转换为本地字节序
2. 将读取消息分成了两个回调函数：
    - session首先注册读取事件，读取消息头长度的消息，读取到消息头后调用HandleReadHead()回调函数
    - 在HandleReadHead()函数中解析消息头中的msg_id和msg_len，之后根据msg_len注册读事件，读取消息体长度的消息，读取到消息体后调用HandleReadMsg()回调函数
    - 在HandleReadMsg()函数中处理消息体（打印并将消息后面加一个~号后发回），处理完后，注册读取事件，读取消息头长度的消息，读取到消息头后调用HandleReadHead()回调函数
    - 由于知道需要读取的长度，读取由_socket->async_read_some()改为boost::asio::async_read()，可以更加方便地处理粘包，不需要自己写切包的逻辑
3. 在main.cpp中，使用信号监听对象监听SIGINT（ctrl+c）和SIGTERM（kill）信号，当收到信号时，调用io_context.stop()，所有未完成的异步操作会被取消，资源会被正确释放。实现程序优雅退出