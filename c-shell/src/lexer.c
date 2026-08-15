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
        args[args_index].value = malloc(100 * sizeof(char));
        if(cmd[i]=='\\'){ //escape
            i++;
            if(i>=len){
                printf("cshell: invalid syntax\n");
                return -1;
            }
            args[args_index].value[0] = cmd[i];
            args[args_index].value[1] = '\0';
            args[args_index].type = 2;
            args_index++;
            
        }
        else if(cmd[i]=='|'){ //soecuak  1/5
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
        else if(cmd[i]=='"'){ //qquote 1
            int k=0;
            i++;
            while(i<len){
                if(cmd[i]=='"'){
                    if(cmd[i-1]!='\\') //end quote might  be afteer an escape 
                    break;
                }
                args[args_index].value[k++]=cmd[i++];
            }
            if(cmd[i]!='"'){
                printf("cshell: invalid syntax\n");
                return -1;
            }
            args[args_index].value[k]='\0';
            args[args_index].type = 1;
            args_index++; 
        }
        else if(cmd[i]=='\''){ //quuote 2
            int k=0;
            i++;
            while(i<len){
                if(cmd[i]=='\''){
                    if(cmd[i-1]!='\\')
                    break;
                }
                args[args_index].value[k++]=cmd[i++];
            }
            if(cmd[i]!='\''){
                printf("cshell: invalid syntax\n");
                return -1;
            }
            args[args_index].value[k]='\0';
            args[args_index].type = 1;
            args_index++;
        }
        else{
            int k=0;
            while(i<len){
                if(cmd[i]==' ' || cmd[i]=='\t' || cmd[i]=='\n' || cmd[i]=='\r' || cmd[i]=='|' || cmd[i]=='&' || cmd[i]=='<' || cmd[i]=='>' || cmd[i]==';'){
                    break;
                }
                if(cmd[i]=='\\'){ //escape
                    i++;
                    if(i>=len){
                        printf("cshell: invalid syntax\n");
                        return -1;
                    }
                    continue;
                }
                args[args_index].value[k++]=cmd[i++];
            }
            args[args_index].value[k]='\0';
            args[args_index].type = 4;
            args_index++;
            i--;
        }
    }
    *len1=args_index;
    return 0;
}