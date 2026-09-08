#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include "lexer.h"
#include "parser.h"
#include "bins.h"
#include "command.h"
#include<stdbool.h>
#include<fcntl.h>
#include<sys/wait.h>
#include "main.h"
#include<signal.h> //to make the signal handling since the child process in bg might sent a signal to fg to print and all that
#include<errno.h> //to make the diiferentiation of errors easier
#include<limits.h> //need this to get the path limits and all that otherwise the os keeps killing vscode when i do runs and hit infinite loops
// #ifndef PATH_MAX
// #define PATH_MAX 4096 //need this for vscode brerakpoint debugging since normal rrrun refuses to acknowledge that limist.sh has pathmax
// #endif
char* pwd;
char* home;
char* user;
char* host;
char* prev;
pid_t shell_pgid; // will need this to recalim the shell whenever we come out of a process
int bg_size=0;
long long jobs=1;
typedef struct bg{
    pid_t pid;
    int job;
    char*cmd;
    pid_t pgid;
    bool sus;
    pid_t* members;
    int num; //this ds is getting out of handed
    char** name;
    char** full_cmd; //resume wants to print the full command
}bg;
bg* bg_list;
void assign_bg(pid_t pid,char* cmd,char** args){
    bg_size++;
    bg_list=realloc(bg_list,bg_size*sizeof(bg));
    bg_list[bg_size-1].pid=pid;
    bg_list[bg_size-1].job=jobs;
    bg_list[bg_size-1].cmd=cmd;
    bg_list[bg_size-1].pgid=pid;
    bg_list[bg_size-1].sus=false;
    bg_list[bg_size-1].members=malloc(sizeof(pid_t));
    bg_list[bg_size-1].num=1;
    bg_list[bg_size-1].members[0]=pid;
    bg_list[bg_size-1].name=malloc(sizeof(char*));
    bg_list[bg_size-1].name[0]=cmd;
    bg_list[bg_size-1].full_cmd=args;
    printf("[%lld] %d\n",jobs,pid);
    jobs++;
}
void assign_stopped(pid_t pid,char* cmd,char** args){
    bg_size++;
    bg_list=realloc(bg_list,bg_size*sizeof(bg));
    bg_list[bg_size-1].pid=pid;
    bg_list[bg_size-1].job=jobs;
    bg_list[bg_size-1].cmd=cmd;
    bg_list[bg_size-1].pgid=pid;
    bg_list[bg_size-1].sus=true;
    bg_list[bg_size-1].members=malloc(sizeof(pid_t));
    bg_list[bg_size-1].num=1;
    bg_list[bg_size-1].members[0]=pid;
    bg_list[bg_size-1].name=malloc(sizeof(char*));
    bg_list[bg_size-1].name[0]=cmd;
    bg_list[bg_size-1].full_cmd=args;
    printf("[%lld] + Stopped\t%s\n",jobs,cmd);
    jobs++;
}
void assign_bg_multi(pid_t* pids,int num,char** names,char* full_cmd,char** args){
    bg_size++;
    bg_list=realloc(bg_list,bg_size*sizeof(bg));
    bg_list[bg_size-1].pid=pids[0];
    bg_list[bg_size-1].job=jobs;
    bg_list[bg_size-1].cmd=full_cmd;
    bg_list[bg_size-1].pgid=pids[0];
    bg_list[bg_size-1].sus=false;
    bg_list[bg_size-1].num=num;
    bg_list[bg_size-1].members=malloc(num*sizeof(pid_t));
    bg_list[bg_size-1].name=malloc(num*sizeof(char*));
    for(int i=0;i<num;i++){
        bg_list[bg_size-1].members[i]=pids[i];
        bg_list[bg_size-1].name[i]=names[i];
    }
    bg_list[bg_size-1].full_cmd=args;
    printf("[%lld] %d\n",jobs,pids[0]);
    jobs++;
}
void assign_stopped_multi(pid_t* pids,char** cmds,int num,char* full_cmd,char** args){
    bg_size++;
    bg_list=realloc(bg_list,bg_size*sizeof(bg));
    bg_list[bg_size-1].pid=pids[0];
    bg_list[bg_size-1].job=jobs;
    bg_list[bg_size-1].cmd=full_cmd;
    bg_list[bg_size-1].pgid=pids[0];
    bg_list[bg_size-1].sus=true;
    bg_list[bg_size-1].num=num;
    bg_list[bg_size-1].members=malloc(num*sizeof(pid_t));
    bg_list[bg_size-1].name=malloc(num*sizeof(char*));
    for(int i=0;i<num;i++){
        bg_list[bg_size-1].members[i]=pids[i];
        bg_list[bg_size-1].name[i]=cmds[i];
    }
    bg_list[bg_size-1].full_cmd=args;
    printf("[%lld] + Stopped\t%s\n",jobs,full_cmd);
    jobs++;
}
void announce_bg(pid_t pid,bool stat){
    int job_idx=-1,member_idx=-1;
    char* name=NULL;
    for(int i=0;i<bg_size;i++){
        for(int m=0;m<bg_list[i].num;m++){
            if(bg_list[i].members[m]==pid){
                job_idx=i;member_idx=m;name=bg_list[i].name[m];
                break;
            }
        }
        if(job_idx!=-1) break;
    }
    if(job_idx==-1) return;
    if(stat)
    printf("%s with pid %d exited normally\n",name,pid);
    else
    printf("%s with pid %d exited abnormally\n",name,pid);
    for(int m=member_idx;m<bg_list[job_idx].num-1;m++){
        bg_list[job_idx].members[m]=bg_list[job_idx].members[m+1];
        bg_list[job_idx].name[m]=bg_list[job_idx].name[m+1];
    }
    bg_list[job_idx].num--;
    if(bg_list[job_idx].num==0){
        free(bg_list[job_idx].members);
        free(bg_list[job_idx].name);
        memmove(&bg_list[job_idx],&bg_list[job_idx+1],(bg_size-job_idx-1)*sizeof(bg));
        bg_size--;
        bg_list=realloc(bg_list,bg_size*sizeof(bg));
    }
}
void alarm_handler(int sig){ //apparently the alarm just kills the process it is called from so i need to just be able to accept it here even if this does nothing
    (void) sig;
}
typedef struct waiting{
    pid_t pid;
    bool stat;
}waiting;
waiting wait_list[100]; // like genuinely don't think more than 100 child processes will finish their work while some process is foregrounding
volatile sig_atomic_t wait_index=0; //volatile since it will be changed in the signal handler and sig_atomic_t since it will be changed in the signal handler
void no_longer_waiting(){
    sigset_t old,blocking;
    sigemptyset(&blocking);
    sigaddset(&blocking,SIGCHLD);
    sigprocmask(SIG_BLOCK,&blocking,&old);
    for(int i=0;i<wait_index;i++){
        announce_bg(wait_list[i].pid,wait_list[i].stat);
    }
    wait_index=0;
    sigprocmask(SIG_SETMASK,&old,NULL); //llm generated code did not know how to write this 
}
void plant(int sig){ //since it will kill zombies, you know pvz reference
    (void)sig; //this is the sigchild boilerplat from stackoverflow as well
    int err_no=errno;
    int status=0;
    pid_t pid;
    while((pid=waitpid(-1,&status,WNOHANG))>0){
        if(wait_index<100){
            wait_list[wait_index].pid=pid;
            wait_list[wait_index].stat=WIFEXITED(status) && WEXITSTATUS(status) == 0;
            wait_index++;
        }
    }
    errno=err_no;
}
bool ctrld1(){
    for(int i=0;i<bg_size;i++){
        if(bg_list[i].sus==true)
        return true;
    }
    return false;
}
void ctrld2(){
    for(int i=0;i<bg_size;i++){
        killpg(bg_list[i].pgid,SIGHUP);
    }
}
void prevdir(){
    prev=calloc(PATH_MAX,sizeof(char)); //just initialsing prev and will do this once could have been in init shell but oh well
    return;
}
void str_replace(char* old,char* sstr,char* new){
    char*pos;
    pos=strstr(old,sstr);
    if ((pos==NULL)){
        return;
    }
    while(pos==old){
        int len=strlen(old)+strlen(new)-strlen(sstr)+1;
        char* temp=calloc(len,sizeof(char)); // if i used calloc after the first run due to the appending \0 the string starrrted maaking random ascii output so  i had to fix tthat hence stackoveflow says calloc does that
        strncpy(temp,old,pos-old);
        strcat(temp,new);
        strcat(temp,pos+strlen(sstr));
        strcpy(old,temp);
        free(temp);
        pos=strstr(old,sstr);
    }
}
void init_shell(){
    pwd=malloc(PATH_MAX * sizeof(char));
    getcwd(pwd,PATH_MAX);
    user=malloc(100 * sizeof(char));
    host=malloc(100 * sizeof(char));
    getlogin_r(user, 100);
    gethostname(host, 100);
    home=&pwd[0];
    struct sigaction sa={0}; //sigaction api boiler plate apparently previously they use to use signal function thats where the header name comes from
    sa.sa_handler=plant;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART;
    sigaction(SIGCHLD,&sa,NULL);
    struct sigaction sa2={0}; //sa2 part is llm generated code but essentially what it is doing is it is attaching a function for internal signals which is alarm handler here so that when we get the sigalrm signal we can actually take it and handle it instead of just dying off
    sa2.sa_handler=alarm_handler;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags=0;
    sigaction(SIGALRM,&sa2,NULL);
    shell_pgid=getpid();
    setpgid(shell_pgid,shell_pgid);
    tcsetpgrp(STDIN_FILENO,shell_pgid);//claiming the shell in init
    signal(SIGINT,SIG_IGN);  //ctrl c handled
    signal(SIGTSTP,SIG_IGN); //ctrl z handled
    signal(SIGTTOU,SIG_IGN); //requirement stop background writes now as well
    signal(SIGQUIT,SIG_IGN); //since the requirement states a different way of quitting out of the shell
}
char* getpwd(){
    pwd=malloc(PATH_MAX * sizeof(char));
    getcwd(pwd,PATH_MAX);
    str_replace(pwd,home,"~");
    return pwd;
}
void process_cmd(char* cmd){
    token *args;
    args=malloc(100*sizeof(token));
    int len=0;
    if(lexer(cmd,args,&len)==-1){
        printf("cshell: invalid syntax\n");
        return;
    }
    // for (int i=0;i<len;i++){
    //     printf("%s %d\n",args[i].value,args[i].type); //lex token stream test
    // }
    node **coms;
    coms=malloc(100*sizeof(node*));
    int len1=0;
    int ret=parser(args,len,coms,&len1);
    if(ret==-1){
        printf("cshell: invalid syntax\n");
        return;
    }
    for(int i=0;i<len1;i++){
        for(int j=0;j<coms[i]->arg_index;j++){
            if(coms[i]->args[j][0]=='~'){
                str_replace(coms[i]->args[j],"~",home);
            }
        }
        if(coms[i]->piped){
            int start=i;
            while(i<len1 && coms[i]->piped){
                i++;
            }
            int end=i;
            for(int k=start+1;k<=end;k++){
                for(int j=0;j<coms[k]->arg_index;j++){
                    if(coms[k]->args[j][0]=='~'){
                        str_replace(coms[k]->args[j],"~",home);
                    }
                }
            }
            piped(coms,start,end,home,prev,coms[end]->background);
            continue; //for requirement of part c ; and & should end but this needs to be removed when starting part D
            //return;
        }
        char *in_files[100];
        int num_in=ext_in(coms[i]->args,&coms[i]->arg_index,in_files);
        if(num_in<0){
            continue;
        }
        char *out_files[100];
        bool appends[100];
        int num_out=ext_out(coms[i]->args,&coms[i]->arg_index,out_files,appends);
        if(num_out<0){
            if(coms[i]->background || coms[i]->semi){
                return;
            }
            continue;
        }
        bool need_redir;
        if(num_in>0 || num_out>0){
            need_redir=true;
        }
        else{
            need_redir=false;
        }
        if(strcmp(coms[i]->cmd,"hop")==0)
        {
            pid_t pid=-2; //pid tracker //changed 0 to -2 since the self report conditions at the bottom works like that
            //int job_tracker=-1; //will make the announcement easier after the process ends //this implementation kept breaking 
            if(coms[i]->background)
            {
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){ //essential if this is a background process we will fork it finish and then just continue, not gonna wait for it to finish
                if (pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if (pid==0){
                setpgid(0,0); //setting up the group pid so that we can implement it in the activites part
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY); //basic opening the devnull then pointing the read end from fd 0 to devnull so that input is blocked to bg processes, i know its not needed in hop but is needed in others so better to just put everywhere
                if(devnull>=0){
                    dup2(devnull,STDIN_FILENO);
                    close(devnull);
                }
            }
            bool changed=false;
            char* ccwd=calloc(PATH_MAX,sizeof(char));
            char* cwd=malloc(PATH_MAX * sizeof(char));
            getcwd(cwd,PATH_MAX);
            if(need_redir){
                pid_t pid2=fork();
                if(pid2==0){
                    setpgid(pid2,pid2);
                    if(redir_in(in_files,num_in)<0)
                    {
                        _exit(1);
                    }
                    if(redir_out(out_files,appends,num_out)<0)
                    {
                        _exit(1);
                    }
                    hop(coms[i]->args,coms[i]->arg_index,&changed,ccwd,prev,home);
                    _exit(1);
                }
                else{
                    int status=0;
                    tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                    waitpid(pid2,&status,WUNTRACED); // to check for stop we need to use untraced and not 0 otherwise your code will not detect ctrl z even though you think it should ....
                    tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                    if(WIFSTOPPED(status)) 
                    assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                }
            }
            else
            hop(coms[i]->args,coms[i]->arg_index,&changed,ccwd,prev,home);
            if(pid==0){
                free(ccwd);
                free(cwd);
                _exit(0);
            }
        }
        else if(strcmp(coms[i]->cmd,"reveal")==0)
        {
            pid_t pid=-2;
            if(coms[i]->background)
            {
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){
                if (pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if (pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){
                    dup2(devnull,STDIN_FILENO);
                    close(devnull);
                }
            }
            if (coms[i]->args[1] == NULL) {
                coms[i]->args[1] = ".";
                coms[i]->arg_index++;
                if(need_redir){
                    pid_t pid2=fork();
                    if(pid2==0){
                        setpgid(pid2,pid2);
                        if(redir_in(in_files,num_in)<0)
                        {
                            _exit(1);
                        }
                        if(redir_out(out_files,appends,num_out)<0)
                        {
                            _exit(1);
                        }
                        reveal(coms[i]->args, 2);
                        _exit(1);
                    }
                    else{
                        int status=0;
                        tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                        waitpid(pid2,&status,WUNTRACED);
                        tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                        if(WIFSTOPPED(status)) 
                        assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                    }
                }
                else
                reveal(coms[i]->args, 2);
                if(pid==0){
                    _exit(0);
                }
                continue;
            }
            int j=1;
            bool preverr=false;
            while(j<coms[i]->arg_index && coms[i]->args[j][0]=='-'){
                if(strcmp(coms[i]->args[j],"-")==0){
                    if(strcmp(prev,"")==0){
                        printf("reveal: no such directory\n");
                        preverr=true;
                        i++;
                        if(i==len1){
                            break;
                        }
                        continue;
                    }
                    strcpy(coms[i]->args[j],prev);
                    j++;
                    break;
                }
                j++;
            }
            if(preverr){
                if(pid==0){
                    _exit(0);
                }
                continue;
            }
            
            if(j+1<coms[i]->arg_index){
                printf("reveal: invalid syntax\n");
                if(pid==0){
                    _exit(0);
                }
                continue;
            }
            else{
                if(need_redir){
                    pid_t pid2=fork();
                    if(pid2==0){
                        setpgid(0,0);
                        signal(SIGINT,SIG_DFL);
                        signal(SIGTSTP,SIG_DFL);
                        signal(SIGTTOU,SIG_DFL);
                        if(redir_in(in_files,num_in)<0)
                        {
                            _exit(1);
                        }
                        if(redir_out(out_files,appends,num_out)<0)
                        {
                            _exit(1);
                        }
                        reveal(coms[i]->args, coms[i]->arg_index);
                        _exit(1);
                    }
                    else{
                        int status=0;
                        tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                        waitpid(pid2,&status,WUNTRACED);
                        tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                        if(WIFSTOPPED(status)) 
                        assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                    }
                }
                else
                {
                    reveal(coms[i]->args, coms[i]->arg_index);
                    if(pid==0){
                        _exit(0);
                    }
                }
            }
        }
        else if(strcmp(coms[i]->cmd,"peek")==0)
        {
            pid_t pid=-2;  
            if(coms[i]->background){
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){
                if (pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if (pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){
                    dup2(devnull,STDIN_FILENO);
                    close(devnull);
                }
            }
            if(coms[i]->arg_index<2){
                coms[i]->args[1]="-";
                coms[i]->arg_index++;
            }
            int j=1;
            while(j<coms[i]->arg_index && coms[i]->args[j][0]=='-'){
                if(strcmp(coms[i]->args[j],"-")==0)
                break;
                j++;
            }
            if(j==coms[i]->arg_index){
                coms[i]->args[j]="-";
                coms[i]->arg_index++;
                j++;
            }
            while(j<coms[i]->arg_index){
                if(coms[i]->args[j][0]=='-'){
                    if(strcmp(coms[i]->args[j],"-")==0){
                        j++;
                        continue;
                    }
                    printf("peek: invalid syntax\n");
                    if(pid==0){
                        _exit(0);
                    }
                    return;
                }
                j++;
            }
            if(need_redir){
                pid_t pid2=fork();
                if(pid2==0){
                    setpgid(pid2,pid2);
                    if(redir_in(in_files,num_in)<0)
                    {
                        _exit(1);
                    }
                    if(redir_out(out_files,appends,num_out)<0)
                    {
                        _exit(1);
                    }
                    peek(coms[i]->args, coms[i]->arg_index);
                    _exit(1);
                }
                else{
                    int status=0;
                    tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                    waitpid(pid2,&status,WUNTRACED);
                    tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                    if(WIFSTOPPED(status)) 
                    assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                }
            }
            else
            {
                peek(coms[i]->args, coms[i]->arg_index);
                if(pid==0){
                    _exit(0);
                }
            }
        }
        else if(strcmp(coms[i]->cmd,"locate")==0)
        {
            pid_t pid=-2;
            if(coms[i]->background){
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){
                if (pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if (pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){
                    dup2(devnull,STDIN_FILENO);
                    close(devnull);
                }
            }
            if(coms[i]->arg_index<2){
                printf("locate: invalid syntax\n");
                if(pid==0){
                    _exit(0);
                }
                continue;
            }
            int k=0;
            char* new_args[100];
            int j=1;
            while(j<coms[i]->arg_index&&coms[i]->args[j][0]!='-'){
                new_args[k++]=coms[i]->args[j];
                j++;
            }
            new_args[k]=NULL;
            char* res=calloc(10000,sizeof(char));
            if(need_redir){
                pid_t pid2=fork();
                if(pid2==0){
                    setpgid(0,0);
                    signal(SIGINT,SIG_DFL);
                    signal(SIGTSTP,SIG_DFL);
                    signal(SIGTTOU,SIG_DFL);
                    if(redir_in(in_files,num_in)<0)
                    {
                        _exit(1);
                    }
                    if(redir_out(out_files,appends,num_out)<0)
                    {
                        _exit(1);
                    }
                    locate(new_args,res,k);
                    printf("%s", res);
                    free(res);
                    _exit(1);
                }
                else{
                    int status=0;
                    tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                    waitpid(pid2,&status,WUNTRACED);
                    tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                    if(WIFSTOPPED(status)) 
                    assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                }
            }
            else
            {
                locate(new_args,res,k);
                printf("%s", res);
            }
            free(res);
            if(pid==0){
                _exit(0);
            }
        }
        else if(strcmp(coms[i]->cmd,"activities")==0){
            pid_t pid=-2;
            if(coms[i]->background){
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){
                if(pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if(pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){ dup2(devnull,STDIN_FILENO); close(devnull); }
            }
            if(need_redir){
                pid_t pid2=fork();
                if(pid2==0){
                    setpgid(pid2,pid2);
                    if(redir_in(in_files,num_in)<0) _exit(1);
                    if(redir_out(out_files,appends,num_out)<0) _exit(1);
                    activities();
                    _exit(0);
                }
                else{
                    int status=0;
                    tcsetpgrp(STDIN_FILENO,pid2);
                    waitpid(pid2,&status,WUNTRACED);
                    tcsetpgrp(STDIN_FILENO,shell_pgid);
                    if(WIFSTOPPED(status))
                    assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                }
            }
            else
            activities();
            if(pid==0){
                _exit(0);
            }
        }
        else if(strcmp(coms[i]->cmd,"resume")==0){
            if(need_redir){
                int saved_in=-1,saved_out=-1;
                if(num_in>0){
                    saved_in=dup(STDIN_FILENO);
                    if(redir_in(in_files,num_in)<0){
                        if(saved_in>=0) close(saved_in);
                        continue;
                    }
                }
                if(num_out>0){
                    saved_out=dup(STDOUT_FILENO);
                    if(redir_out(out_files,appends,num_out)<0){
                        if(saved_in>=0){ dup2(saved_in,STDIN_FILENO); close(saved_in); }
                        if(saved_out>=0) close(saved_out);
                        continue;
                    }
                }
                resume(coms[i]->args,coms[i]->arg_index);
                if(saved_in>=0){ dup2(saved_in,STDIN_FILENO); close(saved_in); }
                if(saved_out>=0){ dup2(saved_out,STDOUT_FILENO); close(saved_out); }
            }
            else{
                resume(coms[i]->args,coms[i]->arg_index);
            }
        }
        else if(strcmp(coms[i]->cmd,"ping")==0){
            pid_t pid=-2;
            if(coms[i]->background){
                pid=fork();
            }
            if(pid!=0 && coms[i]->background){
                if(pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid);
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            if(pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){ dup2(devnull,STDIN_FILENO); close(devnull); }
            }
            if(need_redir){
                pid_t pid2=fork();
                if(pid2==0){
                    setpgid(pid2,pid2);
                    if(redir_in(in_files,num_in)<0) _exit(1);
                    if(redir_out(out_files,appends,num_out)<0) _exit(1);
                    ping(coms[i]->args,coms[i]->arg_index);
                    _exit(0);
                }
                else{
                    int status=0;
                    tcsetpgrp(STDIN_FILENO,pid2);
                    waitpid(pid2,&status,WUNTRACED);
                    tcsetpgrp(STDIN_FILENO,shell_pgid);
                    if(WIFSTOPPED(status))
                    assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                }
            }
            else
            ping(coms[i]->args,coms[i]->arg_index);
            if(pid==0){
                _exit(0);
            }
        }
        else if(coms[i]->cmd==NULL){
            continue;
        }
        else{
            pid_t pid=-2;  
            if(coms[i]->background){
                pid=fork();
            }
            if (pid==0){
                setpgid(0,0);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){
                    dup2(devnull,STDIN_FILENO);
                    close(devnull);
                }
                if(redir_in(in_files,num_in)<0)
                _exit(1);
                if(redir_out(out_files,appends,num_out)<0)
                _exit(1);
                run_cmd(coms[i]->cmd,coms[i]->args,coms[i]->arg_index);
                _exit(1);
            }
            if(pid!=0 && coms[i]->background){ //otherwise you keep getting failed to create child for non bg or well fg processes
                if (pid<0){
                    printf("cshell: failed to create child process\n");
                }
                else{
                    setpgid(pid,pid); // even parents will need to have group id i think it mmight not be used tho
                    assign_bg(pid,coms[i]->cmd,coms[i]->args);
                    continue;
                }
            }
            pid_t pid2=fork();
            if(pid2>0)
            setpgid(pid2,pid2);
            if(pid2==0){
                setpgid(pid2,pid2);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                signal(SIGTTOU,SIG_DFL);
                if(redir_in(in_files,num_in)<0)
                _exit(1);
                if(redir_out(out_files,appends,num_out)<0)
                _exit(1);
                run_cmd(coms[i]->cmd,coms[i]->args,coms[i]->arg_index);
                _exit(1);
            }
            else{
                int status=0;
                tcsetpgrp(STDIN_FILENO,pid2);// giving shell to current process
                waitpid(pid2,&status,WUNTRACED);
                tcsetpgrp(STDIN_FILENO,shell_pgid); //give back terminal to the shell
                if(WIFSTOPPED(status)) 
                assign_stopped(pid2,coms[i]->cmd,coms[i]->args);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 8){ // checking the exact command not found error and breaking, there is no rhyme and reason to use 8 just wanted to i guess
                    if(pid==0){
                        _exit(0);
                    }
                    break;
                }
            }
            if(pid==0){
                _exit(0);
            }
        }
    }
    no_longer_waiting(); //this will announce all the bg processes only when we are done running
}
int main(){
    init_shell();
    prevdir();
    bool ctrld=false;
    while(1){
        no_longer_waiting();
        getpwd();
        printf("<%s@%s:%s>",user,host,pwd);
        char* cmd;
        cmd=malloc(1026*sizeof(char));
        // scanf("%[^\n]s",cmd); //read the command as the whole line breaaking  att new line char thats why this retarded scanf
        // scanf("%*c"); //to eat the \n  from the previous scanf
        if(fgets(cmd,1026,stdin) == NULL){ // i truly hate fgets but in scanf when i just hit enter i produced garbage values so i have to use this
            free(cmd); // to stop the terminal from breaking when i use ctrl d given the result by claude
            printf("\n");
            if(ctrld1() && !ctrld){
                printf("cshell: there are stopped jobs\n");
                ctrld=true;
                clearerr(stdin); //fgets does not eat up the eof and it just exits after printing the there are stopped jobs thingy now it should work
                continue;
            }
            ctrld2();
            break;
        }
        ctrld=false;
        cmd[strcspn(cmd, "\n")] = 0; // remove the trailing newline character
        if(strlen(cmd)==0){
            free(cmd);
            continue;
        }
        process_cmd(cmd);
        free(cmd);
    }
}