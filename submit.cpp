//
// Created by kjctar on 2022/3/21.
//

#include "msg.h"
/*
 * 解析代码提交内容，包含提交tid 占4字节 ，输入长度(4字节) ，输入，输出长度(4字节)，输出，代码语言(1字节)，代码长度(4字节)，代码
 */
class Submit : public Msg{
public:
    std::string tid;
    int language_id;
    std::string input;
    std::string output;
    std::string code;
    int step;
    int in_len;
    int code_len;
    bool decodec(const std::string &data,int &index){
        while(step<5){
            switch(step){
                case 0:{//读取tid
                    if(index+4<=data.size()){//读取tid
                        tid=atoi(data.substr(index,4).c_str());
                         index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 1:{//读取输入长度
                    if( index+4<= data.size()){
                        in_len=atoi( data.substr( index,4).c_str());
                         index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 2:{//读取输入
                    if( index+in_len<= data.size()){//读取用户输入
                        //in_len=atoi( substr(0,4).c_str());
                        output= data.substr( index,in_len);
                         index+=in_len;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 3:{//读取输出长度
                    if( index+4<= data.size()){
                        in_len=atoi( data.substr( index,4).c_str());
                        index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 4:{//读取输出
                    if( index+in_len<= data.size()){//读取用户输入
                        //in_len=atoi( substr(0,4).c_str());
                        output= data.substr( index,in_len);
                        index+=in_len;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 5:{//读取代码语言
                    if( index+4<= data.size()){
                        language_id=atoi( data.substr( index,4).c_str());
                        index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 6:{//读取代码长度
                    if( index+4<= data.size()){
                        code_len=atoi( data.substr( index,4).c_str());
                         index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 7:{//读取代码
                    if( index+code_len<= data.size()){//读取用户输入
                        code= data.substr( index,code_len);
                        index+=in_len;
                        step++;

                    }else{
                        return false;
                    }
                    break;
                }
            }

        }

        return true;
    }
    ~Submit(){

    }
};
