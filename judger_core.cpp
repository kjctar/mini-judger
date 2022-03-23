//
// Created by kjctar on 2022/3/21.
//

#include <cstdlib>

#include <unistd.h>

#include <sys/wait.h>

#include<string>
#include<cassert>
#include "language.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/reg.h>



#include <syslog.h>
#include <cerrno>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <ctime>
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
#include <cstring>
#ifdef OJ_USE_MYSQL
#include <mysql/mysql.h>
#endif
#include <assert.h>
class Sandbox{
private:
    int user;

public:

    std::string workdir;
    //初始化沙盒，编译运行环境的构建
    Sandbox(std::string  workdir){
        this->workdir=workdir;

    }

    void init(){
        std::cout<<workdir<<std::endl;
        system(("rm -rf  "+workdir).c_str());
        system(("mkdir -p "+workdir).c_str());
        system("cp /home/kjctar/Desktop/main.c /home/kjctar/Desktop/rundir/");
        system(("mkdir -p  "+workdir+"/lib/x86_64-linux-gnu").c_str());
        system(("mkdir -p  "+workdir+"/lib64").c_str());
        system(("mkdir -p  "+workdir+"/bin").c_str());
        system(("cp /lib/x86_64-linux-gnu/libtinfo.so.6 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
        system(("cp /lib/x86_64-linux-gnu/libdl.so.2 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
        system(("cp /lib/x86_64-linux-gnu/libc.so.6 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
        system(("cp /lib64/ld-linux-x86-64.so.2 "+workdir+"/lib64").c_str());
        system(("cp /bin/bash "+workdir+"/bin/").c_str());
    }
    //进入沙盒
    int into(int openChroot){
        int err=0;
        err+=chdir(workdir.c_str());
        char dir[40];
        char *p = getcwd(dir , 40);
        std:std::cout<<"当前工作目录："<<dir<<std::endl;
        system(("/bin/touch "+workdir+"/user.out").c_str());
        system(("/bin/touch "+workdir+"/error.out").c_str());
        stdout=freopen("user.out", "w", stdout);
        stderr=freopen("error.out", "a+", stderr);

        if(openChroot){
            std::cout<<"开启 chroot\n";
            err+=chroot(workdir.c_str());
        }



        return err;
    }
};


class Report{
public:

    uint8_t  status[1];//1 compiling ,2 running 11 compile error  21 run error 22 TL  23 ML  3 ok 31 answer error
    int sock;
    struct sockaddr_in targetAddr;

    Report(std::string SERVER_IP,int SERVER_PORT){

        if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("socket");//改成日志最好
           // exit(0);
        }
        memset(&targetAddr, 0, sizeof(targetAddr));
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(SERVER_PORT);
        if( inet_pton(AF_INET, SERVER_IP.c_str(), &targetAddr.sin_addr) <= 0){
            //printf("inet_pton error for %s\n",SERVER_IP.c_str());
            //return 0;
        }

        std::cout<<"connect "<<SERVER_IP<<" "<<SERVER_PORT<<std::endl;
        if( connect(sock, (struct sockaddr*)&targetAddr, sizeof(targetAddr)) < 0){
           // printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
            std::cout<<"链接失败\n";
            return;
        }



    }


    void upStatus(uint8_t new_status){
        status[0]=new_status;
       // std::thread(&Report::feedback,this);
    }
    void feedback(){

        send(sock, status, 1, 0);

    }
    ~Report(){
        close(sock);
    }

};
class Judger{
public:
    Report report;
    language lang;
    const int STD_MB=1048576LL;
    const int BUFFER_SIZE=4096;
    Sandbox box;
    Judger(std::string SERVER_IP,int SERVER_PORT,language lang,std::string workdir):report(SERVER_IP, SERVER_PORT),lang(lang),
                                                                                        box(workdir){
        box.init();
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
            const char *CP_C[] = {"/bin/gcc", "-o", "main", "main.c",  "-lm",  NULL};    // 看起来别扭，但是gcc非要-lm选项在Main.c后面才认
//            const char *CP_X[] = {"g++", "-fno-asm", cc_opt , fmax_errors , cpp_std ,
//                                  "-Wall", "-lm", "--static", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL};

            printf("hhhhhn\n");

           // report.upStatus(1);
            box.into(0);
            execvp(CP_C[0],(char *const *)CP_C);

            //std::cout<<"编译完成！"<<std::endl;

            exit(0);
        }
        else//父进程
        {
            int status = 0;
            std::cout<<"等待编译完成\n";
            waitpid(pid, &status, 0);//阻塞的等待编译命令执行完
            std::cout<<"父进程等待结束\n";
            report.upStatus(11);
            return status;
        }
    }
    //运行
    int get_proc_status(int pid, const char *mark)
    {
        FILE *pf;
        char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
        int ret = 0;
        sprintf(fn, "/proc/%d/status", pid);
        pf = fopen(fn, "re");
        int m = strlen(mark);
        while (pf && fgets(buf, BUFFER_SIZE - 1, pf))//循环查找/proc/%d/status文件里对应参数的值
        {

            buf[strlen(buf) - 1] = 0;
            if (strncmp(buf, mark, m) == 0)//找到对应的参数
            {
                if(1!=sscanf(buf + m + 1, "%d", &ret)) printf("proc read fail\n");//将参数值写入到ret里
            }
        }
        if (pf)
            fclose(pf);
        return ret;
    }
    long get_file_size(const char *filename)
    {
        struct stat f_stat;

        if (stat(filename, &f_stat) == -1)
        {
            return 0;
        }

        return (long)f_stat.st_size;
    }
    void tracer(const int &pid,const int &time_lmt,const int &mem_lmt,int &curmemory,int &usedtime){

        int status, sig, exitcode;
        struct rusage ruse;
        int first = true;
        while (true)
        {
            wait4(pid, &status, __WALL, &ruse);     //等待子进程切换内核态（调用系统API或者运行状态变化） ，其中ruse是为了获取cpu使用时间
            if (first)
            { //
                ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT);
            }
            int temp=get_proc_status(pid, "VmPeak:");
            curmemory = std::max(temp,curmemory);
            //std::cout<<curmemory<<std::endl;
            if ((curmemory<<10) > mem_lmt * STD_MB)
            {
                report.upStatus(23);//内存超出限制
                printf("内存超出限制！\n");
                ptrace(PTRACE_KILL, pid, NULL, NULL);
                break;
            }

            ////1 compiling ,2 running 11 compile error  21 run error 22 TL  23 ML  24 OL 3 ok 31 answer error
            if (WIFEXITED(status)) {
                std::cout<<"正常结束,状态码号："<<WEXITSTATUS(status)<<" "<<strsignal(WEXITSTATUS(status))<<std::endl;
                break;
            }

            if(get_file_size((box.workdir+"/error.out").c_str())){
                printf("运行出错\n");
                break;
            }
            //printf("%d\n",WEXITSTATUS(status));
            if (WIFSIGNALED(status))
            {

                /*  WIFSIGNALED: if the process is terminated by signal
                 *  由外部信号触发的进程终止
                 *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
                 * sig = 5 means Trace/breakpoint trap
                 * sig = 11 means Segmentation fault
                 * sig = 25 means File size limit exceeded
                 */
                sig = WTERMSIG(status);
                printf("有异常信号：%d！\n",sig);
                switch (sig)                  // 根据退出的原因给出判题结果
                {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                        printf("alarm:%d\n", time_lmt);
                    case SIGKILL:
                    case SIGXCPU:
                        report.upStatus(22);//超时
                        //usedtime = time_lmt * 1000;
                        break;
                    case SIGXFSZ:
                        report.upStatus(24);//输出超出限制
                        break;
                    default:
                        report.upStatus(23);//运行出错;
                }
                //print_runtimeerror(infile+strlen(oj_home)+5,strsignal(exitcode));
                std::cout<<strsignal(exitcode)<<std::endl;
                break;
            }


            long  call_id = ptrace(PTRACE_PEEKUSER, pid, ORIG_RAX * 8, NULL);
            //std::cout<<"系统回调id："<<call_id<<std::endl;
            // 白名单机制
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);    // 继续等待下一次的系统调用或其他中断

        }
        usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000) ; // 统计用户态耗时，在更快速的CPU上加以cpu_compensation倍数放大
        usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000) ; // 统计内核态耗时，在更快速的CPU上加以cpu_compensation倍数放大
    }
    void tracee(const int &time_lmt,const int &mem_lmt){
        char * const envp[]={(char * const )"PYTHONIOENCODING=utf-8",
                             (char * const )"LANG=zh_CN.UTF-8",
                             (char * const )"LANGUAGE=zh_CN.UTF-8",
                             (char * const )"LC_ALL=zh_CN.utf-8",NULL};
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        if(nice(19)!=19) printf("......................renice fail... \n");

        // trace me


        // now the user is "judger"
        if(box.into(1)==0){
            std::cout<<"正常进入沙盒\n";
        }


        struct rlimit LIM; // time limit, file limit& memory limit
        LIM.rlim_cur = time_lmt+1;
        LIM.rlim_max = time_lmt+1;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(0);
        alarm(time_lmt);

        // file limit
        LIM.rlim_max = STD_MB<<5 + STD_MB;
        LIM.rlim_cur = STD_MB<<5;
        setrlimit(RLIMIT_FSIZE, &LIM);

        // proc limit
        LIM.rlim_cur = LIM.rlim_max = 1;
        setrlimit(RLIMIT_NPROC, &LIM);

        // set the stack
        LIM.rlim_cur = STD_MB << 8;
        LIM.rlim_max = STD_MB << 8;
        setrlimit(RLIMIT_STACK, &LIM);

        // set the memory
        LIM.rlim_cur = STD_MB * mem_lmt / 2 * 3;
        LIM.rlim_max = STD_MB * mem_lmt * 2;
        setrlimit(RLIMIT_AS, &LIM);//虚拟内存限制


        execle("./main", "./main", (char *)NULL,envp);
    }
    bool run(){

        int mem_lmt=64;
        int  time_lmt=1;
        int usedtime=0;
        int curmemory = 0;
        pid_t pid = fork();
        if (pid == 0){
            tracee(time_lmt,mem_lmt);
        }
        else{
            tracer(pid,time_lmt,mem_lmt,curmemory,usedtime);
            std::cout<<"内存："<<curmemory<<"KB  时间："<<usedtime<<"ms"<<std::endl;
        }
        return true;

    }
    //对比输出文件
    bool diff(){
        return true;
    }
    ~Judger(){

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

    if(argc !=6) return 0;
    //assert(argc==5);
    int ip= atoi(argv[1]);
    int port=atoi(argv[2]);
    int tid= atoi(argv[3]);
    int langid=atoi(argv[4]);
    std::string workdir=argv[5];
    language_set langs("langs.conf");
    if(langs.exist(langid)){
        Judger tc(argv[1],port,langs.get(langid),workdir);
        tc.compile();
        tc.run();
//        if(tc.compile()){
//            if(tc.run()){
//                tc.diff();
//            }
//        }
    }

}
