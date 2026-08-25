#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
int lexer(char* cmd,token *args,int *len1) {
    int len = strlen(cmd);
    int args_index = 0;
    for(int i=0;i<len;i++){
        while((cmd[i]==' ' || cmd[i]=='\t' || cmd[i]=='\n' || cmd[i]=='\r')&& i<len){ //ignore spaces
            i++;
        }
        if(i>=len){
            break;
        }
        args[args_index].value = malloc((len-i+1) * sizeof(char));
        if(cmd[i]=='|'){ //soecuak  1/5
            args[args_index].value[0] = cmd[i];
            args[args_index].value[1] = '\0';
            args[args_index].type = 0;
            args_index++;
             
        }
        else if(cmd[i]=='&'){//special 2/5
            args[args_index].value[0] = cmd[i];
            args[args_index].value[1] = '\0';
            args[args_index].type = 0;
            args_index++;
        }
        else if(cmd[i]=='<'){// special 3/5
            args[args_index].value[0] = cmd[i];
            args[args_index].value[1] = '\0';
            args[args_index].type = 0;
            args_index++;
             
        }
        else if(cmd[i]=='>'){// special 4/5
            int k=0;
            args[args_index].value[k++] = cmd[i];
            if(i<len && cmd[i+1]=='>'){
                args[args_index].value[k++] = cmd[i+1];
                i++;
            }
            args[args_index].value[k]='\0';
            args[args_index].type = 0;
            args_index++;
             
        }
        else if(cmd[i]==';'){// special 5/5
            
            args[args_index].value[0] = cmd[i];
            args[args_index].value[1] = '\0';
            args[args_index].type = 0;
            args_index++;
             
        }
        else{ //just read a doubt on the doubt doc and realised i just read this entirely wrong and now i have to fix both the lexer and parser...
            int k=0;
            args[args_index].type = 4;
            while(i<len && cmd[i]!=' ' && cmd[i]!='\t' && cmd[i]!='\n' && cmd[i]!='\r' && cmd[i]!='|' && cmd[i]!='&' &&cmd[i]!='<'&&cmd[i]!='>'&&cmd[i]!=';'){
                if(cmd[i]=='\\'){
                    i++;
                    if(i>=len){
                        // printf("cshell: invalid syntax\n");
                        return -1;
                    }
                    args[args_index].value[k++]=cmd[i++];
                }
                else if (cmd[i]=='"'){
                    i++;
                    while(i<len && cmd[i]!='"'){
                        if(cmd[i]=='\\'){
                            i++;
                            if(i>=len){
                                // printf("cshell: invalid syntax\n");
                                return -1;
                            }
                            if(cmd[i]=='"'||cmd[i]=='\\'){
                                args[args_index].value[k++]=cmd[i++];
                            }
                            else{
                                args[args_index].value[k++]='\\'; //i don't wanna talk about it
                                args[args_index].value[k++]=cmd[i++];
                            }
                        }
                        else{
                            args[args_index].value[k++]=cmd[i++];
                        }
                    }
                    if (i>=len || cmd[i]!='"'){
                        // printf("cshell: invalid syntax\n");
                        return -1;
                    }
                    i++;
                }
                else if(cmd[i]=='\''){
                    i++;
                    while(i<len && cmd[i]!='\''){
                        args[args_index].value[k++]=cmd[i++];
                    }
                    if(i>=len || cmd[i]!='\''){
                        // printf("cshell: invalid syntax\n");
                        return -1;
                    }
                    i++;
                }
                else{
                    args[args_index].value[k++]=cmd[i++];
                }
            }
            args[args_index].value[k]='\0';
            args_index++;
        }
    }
    *len1=args_index;
    return 0;
}