#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<limits.h>
#include<fcntl.h>
#include<stdbool.h>
#include "command.h"
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
    _exit(1);
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