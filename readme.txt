
# RESP-SERVER
---------------------
一、功能
 基于C++网络编程 reactor框架、leveldb和RESP协议实现的简单redis-server:

二、依赖
 编译时依赖：boost1.41+
 yum install boost

三、编译
 make clean
 make all

四、启动&停止
启动： bin/resp_server -d etc/config.ini 
停止： bin/resp_server -s stop etc/config.ini

五、测试
(1)执行bin/resp_client

(2)使用redis-cli连接，并发送set/get等命令
