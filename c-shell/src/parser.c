#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
/*LINE  ->  ε
      |   WORD ARG

ARG   ->  ε
      |   WORD    ARG
      |   OP_LT   TGT
      |   OP_GT   TGT
      |   OP_GTGT TGT
      |   OP_PIPE CMD
      |   OP_SEMI CMD
      |   OP_AMP  BG

CMD   ->  WORD ARG

TGT   ->  WORD ARG

BG    ->  ε
      |   WORD ARG
because who wants to be opening the question page again and again
*/
int parser(token*args, int len, node**coms, int*len1){
    int coms_index=0;
    coms[coms_index]=malloc(sizeof(node));
    coms[coms_index]->piped=0;
    coms[coms_index]->background=0;
    coms[coms_index]->redir=0;
    coms[coms_index]->redir_type=0;
    coms[coms_index]->output_file=NULL;
    coms[coms_index]->input_file=NULL;
    coms[coms_index]->semi=0;
    coms[coms_index]->cmd=NULL;
    coms[coms_index]->args=malloc(100*sizeof(char*));
    coms[coms_index]->arg_index=0;
    for(int i=0;i<len;i++){
        if(args[i].type==0){
            if(strcmp(args[i].value,"|")==0){ // OP_PIPE CMD for this rule so next has to be new command 
                coms[coms_index]->piped=1;
                coms_index++;
                if(i+1>=len || args[i+1].type==0 || coms[coms_index-1]->cmd==NULL){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]=malloc(sizeof(node));
                coms[coms_index]->piped=0;
                coms[coms_index]->background=0;
                coms[coms_index]->redir=0;
                coms[coms_index]->redir_type=0;
                coms[coms_index]->output_file=NULL;
                coms[coms_index]->input_file=NULL;
                coms[coms_index]->semi=0;
                coms[coms_index]->cmd=NULL;
                coms[coms_index]->args=malloc(100*sizeof(char*));
                coms[coms_index]->arg_index=0;
            }
            else if(strcmp(args[i].value,";")==0){ // OP_SEMI CMD for this rule so next has to be new command 
                coms[coms_index]->semi=1;
                coms_index++;
                if(i+1>=len || args[i+1].type==0 || coms[coms_index-1]->cmd==NULL){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]=malloc(sizeof(node));
                coms[coms_index]->piped=0;
                coms[coms_index]->background=0;
                coms[coms_index]->redir=0;
                coms[coms_index]->redir_type=0;
                coms[coms_index]->output_file=NULL;
                coms[coms_index]->input_file=NULL;
                coms[coms_index]->semi=0;
                coms[coms_index]->cmd=NULL;
                coms[coms_index]->args=malloc(100*sizeof(char*));
                coms[coms_index]->arg_index=0;
            }
            else if(strcmp(args[i].value,"&")==0){// OP_AMP  BG for this rule the next can be a command or nothing or just args
                coms[coms_index]->background=1;
                coms_index++;
                if(coms[coms_index-1]->cmd==NULL){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]=malloc(sizeof(node));
                coms[coms_index]->piped=0;
                coms[coms_index]->background=0;
                coms[coms_index]->redir=0;
                coms[coms_index]->redir_type=0;
                coms[coms_index]->output_file=NULL;
                coms[coms_index]->input_file=NULL;
                coms[coms_index]->semi=0;
                coms[coms_index]->cmd=NULL;
                coms[coms_index]->args=malloc(100*sizeof(char*));
                coms[coms_index]->arg_index=0;
            }
            else if(strcmp(args[i].value,"<")==0){ // OP_LT TGT so there will be a redirect of data and the original ends up becoming the input file
                coms[coms_index]->redir=1;
                coms[coms_index]->redir_type=1;
                if(i+1>=len || args[i+1].type==0){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]->input_file=args[i+1].value;
                i++;
            }
            else if(strcmp(args[i].value,">")==0){ // OP_GT TGT so there will be a redirect of data and the original ends up becoming the output file
                coms[coms_index]->redir=1;
                coms[coms_index]->redir_type=2;
                if(i+1>=len || args[i+1].type==0){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]->output_file=args[i+1].value;
                i++;
            }
            else if(strcmp(args[i].value,">>")==0){ // OP_GTGT TGT so there will be a redirect of data and the original ends up becoming the output file
                coms[coms_index]->redir=1;
                coms[coms_index]->redir_type=3;
                if(i+1>=len || args[i+1].type==0){
                    printf("cshell: invalid syntax\n");
                    return -1;
                }
                coms[coms_index]->output_file=args[i+1].value;
                i++;
            }
        }
        else{ // if word or arg then we just handle it
            if(coms[coms_index]->cmd==NULL)
                coms[coms_index]->cmd=args[i].value;
            coms[coms_index]->args[coms[coms_index]->arg_index++]=args[i].value;
        }
    }
    if(coms[coms_index]->cmd!=NULL){
        coms[coms_index]->args[coms[coms_index]->arg_index]=NULL;
        *len1=coms_index+1;
    }
    else{
        *len1=coms_index;
    }
    return 0;
}