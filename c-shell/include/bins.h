#include<time.h>
#include <limits.h>
#include "main.h"
extern pid_t shell_pgid;
void reveal(char **args,int len);
void hop(char**args,int len,bool *changed,char* cwd,char* prev,char* home);
void locate(char** filename,char* res,int k);
void peek(char**args,int len);
void activities();
void resume(char** args, int arg_index);
void ping(char** args,int arg_index);
void spy(char** args,int arg_index);
void snoop(char** args,int arg_index);
typedef struct element{
    char path[PATH_MAX];
    int  score;
    time_t stamp;
}element;