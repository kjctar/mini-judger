# mini-judger
online judger系统
- 几年前拜读过hustoj的源码，并做了二次开发，目前已不再更新维护；
- 二开做的最多的是web上的改进，judger仅使用链表实现多点测试结果信息提取,增加了一个数据表
- 之后hustoj也更新上了这个功能，不过张老师并没有添加一个数据表，而只是加了一个字段用分隔符分割各个测试结果，从空间利用率和sql调用次数上来看张老师的解决方案更好

- 传统的judger服务器结构，是一个守护进程+N个判题进程，守护进程主要负责管理judger和get提交的代码，hustoj的实现就是这个结构；
- 不过hustoj通过不断轮训数据库的方式获取用户的提交，在没用用户提交的情况下会浪费cpu资源，hustoj作者就这个问题并做了更新，采用在web端向守护进程发送提交信号的方式唤醒守护进程。

为了探索更好的解决方案计划用空余时间逐步完成这个mini-Judger
- [x] c++实现judger
- [ ] golang Web端
- [ ] 引入消息队列
- [ ] 引入分布式文件系统做测试数据的同步
