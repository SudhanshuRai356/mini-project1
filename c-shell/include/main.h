//just to pass the shell_pgid to command i have to make this
extern pid_t shell_pgid;
typedef struct bg{
    pid_t pid;
    int job;
    char*cmd;
    pid_t pgid;
    bool sus;
    pid_t* members;
    int num; //this ds is getting out of handed
    char** name;
    char** full_cmd;
}bg;
extern bg* bg_list;
extern int bg_size;
extern void no_longer_waiting(void);
void assign_stopped(pid_t pid,char* cmd,char **args);
void assign_bg_multi(pid_t* pids,int num,char** names,char* full_cmd,char** args);
void assign_stopped_multi(pid_t* pids,char** cmds,int num,char* full_cmd,char** args);