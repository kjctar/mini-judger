# mini-judger
online judger系统
大部分学校使用过hustoj系统，因为该项目开源免费，我几年前也拜读过hustoj的源码，并做了二次开发；
不过二开做的最多的是web上的改进，至于judger，由于是纯c语言实现，改动略有困难，只做了多点测试结果相关信息提取,增加了一个数据表
之后hustoj也更新上了这个功能，不过张老师并没有添加一个数据表，而只是加了一个字段用分隔符分割各个测试结果，从空间利用率已经sql调用次数上来看张老师的解决方案更好
传统的judger服务器结构，是一个守护进程+N个判题进程，守护进程主要负责管理judger和get提交的代码，当初hustoj的实现就是这个结构；
不过hustoj通过不断轮训数据库的方式获取用户的提交，在没用用户提交的情况下不断轮训会浪费cpu资源，张老师也知道这个问题并做了更新，采用在web端向守护进程发送提交信号的方式唤醒守护进程。

该项目拟使用c++实现judger,golang实现web端,通过消息队列的方式传递用户代码
- [x] c++实现judger
- [ ] golang Web端
- [ ] 引入消息队列
- [ ] 引入分布式文件系统做测试数据的同步