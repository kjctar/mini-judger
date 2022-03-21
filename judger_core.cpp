//
// Created by kjctar on 2022/3/21.
//

#include <stdlib.h>

#include <unistd.h>

#include <sys/wait.h>

#include<string>
#include<assert.h>
#include "language.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <sys/resource.h>

class Sandbox{
private:
    int user;

public:
    std::string workdir;
    //初始化沙盒，编译运行环境的构建
    Sandbox(std::string  workdir){

    }
    //进入沙盒
    void into(){

    }
};


class Report{
public:

    uint8_t  status[1];//1 compiling ,2 running 11 compile error  21 run error 22 TL  23 ML  3 ok 31 answer error
    int sock;
    struct sockaddr_in targetAddr;

    Report(std::string SERVER_IP,int SERVER_PORT){
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            //perror("socket");//改成日志最好
           // exit(0);
        }

        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(SERVER_PORT);
        sockaddr_in serverAddr;
        serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
        if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
            //perror("connect");
           // exit(0);
        }
    }


    void upStatus(uint8_t new_status){
        status[0]=new_status;
        std::thread(&Report::feedback,this);
    }
    void feedback(){

        send(sock, status, 1, 0);

    }
    ~Report(){
        close(sock);
    }

};
class tracer{
public:
    Report report;
    language lang;
    const int STD_MB=1024;
    Sandbox box;
    tracer(std::string SERVER_IP,int SERVER_PORT,language lang,std::string workdir):report(SERVER_IP, SERVER_PORT),lang(lang),
                                                                                    box(workdir){

    }
    //编译
    bool compile(){
        int pid = fork();//创建子进程
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

           // execute_cmd("/bin/chown judge %s ", box.workdir.c_str());//将工作目录的所有者设置为judge
          //  execute_cmd("/bin/chmod 750 %s ", box.workdir.c_str());//并赋予相应的权限


            /*
            X_OK 值为1，判断对文件是可执行权限

            W_OK 值为2，判断对文件是否有写权限

            R_OK 值为4，判断对文件是否有读权限

            注：后三种可以使用或“|”的方式，一起使用，如W_OK|R_OK
            */
            if (access("usr", F_OK ) == -1){//这里就是判断usr目录是否存在，失败返回 -1 ，正常情况下，上面已经清理过了工作目录usr肯定不存在
                //准备编译环境

            }
           // if(chroot(work_dir.c_str())) printf("warning chroot fail!\n");

            while (setgid(1536) != 0)
                sleep(1);
            while (setuid(1536) != 0)
                sleep(1);
            while (setresuid(1536, 1536, 1536) != 0)
                sleep(1);

            /*****************************************各个语言的编译环境已经搭建好了，下面是执行编译命令*********************/
//            switch (lang)
//            {
//                case 0:
//                    execvp(CP_C[0], (char *const *)CP_C);
//                    break;
//                case 1:
//                    execvp(CP_X[0], (char *const *)CP_X);
//                    break;
//
//                default:
//                    printf("nothing to do!\n");
//            }
//
//
//            cout<<"编译完成！"<<endl;
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
    //运行
    bool run(){

    }
    //对比输出文件
    bool diff(){

    }
    ~tracer(){

    }

};

/*
 * argc=5
 * ip
 * port
 * tid
 * argv langid
 *
 */
int main(int argc,char *argv[]){

    assert(argc==5);
   // int ip= atoi(argv[1]);
    int port=atoi(argv[2]);
    int tid= atoi(argv[3]);
    int langid=atoi(argv[4]);
    language_set langs("langs.conf");
    if(langs.exist(langid)){
        tracer tc(argv[1],port,langs.get(langid),"workdir");
        if(tc.compile()){
            if(tc.run()){
                tc.diff();
            }
        }
    }

}
