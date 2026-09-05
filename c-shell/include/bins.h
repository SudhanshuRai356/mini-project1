#include<time.h>
#include <limits.h>
void reveal(char **args,int len);
void hop(char**args,int len,bool *changed,char* cwd,char* prev,char* home);
void locate(char** filename,char* res,int k);
void peek(char**args,int len);
void activities();
typedef struct element{
    char path[PATH_MAX];
    int  score;
    time_t stamp;
}element;