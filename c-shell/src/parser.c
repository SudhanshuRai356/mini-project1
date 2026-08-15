#include<stdio.h>
#include<stdlib.h>
char* stack[500];
top=-1;
void push(char* str){
    stack[++top]=str;
}
void pop(){
    if(top==-1){
        return;
    }
    top--;
}
void parser(char** special, char** quoted, char** escaped, char** ordinary,char **args,char **postfix,int len){
    int postfix_index=0;
    for(int i=0;i<len;i++){
        
    }
}