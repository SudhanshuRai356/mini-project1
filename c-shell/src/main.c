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
#include<sys/wait.h>
#include<limits.h> //need this to get the path limits and all that otherwise the os keeps killing vscode when i do runs and hit infinite loops
// #ifndef PATH_MAX
// #define PATH_MAX 4096 //need this for vscode brerakpoint debugging since normal rrrun refuses to acknowledge that limist.sh has pathmax
// #endif
char* pwd;
char* home;
char* user;
char* host;
char* prev;
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
            while(i<len && coms[i]->piped){
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
            piped(coms,start,end,home,prev);
            continue;
        }
        if(strcmp(coms[i]->cmd,"hop")==0)
        {
            bool changed=false;
            char* ccwd=calloc(PATH_MAX,sizeof(char));
            char* cwd=malloc(PATH_MAX * sizeof(char));
            getcwd(cwd,PATH_MAX);
            hop(coms[i]->args,coms[i]->arg_index,&changed,ccwd,prev,home);
        }
        else if(strcmp(coms[i]->cmd,"reveal")==0)
        {
            if (coms[i]->args[1] == NULL) {
                coms[i]->args[1] = ".";
                coms[i]->arg_index++;
                reveal(coms[i]->args, 2);
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
                continue;
            }
            
            if(j+1<coms[i]->arg_index){
                printf("reveal: invalid syntax\n");
                continue;
            }
            else{
                reveal(coms[i]->args, coms[i]->arg_index);
            }
        }
        else if(strcmp(coms[i]->cmd,"peek")==0)
        {
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
                    return;
                }
                j++;
            }
            peek(coms[i]->args, coms[i]->arg_index);
        }
        else if(strcmp(coms[i]->cmd,"locate")==0)
        {
            if(coms[i]->arg_index<2){
                printf("locate: invalid syntax\n");
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
            locate(new_args,res,k);
            printf("%s", res);
            free(res);
        }
        else if(coms[i]->cmd==NULL){
            continue;
        }
        else{
            char* in_files[100];
            int num_in=ext_in(coms[i]->args,&coms[i]->arg_index,in_files);
            if(num_in<0)
            continue;
            char *out_files[100];
            bool appends[100];
            int num_out=ext_out(coms[i]->args,&coms[i]->arg_index,out_files,appends);
            if(num_out<0)
            continue;
            pid_t pid=fork();
            if(pid==0){
                if(redir_in(in_files,num_in)<0)
                _exit(1);
                if(redir_out(out_files,appends,num_out)<0)
                _exit(1);
                run_cmd(coms[i]->cmd,coms[i]->args,coms[i]->arg_index);
                _exit(1);
            }
            else{
                int status;
                waitpid(pid,&status,0);
            }
        }
    }
}
int main(){
    init_shell();
    prevdir();
    while(1){
        getpwd();
        printf("<%s@%s:%s>",user,host,pwd);
        char* cmd;
        cmd=malloc(999*sizeof(char));
        // scanf("%[^\n]s",cmd); //read the command as the whole line breaaking  att new line char thats why this retarded scanf
        // scanf("%*c"); //to eat the \n  from the previous scanf
        fgets(cmd,999,stdin); // i truly hate fgets but in scanf when i just hit enter i produced garbage values so i have to use this
        cmd[strcspn(cmd, "\n")] = 0; // remove the trailing newline character
        if(strlen(cmd)==0){
            free(cmd);
            continue;
        }
        process_cmd(cmd);
        free(cmd);
    }
}