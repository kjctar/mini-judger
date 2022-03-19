#include <iostream>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>
//#include <sys/types.h>
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
using namespace std;
class language{
    private:
        int type;
        string name;
        string complie_cmd;
        string run_cmd;
    public:
};
class request{
    public:
        string tid;
        string user_input;
        int language_id;
        string user_code;

        request()=default;



};
class response{
    public:
        int status;
        string msg;
        response(int a,string b):status(a),msg(b){}
        string to_stream(){
            char *head=(char*)status;
            int len=msg.size();
            return string((char*)status)+string((char*)len)+msg;
        }
};
template<typename T>
class tsqueue{
    public:
    tsqueue()=default;
    tsqueue(tsqueue const& other){
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value){
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    T pop(){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty(); });
        T value = data_queue.front();
        data_queue.pop();
        return value;
    }
    bool empty() const{
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
    int size() const{
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
    private:
        mutable std::mutex mut;
        std::queue<T> data_queue;
        std::condition_variable data_cond;
};
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
class  decoder{
public:
    decoder(){
        step=0;
    }
    request res(){
        return decode_result;
    }
    bool decode(buffer &buf){
        while(step<5){
            switch(step){
                case 0:{//读取tid
                    if(buf.index+4<=buf.data.size()){//读取tid
                        decode_result.tid=atoi(buf.data.substr(buf.index,4).c_str());
                        buf.index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 1:{//读取用户输入长度
                    if(buf.index+4<=buf.data.size()){
                        in_len=atoi(buf.data.substr(buf.index,4).c_str());
                        buf.index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 2:{
                    if(buf.index+in_len<=buf.data.size()){//读取用户输入
                        //in_len=atoi(buf.substr(0,4).c_str());
                        decode_result.user_input=buf.data.substr(buf.index,in_len);
                        buf.index+=in_len;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 3:{//读取代码长度
                    if(buf.index+4<=buf.data.size()){
                        code_len=atoi(buf.data.substr(buf.index,4).c_str());
                        buf.index+=4;
                        step++;
                    }else{
                        return false;
                    }
                    break;
                }
                case 4:{//读取代码长度
                    if(buf.index+code_len<=buf.data.size()){//读取用户输入
                        decode_result.user_code=buf.data.substr(buf.index,code_len);
                        buf.index+=in_len;
                        step++;

                    }else{
                        return false;
                    }
                    break;
                }


            }

        }
        step=0;
        return true;
    }

    int step=0;
    int in_len;
    int code_len;
    request decode_result;
};

class Client{
    private:

        int skt;
        struct sockaddr_in targetAddr;
        char cache[65535];
        buffer buf;

    public:
        Client()=default;
        Client(string SERVER_IP,int SERVER_PORT){
            if((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                perror("socket");
                exit(0);
            }

            targetAddr.sin_family = AF_INET;
            targetAddr.sin_port = htons(SERVER_PORT);
            sockaddr_in serverAddr;
            serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
            if(connect(skt, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
                perror("connect");
                exit(0);
            }
        }


        void up(string msg){
            send(skt, msg.c_str(), strlen(msg.c_str()), 0);
        }
        string down(){

            int index = recv(skt, cache, strlen(cache), 0);
            cache[index] = '\0';
            return  cache;
        }
};

class MQ{
    private:

        decoder decer;
        Client client;
        bool stop;

        //向服务器建立链接
    public:
        buffer buf;
        tsqueue<request> que;
        tsqueue<response> res;
        MQ()=default;
        MQ(string SERVER_IP,int SERVER_PORT):client(SERVER_IP,SERVER_PORT){


            cout<<"链接建立成功！"<<endl;
        }
        void start(){
            stop= false;
            thread(&MQ::rec_msg,*this);
            thread(&MQ::send_msg,*this);
        }
        //接受消息
        //用县城去调用
        void rec_msg(){//接受服务器的消息

            while(!stop){
                buf.add(client.down());
                while(decer.decode(buf)){

                    que.push(decer.res());
                }
            }
        }
        void  close(){

        }

        void send_msg(){//向服务器发送消息


            while(!stop){

                client.up(res.pop().to_stream());

            }
        }
};
//串行化处理get到的任务，运行过程异步处理put结果，响应父进测试状态，
class judger{
    private:
        int limit;
        request cur;
        bool stop;
        string work_dir;
        void tracer(){
            //mq.res.push(response(0,"hhhh"));
            complier();
            run();
        }
    public:
        MQ mq;
        judger(string SERVER_IP,int SERVER_PORT):mq(SERVER_IP,SERVER_PORT){

        }
        void init(){

        }
        void work(){
            stop= false;
            mq.start();
            while(!stop){
                request tk=move(mq.que.pop());
                tracer();
            }
        }

        int execute_cmd(const char *fmt, ...)   //执行命令获得返回值
        {
            char cmd[1024];

            int ret = 0;
            va_list ap;

            va_start(ap, fmt);
            vsprintf(cmd, fmt, ap);

            ret = system(cmd);
            va_end(ap);
            return ret;
        }
        bool complier(){

            int pid;
            char fmax_errors[1024];
            int lang;

            const char *CP_C[] = {"gcc", "-fno-asm", cc_opt , fmax_errors , cc_std  ,
                                  "-Wall", "--static", "-DONLINE_JUDGE", "-o", "Main", "Main.c",  "-lm",  NULL};    // 看起来别扭，但是gcc非要-lm选项在Main.c后面才认
            const char *CP_X[] = {"g++", "-fno-asm", cc_opt , fmax_errors , cpp_std ,
                                  "-Wall", "-lm", "--static", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL};

            const int STD_MB=1024;
            /***************************************到这里编译命令已经初始化完成，下面创建子进程去执行命令*************/
            pid = fork();//创建子进程
            if (pid == 0)
            {
                struct rlimit LIM;
                int cpu = 50;

                LIM.rlim_max = cpu;//s
                LIM.rlim_cur = cpu;
                setrlimit(RLIMIT_CPU, &LIM);
                alarm(0);//alarm(0)的作用,用于清除闹钟。一个进程只可以有一个闹钟
                alarm(cpu);//设置子进程的闹钟
                LIM.rlim_max = 500 * STD_MB;
                LIM.rlim_cur = 500 * STD_MB;
                setrlimit(RLIMIT_FSIZE, &LIM);


                LIM.rlim_max = STD_MB << 11 ;
                LIM.rlim_cur = STD_MB << 11;


                setrlimit(RLIMIT_AS, &LIM);//设置进程的虚拟地址空间大小，这里是 500mb

                stderr=freopen("ce.txt", "w", stderr);
                //freopen("/dev/null", "w", stdout);

                execute_cmd("/bin/chown judge %s ", work_dir.c_str());//将工作目录的所有者设置为judge
                execute_cmd("/bin/chmod 750 %s ", work_dir.c_str());//并赋予相应的权限


                    /*
                    X_OK 值为1，判断对文件是可执行权限

                    W_OK 值为2，判断对文件是否有写权限

                    R_OK 值为4，判断对文件是否有读权限

                    注：后三种可以使用或“|”的方式，一起使用，如W_OK|R_OK
                    */
                    if (access("usr", F_OK ) == -1){//这里就是判断usr目录是否存在，失败返回 -1 ，正常情况下，上面已经清理过了工作目录usr肯定不存在
                        //准备编译环境

                    }
                    if(chroot(work_dir.c_str())) printf("warning chroot fail!\n");

                while (setgid(1536) != 0)
                    sleep(1);
                while (setuid(1536) != 0)
                    sleep(1);
                while (setresuid(1536, 1536, 1536) != 0)
                    sleep(1);

                /*****************************************各个语言的编译环境已经搭建好了，下面是执行编译命令*********************/
                switch (lang)
                {
                    case 0:
                        execvp(CP_C[0], (char *const *)CP_C);
                        break;
                    case 1:
                        execvp(CP_X[0], (char *const *)CP_X);
                        break;

                    default:
                        printf("nothing to do!\n");
                }


                cout<<"编译完成！"<<endl;
                //exit(!system("cat ce.txt"));
                exit(0);
            }
            else//父进程
            {
                int status = 0;

                waitpid(pid, &status, 0);//阻塞的等待编译命令执行完

                //execute_cmd("/bin/umount -l bin usr lib lib64 etc/alternatives dev 2>/dev/null");//卸载目录
                //execute_cmd("/bin/umount -r %s/* 2>/dev/null", work_dir);
                //umount(work_dir);//清理一遍

                return status;
            }
        }
        int run(){

        }

};

int main() {

    MQ mq;
    std::thread tt(&MQ::rec_msg,mq);
    string ip;
    int port;
    cin>>ip>>port;
    std::cout << "Hello, World!" << std::endl;
//    if(grc<3){
//        cout<<"lack arg server_ip server_port"<<endl;
//    }
//    judger jer(grcv[1],atoi(grcv[2]));
    judger jer(ip,port);
    jer.init();//初始化配置文件，比如要监听的服务器地址，各项语言编译运行环境和参数
    jer.work();


    return 0;
}
