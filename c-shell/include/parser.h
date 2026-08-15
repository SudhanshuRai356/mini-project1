#include "lexer.h"
typedef struct node
{
    int piped; // 1 if it is on left of a |
    int background; // 1 if it is on left of a &
    int redir; // if 1 there is somee redirection either < or > of some form which we can check next
    int redir_type; // 1 if <, 2 if >, 3 if >>, 4
    char* output_file; //only used if redir_type is 2 or 3
    char* input_file; //only used if redir_type is 1
    char* cmd; // the command to be executed
    char** args; // the arguments to be passed to the command
    int semi; //1 if on left of a ; this means ki next command is there and we need to execute it independent to this
    int arg_index; // i realised i will haave to play with realloc otherwise and i am already at the limits of my dynamic memory knowledge
}node;
int parser(token*args,int len,node**coms,int*len1);