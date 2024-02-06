#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h>

#define MAX_INPUT 200
#define MAX_CMD_NUM 100

char* cmdlist[MAX_CMD_NUM];
char* cmdargs[10];
char** argv;
void pipeline(char *ptr);
int makeargv(char *s, const char *delimiters, char** argvp, int MAX_LIST);
void error_func(char *str);
void execute_cmd(char* cmdlist1);

void head(char* fileNm);
void head1(char* linenum, char* fileNm);
void tail(char* fileNm);
void tail1(char* linenum, char* fileNm);
void cat(char* file);
void cp(char* file1, char* file2);
void mv(char* file1, char* file2);
void rm(char* file);
void cd(char* dir);
void pwd();
void exit_func();
void print_error(char* cmd);
void parse_redirect(char* cmd);
int redirect_in(char** argv);
int redirect_out(char** argv);
int redirect_app(char** argv);

int cnt;
int fd1, dum;
int redirect_flag;
int redirectnumin, redirectnumout, redirectnumapp;

int main(){
    int pipecheck = 0;
    int i, spid, status;
    int len;
    while(1){
        redirectnumin = 0;
        redirectnumout = 0;
        redirectnumapp = 0;
        write(1,"$",1);
        signal(SIGINT,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGTSTP,SIG_IGN);
        char* input=(char*)malloc(MAX_INPUT);
        read(1,input,MAX_INPUT);
        if(strcmp(input,"\n")==0){
            free(input);
            continue;
        }
        len = strlen(input)-1;
        while(input[len]!='\n'){
            input[len]='\0';
            len--;
        }
        input[len]='\0';
        if(strchr(input,'|') != NULL){
            pipecheck = 1;
        }
        if(pipecheck){
            pipeline(input);
        }
        else{
            execute_cmd(input);
        }
        
        if(redirectnumin==1){
            close(fd1);
            dup2(dum, 0);
            close(dum);
            remove("dummy");
            redirectnumin = 0;
        }
        if(redirectnumout==1 || redirectnumapp==1){
            close(fd1);
            dup2(dum, 1);
            close(dum);
            remove("dummy");
            redirectnumout = 0;
        }
        
        memset(input, '\0', MAX_INPUT);
        free(input);
        pipecheck = 0;
        memset(cmdlist, '\0', sizeof(cmdlist));
        //memset(cmdargs, '\0', sizeof(cmdargs));
        memset(cmdargs, '\0', sizeof(cmdargs));
    }
    
    return 0;
}

void pipeline(char *ptr){
    char *command1, *command2;
    int fd[2];
    int count, i;
    int spid, status;
    //char **cmdlist1;
    
    if(strchr(ptr,'|') != NULL) {
        if((count = makeargv(ptr, "|", cmdlist, MAX_CMD_NUM)) <= 0){
            perror("makeargv error");
        }
        //cmdlist1 = makeargv(ptr);
        //count = cnt;
    }
    pid_t pid;
    
    for(i=0; i<count; i++){
        if(pid == 0) {
            close(fd[0]);
            dup2(fd[1], 1);
            close(fd[1]);
            execute_cmd(cmdlist[i]);
            //spid = wait(&status);
        }
        else {
            close(fd[1]);
            dup2(fd[0], 0);
            close(fd[0]);
            execute_cmd(cmdlist[i]);
            //spid = wait(&status);
        }
    }
    return;
}

int makeargv(char *s, const char *delimiters, char** argvp, int MAX_LIST)
{
    int i = 0;
    int numtokens = 0;
    char *snew = NULL;
    
    if( (s==NULL) || (delimiters==NULL) )
    {
        return -1;
    }
    
    snew = s+strspn(s, delimiters);
    
    argvp[numtokens]=strtok(snew, delimiters);
    
    if( argvp[numtokens] !=NULL)
        for(numtokens=1; (argvp[numtokens]=strtok(NULL, delimiters)) != NULL; numtokens++)
        {
            if(numtokens == (MAX_LIST-1)) return -1;
        }
    
    if( numtokens > MAX_LIST) return -1;
    
    return numtokens;
}

void error_func(char *str){
    perror(str);
    exit(1);
}

void execute_cmd(char* cmdlist1){
    char* ptr;
    char* cmdlist2;
    int n = 0;
    int i, fd;
    cmdlist2=(char*)malloc(sizeof(MAX_INPUT));
    strcpy(cmdlist2,cmdlist1);
    redirect_flag = 0;
    char* command=(char*)malloc(MAX_INPUT);
    argv=(char**)malloc(sizeof(char*)*10);
    for(int i=0; i<10; i++){
        argv[i]=(char*)malloc(MAX_INPUT);
        argv[i]=NULL;
    }
    ptr=strtok(cmdlist1," ");
    strcpy(command,ptr);
    while(ptr!=NULL){
        argv[n++]=ptr;
        ptr=strtok(NULL," ");
    }
    redirectnumin = redirect_in(argv);
    redirectnumout = redirect_out(argv);
    redirectnumapp = redirect_app(argv);
    if(!strcmp(command,"exit") || !strcmp(command," exit") || strstr(command,"exit") != NULL){
        exit_func();
    }
    else if(!strcmp(command,"cd")){
        cd(argv[1]);
    }
    else{
        int pid=fork();
        if(pid==-1){
            perror("fork error");
        }
        if(pid==0){
            if(!strcmp(command,"cat")){
                for(int i=1;i<n;i++){
                    cat(argv[i]);
                }
            }
            else if(!strcmp(command,"cp")){
                cp(argv[1],argv[2]);
            }
            else if(!strcmp(command,"mv")){
                mv(argv[1],argv[2]);
            }
            else if(!strcmp(command,"rm")){
                rm(argv[1]);
            }
            else if(!strcmp(command,"pwd")){
                pwd();
            }
            else if(!strcmp(command,"head")){
                if(strstr(argv[1],"-n") != NULL){
                    head1(argv[2], argv[3]);
                }
                else{
                    head(argv[1]);
                }
            }
            else if(!strcmp(command,"tail")){
                if(strstr(argv[1],"-n") != NULL){
                    tail1(argv[2], argv[3]);
                }
                else{
                    tail(argv[1]);
                }
            }
            else{
                if(makeargv(cmdlist2," ",cmdargs,10)<=0){
                    perror("makeargv error");
                }
                if(!strcmp(argv[0], "grep")){
                    if(argv[1][0]=='-'){
                        if(argv[2][0]=='\''){
                            char *temp = strtok(argv[2], "'");
                            argv[2] = NULL;
                            argv[2] = temp;
                        }
                    }
                    else if(argv[1][0]=='\''){
                        char *temp = strtok(argv[1], "'");
                        argv[1] = NULL;
                        argv[1] = temp;
                    }
                }
                if(!(!strcmp(argv[0], "ls")||!strcmp(argv[0], "man")||!strcmp(argv[0], "grep")||!strcmp(argv[0], "sort")||!strcmp(argv[0], "awk")||!strcmp(argv[0], "bc"))){
                    fprintf(stderr, "swsh: Command not found\n");
                    return;
                }
                /*if(!strcmp(argv[0], "awk")){
                    if(!strcmp(argv[1],"'{")){
                        if(!strcmp(argv[3],"}'")){
                            strcat(argv[1], argv[2]);
                            printf("hahah: \n");
                            strcat(argv[1], argv[3]);
                            printf("hahah:%s\n", argv[1]);
                            argv[2] = NULL;
                            argv[3] = NULL;
                            for(int i=4; i<10; i++){
                                argv[i-2] = argv[i];
                                argv[i] = NULL;
                            }
                        }
                    }
                    printf("hello!!!\n");
                    if(argv[1][0]=='-'){
                        if(argv[2][0]=='\''){
                            char *temp = strtok(argv[2], "'");
                            argv[2] = NULL;
                            argv[2] = temp;
                        }
                    }
                    else if(argv[1][0]=='\''){
                        char *temp = strtok(argv[1], "'");
                        argv[1] = NULL;
                        argv[1] = temp;
                    }
                }*/
                /*for(i=0; i<10; i++){
                    printf("cmdargs%d : %s\n", i, cmdargs[i]);
                    printf("argv%d : %s\n", i, argv[i]);
                }*/
                execvp(argv[0], argv);
                fprintf(stderr, "swsh: Command not found\n");
                exit(0);
            }
            exit(0);
        }
        else{
            wait(NULL);
            return;
        }
    }
    free(cmdlist2);
    return;
}

void head(char* fileNm){
    char ch;
    int fd=open(fileNm,O_RDONLY);
    int n=0;
    while(read(fd,&ch,1)){
        if(ch=='\n'){
            n++;
        }
        if(n==10){
            break;
        }
        write(1,&ch,1);
    }
    write(1,"\n",1);
}

void head1(char* linenum, char* fileNm){
    char ch;
    int fd=open(fileNm,O_RDONLY);
    int n=0;
    while(read(fd,&ch,1)){
        if(ch=='\n'){
            n++;
        }
        if(n==atoi(linenum)){
            break;
        }
        write(1,&ch,1);
    }
    write(1,"\n",1);
}

void tail(char* fileNm){
    int fp=open(fileNm,O_RDONLY);
    if(fp==-1){
        fprintf(stderr, "swsh: Command not found\n");
    }
    int n=0;
    int flag=0;
    char c;
    lseek(fp,-2,SEEK_END);
    int cur_fp;
    int i=0;
    while(n<=10){
        lseek(fp,-i-1,SEEK_END);
        i++;
        flag++;
        read(fp,&c,1);
        if(c=='\n'||lseek(fp,0,SEEK_CUR)==1){
            if(n==0){
                n++;
                flag=0;
                continue;
            }
            if(lseek(fp,0,SEEK_CUR)==1){
                lseek(fp,-1,SEEK_CUR);
                flag++;
            }
            n++;
            char* one_line=(char*)malloc(512);
            read(fp,one_line,flag);
            write(1,one_line,flag);
            free(one_line);
            if(lseek(fp,0,SEEK_CUR)==flag) break;
            flag=0;
        }
    }
}

void tail1(char* linenum, char* fileNm){
    int fp=open(fileNm,O_RDONLY);
    int n=0;
    int flag=0;
    char c;
    lseek(fp,-2,SEEK_END);
    int cur_fp;
    int i=0;
    while(n<=atoi(linenum)){
        lseek(fp,-i-1,SEEK_END);
        i++;
        flag++;
        read(fp,&c,1);
        if(c=='\n'||lseek(fp,0,SEEK_CUR)==1){
            if(n==0){
                n++;
                flag=0;
                continue;
            }
            if(lseek(fp,0,SEEK_CUR)==1){
                lseek(fp,-1,SEEK_CUR);
                flag++;
            }
            n++;
            char* one_line=(char*)malloc(512);
            read(fp,one_line,flag);
            write(1,one_line,flag);
            free(one_line);
            if(lseek(fp,0,SEEK_CUR)==flag) break;
            flag=0;
        }
    }
}

void cat(char* file){
    char ch;
    int fd=open(file,O_RDONLY);
    while(read(fd,&ch, 1)){
        write(1,&ch,1);
    }
    close(fd);
}

void cp(char* file1, char* file2){
    char ch;
    int fd=open(file1, O_RDONLY);
    int fd2=open(file2,O_WRONLY|O_CREAT|O_TRUNC,0644);
    while(read(fd,&ch,1)){
        write(fd2,&ch,1);
    }
    close(fd);
    close(fd2);
}

void mv(char* file1, char* file2){
    if(file1==NULL || file2==NULL){
        //write(1, "mv: Permission denied\n", 22);
        errno = ENOENT;
        print_error("mv");
        return;
    }
    //int re;
    int move;
    move=link(file1,file2);
    move=unlink(file1);
    /*re = rename( file1, file2 );
    if(re=-1){
        errno = ENOENT;
        print_error("mv");
    }*/
}

void rm(char* file){
    int remove=unlink(file);
    if(remove==-1){
        errno = ENOENT;
        print_error("rm");
    }
}

void cd(char* dir){
    if(strlen(dir)==0){
        //chdir(getenv("HOME"));
        errno = ENOTDIR;
        print_error("cd");
    }
    else{
        if(chdir(dir)==-1){
            errno = ENOTDIR;
            print_error("cd");
        }
    }
}

void pwd(){
    char buf[MAX_INPUT];
    getcwd(buf,MAX_INPUT);
    write(1,buf,strlen(buf));
    write(1,"\n",1);
}

void exit_func(){
    exit(3);
}

int redirect_in(char** argv){
    int i;
    int breakpoint=0;
    for (i = 0; argv[i]!=NULL; i++) {
        if (!strcmp(argv[i], "<")) {
            breakpoint=i;
            break;
        }
    }
    if(!strcmp(argv[breakpoint], "<")){
        if (!argv[breakpoint+1]) {
            errno = ENOENT;
            return 0;
        }
        else {
            fd1 =open(argv[breakpoint+1],O_RDONLY, 0644);
            dum =  dup(STDIN_FILENO);
            dup2( fd1, STDIN_FILENO);
            //redirect_flag = 1;
        
            for (i = i + 2; argv[i] != NULL; i++) {
                argv[i-2] = argv[i];
            }
            argv[i-2] = NULL;
            return 1;
        }
    }
    return 0;
}
int redirect_out(char** argv){
    int i;
    int breakpoint=0;
    for (i = 0; argv[i]!=NULL; i++) {
        if (!strcmp(argv[i], ">")) {
            breakpoint=i;
            break;
        }
    }
    if(!strcmp(argv[breakpoint], ">")){
        if (!argv[breakpoint+1]) {
            errno = ENOENT;
            return 0;
        }
        else {
            fd1 =open(argv[breakpoint+1],O_WRONLY | O_TRUNC, 0644);
            dum =  dup(STDOUT_FILENO);
            dup2( fd1, STDOUT_FILENO);
            //redirect_flag = 2;
            for (i = i + 2; argv[i] != NULL; i++) {
                argv[i-2] = argv[i];
            }
            argv[i-2] = NULL;
        
            return 1;
        }
    }
    return 0;
}
int redirect_app(char** argv){
    int i;
    int breakpoint=0;
    for (i = 0; argv[i]!=NULL; i++) {
        if (!strcmp(argv[i], ">>")) {
            breakpoint=i;
            break;
        }
    }
    if(!strcmp(argv[breakpoint], ">>")){
        if (!argv[breakpoint+1]) {
            errno = ENOENT;
            return 0;
        }
        else {
            fd1 =open(argv[breakpoint+1],O_WRONLY | O_APPEND, 0644);
            dum =  dup(STDOUT_FILENO);
            dup2(fd1, STDOUT_FILENO);
            //redirect_flag = 2;
            for (i = i + 2; argv[i] != NULL; i++) {
                argv[i-2] = argv[i];
            }
            argv[i-2] = NULL;
            
            return 1;
        }
    }
    return 0;
}

void print_error(char* cmd){
    switch (errno) {
        case EACCES:
            fprintf(stderr, "%s: Permission denied.\n", cmd); break;
        case EISDIR:
            fprintf(stderr, "%s: Is a directory.\n", cmd); break;
        case ENOENT:
            fprintf(stderr, "%s: No such file or directory\n", cmd); break;
        case ENOTDIR:
            fprintf(stderr, "%s: Not a directory.\n", cmd); break;
        case EPERM:
            fprintf(stderr, "%s: Permission denied.\n", cmd); break;
        default:
            fprintf(stderr, "swsh: Command not found\n");
    }
    return;
}
