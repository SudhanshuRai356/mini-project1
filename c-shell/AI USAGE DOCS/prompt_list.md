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

### Prompt 4

```
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h> // i guess even if the directory is posix it does not need to have everything its non posix variantttt had so retarded
#include<sys/stat.h> //stackoverflow recommended swap for d_type
#include<stdbool.h>
#include <limits.h>
#include<fcntl.h>
#include<float.h>
#include<time.h>
#include "bins.h"
char *list;
bool search(char*home,char*path,char*match){
    char filepath[PATH_MAX];
    snprintf(filepath,PATH_MAX,"%s/frerency",home);
    FILE *file=fopen(filepath,"r");
    if(file==NULL){
        return false;
    }
    float max_rank=-1.0;
    char line[PATH_MAX+100];
    bool matched=false;
    time_t ct=time(NULL);
    while(fgets(line,sizeof(line),file)){
        element el;
        if(sscanf(line,"{p:\"%[^\"]\",s:\"%d\",t:\"%ld\"}",el.path,&el.score,&el.stamp)==3){
            if(strstr(el.path,path)!=NULL){
                struct stat st;
                if(stat(el.path,&st)==0&&S_ISDIR(st.st_mode)){
                    double units_old=difftime(ct,el.stamp)/12813.0; //3 hours 33 min 33 sec, give me choice get weird results
                    double rank=(double)el.score/((units_old/11)+1);//yeah so instead of storing my rank i am storing score and time so that whenever it comes in we can check rank for that time hence the decay is real
                    if(rank>max_rank){
                        max_rank=rank;
                        strcpy(match,el.path);
                        matched=true;
                    }
                }
            }
        } 
    }
    fclose(file);
    return matched;
}
void update(char* path,char*home){
    char filepath[PATH_MAX];
    snprintf(filepath,PATH_MAX,"%s/frerency",home);
    element eldb[100];
    int count=0;
    double min_rank=DBL_MAX;
    int min=0;
    time_t ct=time(NULL);
    FILE *file=fopen(filepath,"r");
    if(file!=NULL){
        char line[PATH_MAX+100];
        while(fgets(line,sizeof(line),file)&&count<100){
            if(sscanf(line,"{p:\"%[^\"]\",s:\"%d\",t:\"%ld\"}",eldb[count].path,&eldb[count].score,&eldb[count].stamp)==3){
                double units_old=difftime(ct,eldb[count].stamp)/12813.0;
                double rank=(double)eldb[count].score/((units_old)/11)+1;
                if(rank<min_rank){
                    min_rank=rank;
                    min=count;
                }
                count++;
            }
        }
        fclose(file);
    }
    int found=-1;
    for(int i=0;i<count;i++){
        if(strcmp(eldb[i].path,path)==0){
            found=i;
            break;
        }
    }
    if(found!=-1){
        eldb[found].score+=1;//freqquency update
        eldb[found].stamp=ct;//recency update
    }
    else if(count<100){
        strcpy(eldb[count].path,path);
        eldb[count].score=1;
        eldb[count].stamp=ct;
        count++;
    }
    else{
        strcpy(eldb[min].path,path);
        eldb[min].score=1;
        eldb[min].stamp=ct;
    }
    file=fopen(filepath,"w");
    if(file){
        for(int i=0;i<count;i++){
            fprintf(file,"{p:\"%s\",s:\"%d\",t:\"%ld\"}\n",eldb[i].path,eldb[i].score,(long)eldb[i].stamp);
        }
        fclose(file);
    }
}
void hop(char**args,int len,bool *changed,char* cwd,char* prev,char* home){
    if(len==1){
        char* temp=malloc(PATH_MAX*sizeof(char));
        getcwd(temp,PATH_MAX);
        chdir(home);
        *changed=true;
        if(strcmp(prev,temp)!=0)
        strcpy(prev,temp);
        update(home,home);
        free(temp);
        return;
    }
    for(int i=1;i<len;i++){
        char *ccwd;
        ccwd=calloc(PATH_MAX,sizeof(char));
        getcwd(ccwd,PATH_MAX);
        char*tar;
        tar=calloc(PATH_MAX,sizeof(char));
        bool hopped=false;
        struct stat path;
        if(strcmp(args[i],"~")==0){
            strcpy(tar,home);
            hopped=true;
        }
        else if(strcmp(args[i],"-")==0){
            if(strcmp(prev,"")==0){
                continue;
            }
            strcpy(tar,prev);
            hopped=true;
        }
        else if(strcmp(args[i],".")==0){
            continue;
        }
        else if(strcmp(args[i],"..")==0){
            if(strcmp(ccwd,"/")==0)
            continue;
            strcpy(tar,"..");
            hopped=true;
        }
        else{
            if(stat(args[i], &path)==0 && S_ISDIR(path.st_mode)){
                strcpy(tar,args[i]);
                hopped=true;
            }
            else{
                if(search(home,args[i],tar))
                hopped=true;
                else
                printf("hop: no such directory");
            }
        }
        if(hopped){
            if(chdir(tar)==0){
                *changed=true;
                if(strcmp(prev,ccwd)!=0)
                strcpy(prev,ccwd);
                char ncwd[PATH_MAX];
                getcwd(ncwd,PATH_MAX);
                update(ncwd,home);
            }
        }
        free(ccwd);
        free(tar);
    }
}
void reveal(char **args,int len,char* res){
    bool hidden=false;
    bool recurse=false;
    if(len==2){ //readdir does not give a sorted array and too  much work to sort it myself so the convinient function scandir idk why the documentation did not have this prior prolly due to  alphabetical order full block reqwritten
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
void locate(char** filename,char* res,int k){// i did  not expect to get punched with an OOM today, also reading the most recent doubt i realised i might have some reading comprehension issues and asked AI for help here as well gonna overhaul the full implemenatation     
    char *paths=getenv("PATH");
    char* cwd;
    cwd=calloc(PATH_MAX,sizeof(char)); //i truly need to start reading the quuestions 
    getcwd(cwd,PATH_MAX);
    if(paths==NULL){
        printf("cshell: PATH environment variable not set\n");
        return;
    }
    char*temp=calloc(strlen(paths)+2+strlen(cwd),sizeof(char));
    snprintf(temp,strlen(paths)+2+strlen(cwd),"%s:%s",cwd,paths);
    for(int i=0;i<k;i++){
        bool exist=false;
        char* pointer=temp;
        char* end;
        while(*pointer){
            end=strchr(pointer,':');
            if(end==NULL){
                end=pointer+strlen(pointer);
            }
            int len=end-pointer;
            if(len>0){
                char *target;
                target=calloc(PATH_MAX,sizeof(char));
                snprintf(target,PATH_MAX,"%.*s/%s",len,pointer,filename[i]);
                //printf("target: %s\n",target); //for debugging// apparently i need to use %.*s because
                struct stat path_stat;
                if(stat(target, &path_stat)==0 && S_ISREG(path_stat.st_mode) && access(target, X_OK) == 0){ //saw that the file needs to be executable got his boiler plate code so access is a function which takes the string and checks if the file is executable or not, also stat with S_ISREG is a macro to check if the file is a regular file or not
                    char dup[PATH_MAX+2]; //i just saw my own systems env variable has the same path twice(i was hosting some site) so i am running a basic dup check on itt
                    snprintf(dup,PATH_MAX+2,"%s\n",target);
                    if(strstr(res,dup)==NULL){
                        strcat(res,target);
                        strcat(res,"\n");
                        exist=true;
                    }
                }
                free(target);
            }
            pointer=end;
            if(*pointer==':'){
                pointer++;
            }
        }
        if(!exist){
            strcat(res,"locate: command not found (");
            strcat(res,filename[i]);
            strcat(res,")\n");
        }
    }
    free(temp);
}
void peek(char**args,int len){
    int i=1;
    bool rev=false;
    bool num=false;
    while(i<len && args[i][0]=='-'){
        size_t j=1;
        while(j<strlen(args[i])){
            if(args[i][j]=='r'){
                rev=true;
            }
            else if(args[i][j]=='n'){
                num=true;
            }
            else{
                printf("peek: invalid option %c\n",args[i][j]);
                return;
            }
            j++;
        }
        i++;
    }
    if(i==len){
        if(strcmp(args[i-1],"-")!=0){
        printf("peek: invalid syntax\n");
        return;   
        }
        i--;
    }
    if(!rev){
        while(i<len){
            int k=0;
            FILE *file;
            if(strcmp(args[i],"-")==0){
                file=stdin; //since this is what the question asks for
            }
            else{    
                struct stat is_dir;
                if(stat(args[i], &is_dir) == 0 && S_ISDIR(is_dir.st_mode)){
                    printf("peek: is a directory\n");
                    i++;
                    continue;
                }
                file=fopen(args[i],"r");
                if(file==NULL){
                    printf("peek: no such file or directory\n");
                    i++;
                    continue;
                }
            }
            char *line=NULL; //posix read file boiler plate
            size_t length=0;
            ssize_t read;
            read=getline(&line,&length,file);
            while(read!=-1){
                if(strcmp(line,"\n")==0){ //doubted currently maybe it should not run at all or it should yell then continue currently have the yell then continue setup made
                    printf("\n");
                }
                else{
                    if(read>0 && line[read-1]=='\n'){
                        line[read-1]='\0';
                    }
                    if(num)
                    printf("%d %s\n",++k,line);
                    else
                    printf("%s\n",line);
                }
                read=getline(&line,&length,file);
            }
            free(line);
            if(file!=stdin)
            fclose(file);
            i++;
        }
    }
    else{
        while(i<len){
            int k=0;
            struct stat is_dir;
            if(strcmp(args[i],"-")==0){
                printf("peek: invalid syntax\n");
                return;
            }
            if(stat(args[i], &is_dir) == 0 && S_ISDIR(is_dir.st_mode)){ //directory check is first since opening a directory also gives null, then the errors output don't need to have one saying not file or dir idk
                printf("peek: is a directory\n");
                i++;
                continue;;
            }
            // FILE *file=fopen(args[i],"r");
            int fd=open(args[i],O_RDONLY);//lseek uses the direct file descriptor and not the file itself
            if(fd<0){
                printf("peek: no such file or directory\n"); //this is changed boiler plate to handle the case of continue with a shout out
                i++;
                continue;
            }
            off_t size=lseek(fd,0,SEEK_END); //lseek returns the offset of the file descriptor and SEEK_END is a macro to seek to the end of the file
            if(size==0){
                close(fd);
                i++;
                continue;
            }
            if(num){// hey i can either go through the whole thing in reverse find value of k then do k-- or i can just copy paste from above and change 2 lines guess what i did
                FILE *file=fopen(args[i],"r");
                char *line=NULL;
                size_t length=0;
                ssize_t read;
                read=getline(&line,&length,file);
                while(read!=-1){
                    if((strcmp(line,"\n")!=0)&&strlen(line)>0)
                    k++;
                    read=getline(&line,&length,file);
                }
                free(line);
                fclose(file);
            }
            off_t pos=size;
            int chunk=2048;
            char *rest=NULL;
            int numrest=0;
            while(pos>0){
                int reade;
                if(pos>chunk)
                reade=chunk;
                else
                reade=(int)pos;
                pos-=reade;
                lseek(fd,pos,SEEK_SET);
                char *buffer=malloc(reade+1+numrest);
                read(fd,buffer,reade);
                if(numrest>0){// this is essentially copying back something left in last slice since we read chunk by chunk in lseek like the final x in one chunk like that
                    memcpy(buffer+reade,rest,numrest);
                    free(rest);
                }
                buffer[reade+numrest]='\0';
                int prev=reade+numrest-1;
                if(pos+reade==size && buffer[prev]=='\n'){
                    prev--;
                }
                for(int j=prev;j>=0;j--){ //adjusting for the question
                    if(buffer[j]=='\n'){
                        int str=prev-j;
                        char *line=malloc(str+1);
                        strncpy(line,buffer+j+1,str);
                        line[str]='\0';
                        if(str==0)
                        printf("\n");
                        else{
                            if(num)
                            printf("%d %s\n",k--,line);
                            else
                            printf("%s\n",line);
                        }
                        free(line);
                        prev=j-1;
                    }
                }
                numrest=prev+1;
                if(numrest>0){
                    rest=malloc(numrest);
                    memcpy(rest,buffer,numrest);
                }
                else{
                    rest=NULL;
                }
                free(buffer);
            }
            if(numrest>0){
                char*line=malloc(numrest+1);
                strncpy(line,rest,numrest);
                line[numrest]='\0';
                if(num)
                printf("%d %s\n",k--,line);
                else
                printf("%s\n",line);
                free(line);
                free(rest);
            }
            close(fd);
            i++;
        }
    }
}
so this will work for hop and my way of score and decay are : Scoring, decay, and storage are up to you, but must be deterministic.
```
![alt text](image-11.png)
![alt text](image-12.png)
![alt text](image-13.png)