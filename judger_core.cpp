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

#include <assert.h>


#define OJ_WT0 0     //提交排队
#define OJ_WT1 1     //重判排队
#define OJ_CI 2      //编译中（任务已派发）
#define OJ_RI 3      //运行中
#define OJ_AC 4      //答案正确
#define OJ_PE 5      //格式错误
#define OJ_WA 6      //答案错误
#define OJ_TL 7      //时间超限
#define OJ_ML 8      //内存超限
#define OJ_OL 9      //输出超限
#define OJ_RE 10     //运行错误
#define OJ_CE 11     //编译错误
#define OJ_CO 12     //编译完成
#define OJ_TR 13     //测试运行结束
#define OJ_MC 14     // 等待裁判手工确认
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
       // std::cout<<workdir<<std::endl;
        system(("rm -rf  "+workdir).c_str());
        system(("mkdir -p "+workdir).c_str());
        system("cp /home/kjctar/Desktop/Main.c /home/kjctar/Desktop/rundir/");
//        system(("mkdir -p  "+workdir+"/lib/x86_64-linux-gnu").c_str());
//        system(("mkdir -p  "+workdir+"/lib64").c_str());
//        system(("mkdir -p  "+workdir+"/bin").c_str());
//        system(("mkdir -p  "+workdir+"/proc").c_str());
//        system(("cp /lib/x86_64-linux-gnu/libtinfo.so.6 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
//        system(("cp /lib/x86_64-linux-gnu/libdl.so.2 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
//        system(("cp /lib/x86_64-linux-gnu/libc.so.6 "+workdir+"/lib/x86_64-linux-gnu/").c_str());
//        system(("cp /lib64/ld-linux-x86-64.so.2 "+workdir+"/lib64").c_str());
//        system(("cp /bin/bash "+workdir+"/bin/").c_str());
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
        if(chdir(box.workdir.c_str())) exit(-3);
        int pid;
        char fmax_errors[BUFFER_SIZE];
        if(__GNUC__ > 4 || (  __GNUC__ == 4 &&  __GNUC_MINOR__ >= 8 ) ){
            sprintf(fmax_errors,"-fmax-errors=10");
        }else{
            sprintf(fmax_errors,"-Wformat");
        }
        const char *CP_C[] = {"gcc", "-fno-asm", "-O2" , fmax_errors , "-std=c99"  ,
                              "-Wall", "--static", "-DONLINE_JUDGE", "-o", "Main", "Main.c",  "-lm",  NULL};    // 看起来别扭，但是gcc非要-lm选项在Main.c后面才认

        pid = fork();
        if (pid == 0)
        {
            struct rlimit LIM;
            int cpu = 50;
            LIM.rlim_max = cpu;
            LIM.rlim_cur = cpu;
            setrlimit(RLIMIT_CPU, &LIM);
            LIM.rlim_max = 500 * STD_MB;
            LIM.rlim_cur = 500 * STD_MB;
            setrlimit(RLIMIT_FSIZE, &LIM);

            LIM.rlim_max = STD_MB << 11 ;
            LIM.rlim_cur = STD_MB << 11;
            setrlimit(RLIMIT_AS, &LIM);

           // stderr=freopen("ce.txt", "w", stderr);
            execvp(CP_C[0], (char *const *)CP_C);
            printf("compile end!\n");
            exit(0);
        }
        else
        {
            int status = 0;
            waitpid(pid, &status, 0);
            printf("status=%d\n", status);
            return status;
        }
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

    int get_proc_status(int pid, const char *mark)
    {
        FILE *pf;
        char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
        int ret = 0;
        sprintf(fn, "/proc/%d/status", pid);
        pf = fopen(fn, "re");
        int m = strlen(mark);
        while (pf && fgets(buf, BUFFER_SIZE - 1, pf))
        {

            buf[strlen(buf) - 1] = 0;
            if (strncmp(buf, mark, m) == 0)
            {
                if(1!=sscanf(buf + m + 1, "%d", &ret)) printf("proc read fail\n");
            }
        }
        if (pf)
            fclose(pf);
        return ret;
    }


    int get_page_fault_mem(struct rusage &ruse, pid_t &pidApp)
    {
        //java use pagefault
        int m_vmpeak, m_vmdata, m_minflt;
        m_minflt = ruse.ru_minflt * getpagesize();

        m_vmpeak = get_proc_status(pidApp, "VmPeak:");
        m_vmdata = get_proc_status(pidApp, "VmData:");
       // printf("VmPeak:%d KB VmData:%d KB minflt:%d KB\n", m_vmpeak, m_vmdata,  m_minflt >> 10);

        return m_minflt;
    }
    void tracer( int pidApp,const int &time_lmt,const int &mem_lmt,int &curmemory,int &usedtime){

        // 父进程中的保姆程序
        int tempmemory = 0;
        int topmemory=0;
        int ACflg=OJ_AC;
        int status, sig, exitcode;
        struct user_regs_struct reg;
        struct rusage ruse;
        int first = true;
        while (1){
            wait4(pidApp, &status, __WALL, &ruse);     //等待子进程切换内核态（调用系统API或者运行状态变化）
            if (first){
                ptrace(PTRACE_SETOPTIONS, pidApp, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT);
            }

            tempmemory = get_page_fault_mem(ruse, pidApp);

            if (tempmemory > topmemory)
                topmemory = tempmemory;
            if (topmemory > mem_lmt * STD_MB){

                //printf("out of memory %d\n", topmemory);
                if (ACflg == OJ_AC)
                    ACflg = OJ_ML;
                ptrace(PTRACE_KILL, pidApp, NULL, NULL);
                break;
            }
            //sig = status >> 8;/*status >> 8 EXITCODE*/

            if (WIFEXITED(status))  // 子进程已经退出
                break;
            if (get_file_size("error.out") )
            {
                ACflg = OJ_RE;
                //addreinfo(solution_id);
                ptrace(PTRACE_KILL, pidApp, NULL, NULL);
                break;
            }



            exitcode = WEXITSTATUS(status);
            /*exitcode == 5 waiting for next CPU allocation          * ruby using system to run,exit 17 ok
             *  Runtime Error:Unknown signal xxx need be added here
                     */
           // std::cout<<exitcode<<std::endl;
            if (exitcode == 0x05 || exitcode == 0 || exitcode == 133){

            }else{

              //  printf("异常退出信号：%d",exitcode);
                if (ACflg == OJ_AC){
                    switch (exitcode)                  // 根据退出的原因给出判题结果
                    {
                        case SIGCHLD:
                        case SIGALRM:
                            alarm(0);


                        case SIGKILL:
                        case SIGXCPU:
                            ACflg = OJ_TL;
                            usedtime = time_lmt * 1000;

                            printf("TLE:%d\n", usedtime);
                            break;
                        case SIGXFSZ:
                            ACflg = OJ_OL;
                            break;
                        default:
                            ACflg = OJ_RE;
                    }

                }
                ptrace(PTRACE_KILL, pidApp, NULL, NULL);    // 杀死出问题的进程
                break;
            }
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


             //   printf("异常外部信号：%d",sig);

                if (ACflg == OJ_AC)
                {
                    switch (sig)      //根据原因给出结论
                    {
                        case SIGCHLD:
                        case SIGALRM:
                            alarm(0);
                        case SIGKILL:
                        case SIGXCPU:
                            ACflg = OJ_TL;
                            break;
                        case SIGXFSZ:
                            ACflg = OJ_OL;
                            break;

                        default:
                            ACflg = OJ_RE;
                    }

                }
                break;
            }
            ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);    // 继续等待下一次的系统调用或其他中断
            first = false;
            //usleep(1);
        }
        usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000) ; // 统计用户态耗时，在更快速的CPU上加以cpu_compensation倍数放大
        usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000) ; // 统计内核态耗时，在更快速的CPU上加以cpu_compensation倍数放大
        waitpid(pidApp,NULL, 0);
        printf("Acflag=%d\n",ACflg);
    }
    void tracee(const int &time_lmt,const int &mem_lmt){

        //准备环境变量处理中文，如果希望使用非中文的语言环境，可能需要修改这些环境变量
        char * const envp[]={(char * const )"PYTHONIOENCODING=utf-8",
                             (char * const )"LANG=zh_CN.UTF-8",
                             (char * const )"LANGUAGE=zh_CN.UTF-8",
                             (char * const )"LC_ALL=zh_CN.utf-8",NULL};
        if(nice(19)!=19) printf("......................renice fail... \n");
        // now the user is "kjctarr"
        if(chdir(box.workdir.c_str())){
            // write_log("Working directory :%s switch fail...",work_dir);
            exit(-4);
        }
        int re=chroot(box.workdir.c_str());
        std::cout<<"\n\nchroot result :"<<re<<std::endl;
        char dir[40];
        char *p = getcwd(dir , 40);
        std:std::cout<<"当前工作目录："<<dir<<std::endl;
        system("ls");
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        struct rlimit LIM;
        LIM.rlim_cur = (time_lmt) + 1;
        LIM.rlim_max = LIM.rlim_cur;
        //if(DEBUG) printf("LIM_CPU=%d",(int)(LIM.rlim_cur));
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(0);
        alarm( time_lmt );
        // file limit
        LIM.rlim_max = (STD_MB<<5) + STD_MB;
        LIM.rlim_cur = (STD_MB<<5);
        setrlimit(RLIMIT_FSIZE, &LIM);
        LIM.rlim_cur = LIM.rlim_max = 1;
        setrlimit(RLIMIT_NPROC, &LIM);

        // set the stack
        LIM.rlim_cur = STD_MB << 8;
        LIM.rlim_max = STD_MB << 8;
        setrlimit(RLIMIT_STACK, &LIM);
        // set the memory
        LIM.rlim_cur = STD_MB * mem_lmt / 2 * 3;
        LIM.rlim_max = STD_MB * mem_lmt * 2;
        setrlimit(RLIMIT_AS, &LIM);
        stdout=freopen("user.out", "w", stdout);
        stderr=freopen("error.out", "a+", stderr);
        execle("./Main", "./Main", (char *)NULL, (char *)NULL);
        //fflush(stderr);
        exit(0);
    }
    bool run(){

        int mem_lmt=512;
        int  time_lmt=1;
        int usedtime=0;
        int curmemory = 0;
        pid_t pid = fork();
        if (pid == 0){
            tracee(time_lmt,mem_lmt);
        }
        else{
            tracer(pid,time_lmt,mem_lmt,curmemory,usedtime);
            std::cout<<"内存："<<(curmemory>>20)<<"MB  时间："<<usedtime<<"ms"<<std::endl;
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
