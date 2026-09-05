#include "parser.h"
#include "main.h"
extern pid_t shell_pgid;
bool builtin(char* cmd);
void exec_builtin(node* com, char* home, char* prev);
void run_cmd(char* cmd,char** args,int arg_index);
int redir_in(char**in,int num);
int redir_out(char**out,bool *append,int num);
int ext_in(char **args,int *args_index,char** in);
int ext_out(char **args,int *args_index,char** out,bool* append);
void piped(node** coms,int start,int end,char*home,char* prev);