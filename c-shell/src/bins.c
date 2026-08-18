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
    if(paths==NULL){
        printf("cshell: PATH environment variable not set\n");
        return;
    }
    char*temp=calloc(strlen(paths)+3,sizeof(char));
    snprintf(temp,strlen(paths)+3,"%s:.",paths);
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