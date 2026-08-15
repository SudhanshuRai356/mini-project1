typedef struct token{
    char* value;
    int type; 
    // les just say 0 is special 1 is quotted, 2 is escape, 4 is ordinary
}token;
void lexer(char*cmd,token*args,int*len1);