#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include "lexer.h"
#include "parser.h"
#include<limits.h> //need this to get the path limits and all that otherwise the os keeps killing vscode when i do runs and hit infinite loops
// #ifndef PATH_MAX
// #define PATH_MAX 4096 //need this for vscode brerakpoint debugging since normal rrrun refuses to acknowledge that limist.sh has pathmax
// #endif
char* pwd;
char* home;
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
    else if(ret==0){
        printf("success\n");
        return;
    }
}
int main(){
    init_shell();
    while(1){
        getpwd();
        printf("<shura@iiit:%s>",pwd);
        char* cmd;
        cmd=malloc(999*sizeof(char));
        scanf("%[^\n]s",cmd); //read the command as the whole line breaaking  att new line char thats why this retarded scanf
        scanf("%*c"); //to eat the \n  from the previous scanf
        process_cmd(cmd);
    }
}