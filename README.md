# dls
分布式锁服务器(Distributed Lock Server)
  * 需要libevent2.0库，并安装到/usr/local
  * 需要glib>=2.40库，并安装到/usr/local
  * 编译与使用命令
    * make clean all
    * nohup dls2 >/dev/null 2>&1 &
    * netstat -ntpl | grep 1112
