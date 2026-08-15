#include <stdio.h>
#include <stdlib.h>

void lexer(char* cmd, char** special, char** quoted, char** escaped, char** ordinary,char **args,int *len1) {
    int len = strlen(cmd);
    int special_index = 0;
    int quoted_index = 0;
    int escaped_index = 0;
    int ordinary_index = 0;
    int args_index = 0;
    for(int i=0;i<len;i++){
        char *arg = malloc(100*sizeof(char));
        while((cmd[i]==' ' || cmd[i]=='\t' || cmd[i]=='\n' || cmd[i]=='\r')&& i<len){ //ignore spaces
            i++;
        }
        if(i>=len){
            break;
        }
        if(cmd[i]=='\\'){ //escape
            i++;
            arg[0]=cmd[i];
            arg[1]='\0';
            escaped[escaped_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='|'){ //soecuak  1/5
            arg[0]=cmd[i];
            arg[1]='\0';
            special[special_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='&'){//special 2/5
            arg[0]=cmd[i];
            arg[1]='\0';
            special[special_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='<'){// special 3/5
            arg[0]=cmd[i];
            arg[1]='\0';
            special[special_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='>'){// special 4/5
            int k=0;
            arg[k++]=cmd[i];
            if(i<len && cmd[i+1]=='>'){
                arg[k++]=cmd[i+1];
                i++;
            }
            arg[k]='\0';
            special[special_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]==';'){// special 5/5
            arg[0]=cmd[i];
            arg[1]='\0';
            special[special_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='"'){ //qquote 1
            int k=0;
            while(i<len){
                if(cmd[i]=='"'){
                    if(cmd[i-1]!='\\') //end quote might  be afteer an escape 
                    break;
                }
                arg[k++]=cmd[i++];
            }
            arg[k++]=cmd[i]; 
            arg[k]='\0';
            quoted[quoted_index++]=arg;
            args[args_index++]=arg;
             
        }
        else if(cmd[i]=='\''){ //quuote 2
            int k=0;
            while(i<len){
                if(cmd[i]=='\''){
                    if(cmd[i-1]!='\\')
                    break;
                }
                arg[k++]=cmd[i++];
            }
            arg[k++]=cmd[i];
            arg[k]='\0';
            quoted[quoted_index++]=arg;
            args[args_index++]=arg;
             
        }
        else{
            int k=0;
            while(i<len){
                if(cmd[i]==' ' || cmd[i]=='\t' || cmd[i]=='\n' || cmd[i]=='\r' || cmd[i]=='|' || cmd[i]=='&' || cmd[i]=='<' || cmd[i]=='>' || cmd[i]==';'){
                    break;
                }
                arg[k++]=cmd[i++];
            }
            arg[k]='\0';
            ordinary[ordinary_index++]=arg;
            args[args_index++]=arg;
            i--;
        }
    }
    *len1=args_index;
}