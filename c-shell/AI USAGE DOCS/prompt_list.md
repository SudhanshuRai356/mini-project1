### Prompt 1

```LINE  ->  ε
      |   WORD ARG

ARG   ->  ε
      |   WORD    ARG
      |   OP_LT   TGT
      |   OP_GT   TGT
      |   OP_GTGT TGT
      |   OP_PIPE CMD
      |   OP_SEMI CMD
      |   OP_AMP  BG

CMD   ->  WORD ARG

TGT   ->  WORD ARG

BG    ->  ε
      |   WORD ARG
ok that  worked  above is the grammar how am i supposed to make a parser, since its rlg the parse tree is actually a parse linked list or i want  to just make it an array or something
```

![alt text](image.png)
![alt text](image-1.png)
![alt text](image-2.png)
![alt text](image-3.png)

used this prompt to understand how i would convert the given grammar to parser had some intution but the actual implementation was  not clicking the struct idea given by AI helped a lot

### Prompt 2
 ```
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h> // i guess even if the directory is posix it does not need to have everything its non posix variantttt had so retarded
#include<sys/stat.h> //stackoverflow recommended swap for d_type
#include<stdbool.h>
#include <limits.h>
#include "bins.h"
void hop(char**args,int len){
    //hoga kuch need a blocker cleared
}
void reveal(char **args,int len,char* res){
    bool hidden=false;
    bool recurse=false;
    if(len==2){ //readdir does not give a sorted array and too  much work to sort it myself so the convinient function scandir idk why the documentation did not have this prior prolly due to  alphabetical order full block reqwritten
        char* cur_dir=args[1];
        if (args[1][0]=='-'){
            for(size_t j=1;j<strlen(args[1]);j++){
                if(args[1][j]=='a'){
                    hidden=true;
                }
                else if(args[1][j]=='t'){
                    recurse=true;
                }
                else{
                    printf("cshell: invalid option %c\n",args[1][j]);
                    return;
                }
            }
            cur_dir=".";
        }
        struct dirent **list;
        int n=scandir(cur_dir,&list,NULL,alphasort);
        if(n<0){
            printf("reveal: no such directory\n");
            return;
        }
        for (int k=0;k<n;k++){
            if(!hidden && list[k]->d_name[0]=='.'){// i know there is no way fir hidden to be true but i did copied fromn below and now i realised instead of ranting i could have removed it,eh
                free(list[k]); // lol hidden was something that can happen when i tried reveal -t i got cooked re write now
                continue;
            }
            char *path=malloc(PATH_MAX*sizeof(char)); //boilerplatte implementation of the sys/stat.h
            if(strcmp(cur_dir,".")==0){
                snprintf(path,PATH_MAX,"%s",list[k]->d_name);
            }
            else if(cur_dir[strlen(cur_dir)-1]=='/'){
                snprintf(path,PATH_MAX,"%s%s",cur_dir,list[k]->d_name);
            }
            else{
                snprintf(path,PATH_MAX,"%s/%s",cur_dir,list[k]->d_name);
            }
            struct stat path_stat;
            bool dir=false;
            if(stat(path, &path_stat)==0 && S_ISDIR(path_stat.st_mode))
                dir=true;
            if(recurse || hidden){
                strcat(res,path);
                if(dir)
                strcat(res,"/");
                strcat(res,"\n");
            }
            else{
                strcat(res,list[k]->d_name);
                strcat(res," ");
            }
            if(recurse && dir){
                if(strcmp(list[k]->d_name,".")==0 || strcmp(list[k]->d_name,"..")==0){
                    free(path);
                    free(list[k]);
                    continue;
                }
                char *new_args[4];
                new_args[0] = args[0];
                new_args[1] = args[1];
                new_args[2] = path;
                new_args[3] = NULL;
                reveal(new_args, 3, res);
            }
            free(path);
            free(list[k]);
        }
        free(list);
    }
    else if(len>1){
        int i=1;
        while(args[i] !=NULL && args[i][0]=='-'){
            for(size_t j=1;j<strlen(args[i]);j++){ //because strlen directly returns size_t not int, only ever did int x=strlen so never knew
                if(args[i][j]=='a'){
                    hidden=true;
                }
                else if(args[i][j]=='t'){
                    recurse=true;
                }
                else{
                    printf("cshell: invalid option %c\n",args[i][j]);
                    return;
                }
            }
            i++;
        }
        char* dirs;
        if(i>=len){
           dirs=".";
        }
        else if(i+1!=len){
            printf("reveal: invalid syntax\n");
            return;
        }
        else{
            dirs=args[i];
        }
        struct dirent **list;
        int n=scandir(dirs,&list,NULL,alphasort);
        if(n<0){
            printf("reveal: no such directory\n");
            return;
        }
        for(int k=0;k<n;k++){
            struct dirent *entry=list[k];
            if(!hidden && list[k]->d_name[0]=='.'){
                free(list[k]);
                continue;
            }
            char *path=malloc(PATH_MAX*sizeof(char)); //boilerplatte implementation of the sys/stat.h
            if(strcmp(dirs,".")==0){//gotta match formatting given
                snprintf(path,PATH_MAX,"%s",entry->d_name);//i just saw the code block uses snprintf instead of printf , so googled and found out snprintf is used because of some buffer overflow safety practice and it can print to a buffer instead of stdout thats why most boilerplates use this
            }
            else if(dirs[strlen(dirs)-1]=='/'){
                snprintf(path,PATH_MAX,"%s%s",dirs,entry->d_name);
            }
            else{
                snprintf(path,PATH_MAX,"%s/%s",dirs,entry->d_name);
            }
            struct stat path_stat;
            bool dir=false;
            if(stat(path, &path_stat)==0 && S_ISDIR(path_stat.st_mode))
                dir=true;
            if(recurse || hidden){
                strcat(res,path);
                if(dir)
                strcat(res,"/");
                strcat(res,"\n");
            }
            else{
                strcat(res,entry->d_name);
                strcat(res," ");
            }
            if(recurse && dir){
                if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0){
                    free(path);
                    free(list[k]);
                    continue;
                }
                char *new_args[len+1];
                for(int j=0;j<len-1;j++){
                    new_args[j]=args[j];
                }
                new_args[len-1]=path;
                new_args[len]=NULL;
                reveal(new_args, len, res);
            }
            free(path);
            free(list[k]);
        }
        free(list);
    }
}
void locate(char** filename,char* res,int k){
    char* args[3];
    char *mid;
    for(int i=0;i<k;i++){
        mid=calloc(10000,sizeof(char));
        args[0]="reveal";
        args[1]="-t";
        args[2]=".";
        reveal(args,2,mid);
        char *pos =strstr(mid,filename[i]);
        if((pos)!=NULL){
            int j=0;
            while(&(pos-j)!='\0'&& &(pos-j)!='\n'&& &(pos-j)!=' '){
                j++;
            }
            if (pos-j==NULL){
                strcat(res,"locate: command not found (");
                strcat(res,filename[i]);
                strcat(res,")\n");
            }
            else
            strcat(res,&(pos-j+1));
        }
        free(mid);
        mid=calloc(10000,sizeof(char));
        args[0]="reveal";
        args[1]="-t";
        args[2]="/";
        reveal(args,2,mid);
        pos =strstr(mid,filename[i]);
        if((pos)!=NULL){
            int j=0;
            while(&(pos-j)!='\0'&& &(pos-j)!='\n'&& &(pos-j)!=' '){
                j++;
            }
            if (pos-j==NULL){
                strcat(res,"locate: command not found (");
                strcat(res,filename[i]);
                strcat(res,")\n");
            }
            else
            strcat(res,&(pos-j+1));
        }
        free(mid);
    }
}
above is updated bins.c and below is updated main.c
#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include "lexer.h"
#include "parser.h"
#include "bins.h"
#include<limits.h> //need this to get the path limits and all that otherwise the os keeps killing vscode when i do runs and hit infinite loops
// #ifndef PATH_MAX
// #define PATH_MAX 4096 //need this for vscode brerakpoint debugging since normal rrrun refuses to acknowledge that limist.sh has pathmax
// #endif
char* pwd;
char* home;
char* user;
char* host;
void str_replace(char* old,char* sstr,char* new){
    char*pos;
    pos=strstr(old,sstr);
    if ((pos==NULL)){
        return;
    }
    while(pos==old){
        int len=strlen(old)+strlen(new)-strlen(sstr)+1;
        char* temp=calloc(len,sizeof(char)); // if i used calloc after the first run due to the appending \0 the string starrrted maaking random ascii output so  i had to fix tthat hence stackoveflow says calloc does that
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
    //     printf("%s %d\n",args[i].value,args[i].type); //lex token stream test
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
        if(strcmp(coms[i]->cmd,"hop")==0)
        {
            // Handle hop command
        }
        else if(strcmp(coms[i]->cmd,"reveal")==0)
        {
            char* res=calloc(10000,sizeof(char));
            if (coms[i]->args[1] == NULL) {
                coms[i]->args[1] = ".";
                coms[i]->arg_index++;
                reveal(coms[i]->args, 2, res);
                printf("%s", res);
                if(res[strlen(res)-1]!='\n'){
                    printf("\n");
                }
                free(res);
                continue;
            }
            int j;
            for(j=1;j<coms[i]->arg_index;j++){
                if(coms[i]->args[j][0]=='-'){
                    continue;
                }
            }
            if(j+1<coms[i]->arg_index){
                printf("reveal: invalid syntax\n");
                free(res);
                return;
            }
            else{
                reveal(coms[i]->args, coms[i]->arg_index, res);
                printf("%s", res);
                if(res[strlen(res)-1]!='\n'){
                    printf("\n");
                }
                free(res);
            }
        }
        else if(strcmp(coms[i]->cmd,"peek")==0)
        {
            // Handle peek command
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
            printf("cshell: command not found: %s\n",coms[i]->cmd);
        }
    }
}
int main(){
    init_shell();
    while(1){
        getpwd();
        printf("<%s@%s:%s>",user,host,pwd);
        char* cmd;
        cmd=malloc(999*sizeof(char));
        // scanf("%[^\n]s",cmd); //read the command as the whole line breaaking  att new line char thats why this retarded scanf
        // scanf("%*c"); //to eat the \n  from the previous scanf
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
i am getting an OOM in my implementation of a find like program in my cshell implementation what is the cause of this issue, is there a logic error?
```

![alt text](image-4.png)
![alt text](image-5.png)
![alt text](image-6.png)

### Prompt 3
```
ok so similarly i need to put a function callled peek into my bins which has to “n”: counts the number of non-empty lines in the input. The line number 
must be printed before the corresponding line.
“r”: print the contents of the input in reverse line order. For seekable
 input (regular files), use lseek to read the file backwards in 
fixed-size chunks rather than loading it all into memory at once. For 
non-seekable input (pipes, standard input), buffering the full input 
before reversing is allowed., explain don't give me the whole code, boiler plate is fine
```

![alt text](image-7.png)
![alt text](image-8.png)
![alt text](image-9.png)
![alt text](image-10.png)