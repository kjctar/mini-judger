//
// Created by kjctar on 2022/3/21.
//
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include<queue>#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <mutex>
#include <stack>
#include <string>
#include <exception>
#include <memory> // For std::shared_ptr<>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class buffer{
public:
    string data;

    int index;
    void add( string temp){
        data+=temp;

    }
    void update(){
        data.erase(0,index);
        index=0;
    }

};
class JudgerServer{

};

