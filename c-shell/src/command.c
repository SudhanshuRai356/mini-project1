#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<limits.h>
#include<fcntl.h>
#include<stdbool.h>
#include<signal.h>
#include "command.h"
#include "parser.h"
#include "bins.h"
#include "main.h"
extern pid_t shell_pgid;
void assign_stopped(pid_t pid,char* cmd,char** args);
void assign_bg_multi(pid_t* pids,int num,char** names,char* full_cmd,char** args);
void assign_stopped_multi(pid_t* pids,char** cmds,int num,char* full_cmd,char** args);
bool builtin(char* cmd){
    if(!cmd)
    return false;
    return (strcmp(cmd, "hop") == 0 || strcmp(cmd, "reveal") == 0 ||strcmp(cmd, "peek") == 0 || strcmp(cmd, "locate") == 0);
}
void exec_builtin(node* com, char* home, char* prev){
    if (strcmp(com->cmd, "hop") == 0) {
        bool changed = false;
        char* ccwd = calloc(PATH_MAX, sizeof(char));
        char* cwd = malloc(PATH_MAX * sizeof(char));
        getcwd(cwd, PATH_MAX);
        hop(com->args, com->arg_index, &changed, ccwd, prev, home);
        free(ccwd);
        free(cwd);
    } else if (strcmp(com->cmd, "reveal") == 0) {
        if (com->args[1] == NULL) {
            com->args[1] = ".";
            com->arg_index++;
        }
        reveal(com->args, com->arg_index);
    } else if (strcmp(com->cmd, "peek") == 0) {
        peek(com->args, com->arg_index);
    } else if (strcmp(com->cmd, "locate") == 0) {
        int k = 0;
        char* new_args[100];
        int j = 1;
        while (j < com->arg_index && com->args[j][0] != '-') {
            new_args[k++] = com->args[j];
            j++;
        }
        new_args[k] = NULL;
        char* res = calloc(10000, sizeof(char));
        locate(new_args, res, k);
        printf("%s", res);
        free(res);
    }
}
void run_cmd(char* cmd,char** args,int arg_index){
    if(!cmd||cmd[0]=='\0'){
        _exit(0);
    }
    args[arg_index]=NULL;
    if(strchr(cmd,'/')!=NULL){
        execv(cmd,args);
        printf("cshell: command not found (%s)\n",cmd);
        _exit(1);
    }
    if(cmd[0]=='%'){
        char* tar=cmd+1;
        args[0]=tar;
        execvp(tar,args);
        printf("cshell: command not found (%s)\n",tar);
        _exit(1);
    }
    char cwd[PATH_MAX];
    snprintf(cwd,PATH_MAX,"./%s",cmd);
    struct stat st;
    if(stat(cwd,&st)==0&&S_ISREG(st.st_mode)&&access(cwd,X_OK)==0){
        execv(cwd,args);
    }
    execvp(cmd,args);
    printf("cshell: command not found (%s)\n",cmd);
    _exit(8); //to differentiate from normal command errors
}
int redir_in(char**in,int num){
    if(num==0)
    return 0;
    for(int i=0;i<num;i++){//checking if all files are readable otherwise dup will start panicking
        int fd=open(in[i],O_RDONLY);
        if(fd<0){
            printf("cshell: no such file or directory\n");
            return -1;
        }
        close(fd);
    }
    if(num==1){
        int fd=open(in[0],O_RDONLY);
        if(fd<0)
        return -1;
        dup2(fd,STDIN_FILENO);// redireting input reader to the open file fd
        close(fd);
        return 0;
    }
    else{
        int pip[2];
        if(pipe(pip)<0)
        return -1;
        pid_t pid=fork();
        if(pid==0){
            close(pip[0]);// don't actually need to read rather just write
            char *buff;
            buff=calloc(4096,sizeof(char));
            for(int i=0;i<num;i++){
                int fd=open(in[i],O_RDONLY);
                if(fd>=0){
                    ssize_t bytes_read;
                    while((bytes_read=read(fd,buff,4096))>0){
                        write(pip[1],buff,bytes_read);
                    }
                    close(fd);
                }
            }
            free(buff);
            close(pip[1]);
            _exit(0);
        }
        close(pip[1]);
        dup2(pip[0],STDIN_FILENO);
        close(pip[0]);
        return 0;
    }
}
int redir_out(char**out,bool *append,int num){
    if(!out)
    return 0;
    if(num==0)
    return 0;
    int fds[num+1];
    for(int i=0;i<num;i++){        
        int flag;
        if(append[i]){
            flag=O_WRONLY|O_CREAT|O_APPEND; // the flags for write only, create a file if does not exist and the flag to append so that we are in >> mode
        }
        else
        flag=O_WRONLY|O_CREAT|O_TRUNC; // the flag for truncate which means we are in > mode
        fds[i]=open(out[i],flag,0644);
        if(fds[i]<0){
            printf("cshell: unable to create file for writing\n");
            for(int j=0;j<i;j++){
                close(fds[j]);
            }
            return -1;
        }
    }
    if(num==1){
        dup2(fds[0],STDOUT_FILENO);
        close(fds[0]);
        return 0;
    }
    int pip[2];
    if(pipe(pip)<0){
        for(int j=0;j<num;j++){
            close(fds[j]);
        }
        return -1;
    }
    pid_t pid=fork();
    if(pid<0){
        for(int i=0;i<num;i++)
        close(fds[i]);
        close(pip[0]);
        close(pip[1]);
        return -1;
    }
    if(pid==0){
        close(pip[1]);
        char* buff;
        buff=calloc(4096,sizeof(char));
        ssize_t bytes_read;
        while ((bytes_read = read(pip[0], buff, sizeof(buff))) > 0) {
            for (int i = 0; i < num; i++) {
                write(fds[i], buff, bytes_read);
            }
        }
        close(pip[0]);
        for(int i=0;i<num;i++)
        close(fds[i]);
        _exit(0);
    }
    else{
        close(pip[0]);
        dup2(pip[1], STDOUT_FILENO);
        close(pip[1]);
        for (int i = 0; i < num; i++) close(fds[i]);
        return 0;
    }
    return 0;
}
int ext_in(char **args,int *args_index,char** in){
    int count=0;
    int num_in=0;
    for(int i=0;i<*args_index;i++){
        if(strcmp(args[i],"<")==0){
            if(i+1<*args_index && strcmp(args[i+1],"<")!=0 && strcmp(args[i+1],">")!=0 && strcmp(args[i+1],">>")!=0)
            in[num_in++]=args[++i];
            else{
                printf("cshell: invalid syntax\n");
                return -1;
            }
        }
        else
        args[count++]=args[i];
    }
    args[count]=NULL;
    *args_index=count;
    return num_in;
}
int ext_out(char **args,int *args_index,char** out,bool* append){
    int count=0;
    int num_out=0;
    for(int i=0;i<*args_index;i++){
        if(strcmp(args[i],">")==0){
            if(i+1<*args_index && strcmp(args[i+1],"<")!=0 && strcmp(args[i+1],">")!=0 && strcmp(args[i+1],">>")!=0){
                out[num_out++]=args[++i];
                append[num_out-1]=false;
            }
            else{
                printf("cshell: invalid syntax\n");
                return -1;
            }
        }
        else if(strcmp(args[i],">>")==0){
            if(i+1<*args_index && strcmp(args[i+1],"<")!=0 && strcmp(args[i+1],">")!=0 && strcmp(args[i+1],">>")!=0){
                out[num_out++]=args[++i];
                append[num_out-1]=true;
            }
            else{
                printf("cshell: invalid syntax\n");
                return -1;
            }
        }
        else
        args[count++]=args[i];
    }
    args[count]=NULL;
    *args_index=count;
    return num_out;
}
void piped(node** coms,int start,int end,char*home,char* prev,bool back){
    int num=end-start+1;
    int pipes[num][2];
    for(int k=0;k<num-1;k++){
        if(pipe(pipes[k])<0){
            printf("cshell: pipe not made"); //no error message specified for this but need to put it here still so this is it
            return;
        }
    }
    pid_t pids[num];
    char* cmd_names[num];
    for(int k=0;k<num;k++){
        node* cur_cmd=coms[start+k];
        cmd_names[k]=cur_cmd->cmd;
        char *in_files[100];
        int num_in=ext_in(cur_cmd->args,&cur_cmd->arg_index,in_files);
        char*out_files[100];
        bool appends[100];
        int num_out=ext_out(cur_cmd->args,&cur_cmd->arg_index,out_files,appends);
        pids[k]=fork();
        if(pids[k]>0){
            if(k==0)
            setpgid(pids[k],pids[k]);
            else
            setpgid(pids[k],pids[0]);
        }
        if(pids[k]==0){
            if(k==0)
            setpgid(0,0); //for pipes that is grouped ones first is the leader the rest are just group members
            else
            setpgid(0,pids[0]);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);
            signal(SIGTTOU,SIG_DFL);
            if(back){
                int devnull=open("/dev/null",O_RDONLY);
                if(devnull>=0){ dup2(devnull,STDIN_FILENO); close(devnull); }
            }
            if(k>0){
                dup2(pipes[k-1][0],STDIN_FILENO);
            }
            if(k<num-1){
                dup2(pipes[k][1],STDOUT_FILENO);
            }
            for(int p=0;p<num-1;p++){
                close(pipes[p][0]);
                close(pipes[p][1]);
            }
            if(num_in>0){
                if(redir_in(in_files,num_in)<0)
                _exit(1);
            }
            else if(num_in<0)
            _exit(0);
            if (num_out > 0) {
                if (redir_out(out_files, appends, num_out) < 0) 
                _exit(1);
            } else if (num_out < 0) {
                _exit(1);
            }
            if(builtin(cur_cmd->cmd)){
                exec_builtin(cur_cmd,home,prev);
                _exit(0);
            }
            else{
                run_cmd(cur_cmd->cmd, cur_cmd->args, cur_cmd->arg_index);
                _exit(1);
            }
        }
    }
    for (int p = 0; p < num - 1; p++) {
        close(pipes[p][0]);
        close(pipes[p][1]);
    }
    if(back){
        assign_bg_multi(pids,num,cmd_names,coms[start]->cmd,coms[start]->args);
        return;
    }
    tcsetpgrp(STDIN_FILENO, pids[0]);
    bool any_stopped=false;
    for (int k = 0; k < num; k++) {
        int status;
        waitpid(pids[k], &status, WUNTRACED);
        if(WIFSTOPPED(status)) any_stopped=true;
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    if(any_stopped){
        assign_stopped_multi(pids,cmd_names,num,coms[start]->cmd,coms[start]->args);
    }
}
