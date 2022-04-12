//
// Created by kjctar on 2022/3/21.
//

#ifndef UNTITLED_LANGUAGE_H
#define UNTITLED_LANGUAGE_H

#endif //UNTITLED_LANGUAGE_H
#include <string>
#include <unordered_map>
#include <assert.h>
#include <iostream>
class language{
    public:
        int tid;
        std::string name;
        std::string complie_cmd;
        std::string run_cmd;
        std::string to_str(){
            return "tid:"+std::to_string(tid)+" name:"+name+" complie_cmd:"+complie_cmd+" run_cmd:"+run_cmd;
        }
};
class language_set{
private:
    std::unordered_map<int,language> langs;
    int sum;
public:
    language_set()=delete;
    language_set(std::string config){
        //读取语言配置文件
        langs[1]=language();

    }
    bool exist(const int &tid){
        if(langs.count(tid)){

            return true;
        }else{
            return false;
        }
    }
    language get(const int &tid) {
        assert(langs.count(tid)==1);
        return langs[tid];
    }
    int size() const{
        return sum;
    }
    void display(){
        for(auto &&it:langs){
            std::cout<<"[key]="<<it.first<<std::endl;
            std::cout<<"[value]="<<it.second.to_str()<<std::endl;
        }
    }

};

