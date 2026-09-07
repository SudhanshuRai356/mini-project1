#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h> // i guess even if the directory is posix it does not need to have everything its non posix variantttt had so retarded
#include<sys/stat.h> //stackoverflow recommended swap for d_type
#include<stdbool.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h>
#include <limits.h>
#include<fcntl.h>
#include<float.h>
#include<time.h>
#include "bins.h"
#include "main.h"
char *list;
extern pid_t shell_pgid;
bool search(char*home,char*path,char*match){
    char filepath[PATH_MAX];
    snprintf(filepath,PATH_MAX,"%s/frerency",home);
    FILE *file=fopen(filepath,"r");
    if(file==NULL){
        return false;
    }
    double max_rank=-1.0;
    double max_rank_bk=-1.0;
    char line[PATH_MAX+100];
    bool matched=false;
    bool matched_bk=false;
    char matched_path_bk[PATH_MAX];
    char matched_path[PATH_MAX];
    time_t ct=time(NULL);
    while(fgets(line,sizeof(line),file)){
        element el;
        if(sscanf(line,"{p:\"%[^\"]\",s:\"%d\",t:\"%ld\"}",el.path,&el.score,&el.stamp)==3){
            if(strstr(el.path,path)==NULL)
            continue;
            struct stat st;
            if(stat(el.path, &st) != 0||!S_ISDIR(st.st_mode)){
                continue;
            }
            double units_old=difftime(ct,el.stamp)/12813.0;
            double rank=(double)el.score/((units_old/11)+1);
            char* base=strrchr(el.path,'/');
            if(base==NULL){
                base=el.path;
            }
            else{
                base++;
            }
            if(strstr(base,path)!=NULL){
                if(rank>max_rank){
                    max_rank=rank; 
                    strcpy(matched_path,el.path); 
                    matched=true;
                }
            }
            if(rank>max_rank_bk){
                max_rank_bk=rank; 
                strcpy(matched_path_bk,el.path); 
                matched_bk=true;
            }
        } 
    }
    fclose(file);
    if(matched){
        strcpy(match,matched_path);
        return true;
    }
    else if(matched_bk){
        strcpy(match,matched_path_bk);
        return true;
    }
    else{
        return false;
    }
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
                double rank=(double)eldb[count].score/((units_old/11)+1);
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
                printf("hop: no such directory\n");
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
void reveal(char **args,int len){ //kept having issues when running reveal -t / so i just decided to do away with the buffer all together
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
            if((!hidden && list[k]->d_name[0]=='.')||(strcmp(list[k]->d_name,".")==0)||(strcmp(list[k]->d_name,"..")==0)){// i know there is no way fir hidden to be true but i did copied fromn below and now i realised instead of ranting i could have removed it,eh
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
                char* segment=path;
                while(true){
                    char* sl=strchr(segment,'/');
                    int seglen;
                    if(sl==NULL){
                        seglen=strlen(segment);
                    }
                    else{
                        seglen=sl-segment;
                    }
                    if(memchr(segment,' ',seglen)!=NULL){
                        printf("'%.*s'",seglen,segment);
                    }
                    else{
                        printf("%.*s",seglen,segment);
                    }
                    if(sl==NULL){
                        break;
                    }
                    else{
                        printf("/");
                        segment=sl+1;
                    }
                }
                if(dir && recurse)
                printf("/");
                printf("\n");
            }
            else{
                if(strchr(list[k]->d_name,' ')!=NULL){
                    printf("\'%s\' ",list[k]->d_name);
                }
                else{
                    printf("%s ",list[k]->d_name);
                }
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
                reveal(new_args, 3);
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
            if((!hidden && list[k]->d_name[0]=='.')||(strcmp(list[k]->d_name,".")==0)||(strcmp(list[k]->d_name,"..")==0)){
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
                char* segment=path;
                while(true){
                    char* sl=strchr(segment,'/');
                    int seglen;
                    if(sl==NULL){
                        seglen=strlen(segment);
                    }
                    else{
                        seglen=sl-segment;
                    }
                    if(memchr(segment,' ',seglen)!=NULL){
                        printf("'%.*s'",seglen,segment);
                    }
                    else{
                        printf("%.*s",seglen,segment);
                    }
                    if(sl==NULL){
                        break;
                    }
                    else{
                        printf("/");
                        segment=sl+1;
                    }
                }
                if(dir && recurse)
                printf("/");
                printf("\n");
            }
            else{
                char* segment=entry->d_name;
                while(true){
                    char* sl=strchr(segment,'/');
                    int seglen;
                    if(sl==NULL){
                        seglen=strlen(segment);
                    }
                    else{
                        seglen=sl-segment;
                    }
                    if(memchr(segment,' ',seglen)!=NULL){
                        printf("'%.*s''",seglen,segment);
                    }
                    else{
                        printf("%.*s",seglen,segment);
                    }
                    if(sl==NULL){
                        break;
                    }
                    else{
                        printf("/");
                        segment=sl+1;
                    }
                }
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
                reveal(new_args, len);
            }
            free(path);
            free(list[k]);
        }
        free(list);
    }
    if(!recurse && !hidden){
        printf("\n");
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
    while(i<len && args[i][0]=='-' && strlen(args[i])>1){
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
            char **lines=NULL; //posix read file boiler plate
            char *linebuf=NULL;
            int lcount=0, lcap=0;
            size_t length=0;
            ssize_t r;
            while((r=getline(&linebuf,&length,file))!=-1){
                if(lcount==lcap){
                    lcap = lcap ? lcap*2 : 16;
                    lines=realloc(lines, lcap*sizeof(char*));
                }
                if(r>0 && linebuf[r-1]=='\n'){
                    linebuf[r-1]='\0';
                }
                lines[lcount++]=strdup(linebuf);
            }
            free(linebuf);
            if(file!=stdin)
            fclose(file);
            else
            clearerr(stdin); // terminal just quitting for some reason this should help
            for(int m=0;m<lcount;m++){
                if(strcmp(lines[m],"")==0){
                    printf("\n");
                }
                else{
                    if(num)
                    printf("%d %s\n",++k,lines[m]);
                    else
                    printf("%s\n",lines[m]);
                }
                free(lines[m]);
            }
            free(lines);
            i++;
        }
    }
    else{
        while(i<len){
            int k=0;
            struct stat is_dir;
                        if(strcmp(args[i],"-")==0){
                char **lines=NULL;
                int lcount=0, lcap=0;
                char *linebuf=NULL;
                size_t length=0;
                ssize_t r;
                while((r=getline(&linebuf,&length,stdin))!=-1){
                    if(lcount==lcap){
                        lcap = lcap ? lcap*2 : 16;
                        lines=realloc(lines, lcap*sizeof(char*));
                    }
                    if(r>0 && linebuf[r-1]=='\n'){
                        linebuf[r-1]='\0';
                    }
                    lines[lcount++]=strdup(linebuf);
                }
                clearerr(stdin); // same hopes of fixing exit shell issues
                free(linebuf);
                int total=0;
                for(int m=0;m<lcount;m++)
                    if(strlen(lines[m])>0) total++;
                int kk=total;
                for(int m=lcount-1;m>=0;m--){
                    if(strcmp(lines[m],"")==0){
                        printf("\n");
                    }
                    else{
                        if(num)
                        printf("%d %s\n",kk--,lines[m]);
                        else
                        printf("%s\n",lines[m]);
                    }
                    free(lines[m]);
                }
                free(lines);
                i++;
                continue;
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
void activities(){
    no_longer_waiting();
    for(int i=0;i<bg_size;i++){
        printf("[%d] pgid %d\n",bg_list[i].job,bg_list[i].pgid);
        for(int m=0;m<bg_list[i].num;m++){
            if(bg_list[i].sus)
            printf("  %d %s  Stopped\n",bg_list[i].members[m],bg_list[i].name[m]);
            else
            printf("  %d %s  Running\n",bg_list[i].members[m],bg_list[i].name[m]);
        }
    }
}
void resume(char** args, int arg_index){
    if(arg_index<3|| args[1][0]!='%'){
        printf("resume: invalid syntax\n");
        return;
    }
    char* end;
    long long job_num=strtol(args[1]+1,&end,10);
    if(*end!='\0'){
        printf("resume: invalid syntax\n");
        return;
    }
    int job=-1;
    for(int i=0;i<bg_size;i++){
        if(bg_list[i].job==job_num){
            job=i;
            break;
        }
    }
    if(job==-1){
        printf("resume: no such job\n");
        return;
    }
    if(strcmp(args[2],"bg")==0){
        if(arg_index!=3){
            printf("resume: invalid syntax\n");
            return;
        }
        killpg(bg_list[job].pgid,SIGCONT); // i just wanna say the function name kinda ironic made me read unix history
        bg_list[job].sus=false;
        printf("[%d] + Running\t",bg_list[job].job);
        for(int i=0;bg_list[job].full_cmd[i]!=NULL;i++){
            printf("%s ",bg_list[job].full_cmd[i]);
        }
        printf("\n");
        return;
    }
    else if(strcmp(args[2],"fg")==0){
        if(arg_index>3){
            if(arg_index!=5){
                printf("resume: invalid syntax\n");
                return;
            }
            if(strcmp(args[3],"--timeout")!=0){
                printf("resume: invalid syntax\n");
                return;
            }
            char* num_end;
            long long timeout=(int)strtol(args[4],&num_end,10);
            if(*num_end!='\0'||timeout<=0){
                printf("resume: invalid syntax\n");
                return;
            }
            for(int i=0;bg_list[job].full_cmd[i]!=NULL;i++){
                printf("%s ",bg_list[job].full_cmd[i]);
            }
            printf("\n");
            pid_t pgid=bg_list[job].pgid;
            int num=bg_list[job].num;
            pid_t members[num];
            memcpy(members,bg_list[job].members,num*sizeof(pid_t));
            bg_list[job].sus=false;
            killpg(pgid,SIGCONT);
            tcsetpgrp(STDIN_FILENO,pgid);
            alarm((unsigned int)timeout); //welp alarm does not take longlong and i am not changing the above long long
            bool timed=false;
            bool stopped=false;
            for(int m=0;m<num;m++){
                int status;
                pid_t r=waitpid(members[m],&status,WUNTRACED);
                if(r<0 && errno==EINTR){ 
                    timed=true; 
                    break; 
                }
                if(WIFSTOPPED(status)) 
                stopped=true;
            }
            alarm(0); //needed other wise even with an interrupt it just rings an alarm out of nowhere in some other process
            if(timed){
                killpg(pgid,SIGTERM);
                printf("resume: job timed out\n");
            }
            tcsetpgrp(STDIN_FILENO,shell_pgid);
            if(!timed){
                if(stopped){
                    bg_list[job].sus=true;
                }
                else{
                    free(bg_list[job].members);
                    free(bg_list[job].name);
                    memmove(&bg_list[job],&bg_list[job+1],(bg_size-job-1)*sizeof(bg));
                    bg_size--;
                    bg_list=realloc(bg_list,bg_size*sizeof(bg));
                }
            }
        }
        else{
            for(int i=0;bg_list[job].full_cmd[i]!=NULL;i++){
                printf("%s ",bg_list[job].full_cmd[i]);
            }
            printf("\n");
            pid_t pgid=bg_list[job].pgid;
            int num=bg_list[job].num;
            pid_t members[num];
            memcpy(members,bg_list[job].members,num*sizeof(pid_t));
            bg_list[job].sus=false;
            killpg(pgid,SIGCONT);
            tcsetpgrp(STDIN_FILENO,pgid);
            bool stopped=false;
            for(int m=0;m<num;m++){
                int status;
                waitpid(members[m],&status,WUNTRACED);
                if(WIFSTOPPED(status)) 
                stopped=true;
            }
            tcsetpgrp(STDIN_FILENO,shell_pgid);
            if(stopped){
                bg_list[job].sus=true;
            }
            else{
                free(bg_list[job].members);
                free(bg_list[job].name);
                memmove(&bg_list[job],&bg_list[job+1],(bg_size-job-1)*sizeof(bg));
                bg_size--;
                bg_list=realloc(bg_list,bg_size*sizeof(bg));
            }
        }
    }
    else{
        printf("resume: invalid syntax\n");
    }
}