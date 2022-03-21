//
// Created by kjctar on 2022/3/21.
//

#ifndef UNTITLED_MSG_CACHE_H
#define UNTITLED_MSG_CACHE_H

#endif //UNTITLED_MSG_CACHE_H
#include <string>
#include <iostream>
#include <functional>
#include <queue>
#include <memory>
#include <iostream>
//#include <utility>
#include "msg.h"
template<typename M>
class Cache{
public:
    std::string buf;
    std::queue<std::unique_ptr<M>>  msgs;

    int index;
    Cache(){
        index=0;
    }
    void add(const std::string &temp){
        buf+=temp;
    }
    void add(const M msg){
        buf+=msg.decode();
    }
    void work(){
        if(index>0){
            buf.erase(0,index);
            index=0;
        }

    }
    void   decodec(){
        temp->
    };
    void   encodec(){

    };

};