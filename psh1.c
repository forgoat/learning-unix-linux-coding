/* prompting shell version 1
 * Prompts for the command and its arguments
 * Builds the argument vector for the call to execvp
 * Use execvp(),and never returns
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>

#define MAXARGS 20
#define ARGLEN 100

void execute(char* arglist[]);
char* makestring(char*);

int main(){
    char *arglist[MAXARGS+1];
    int numargs=0;
    char argbuf[ARGLEN];
    while(numargs<MAXARGS){
        printf("Arg[%d]?",numargs);
        if(fgets(argbuf,ARGLEN,stdin)&&*argbuf!='\n'){
            arglist[numargs++]=makestring(argbuf);
        }
        else{
            if(numargs>0){
                arglist[numargs]==NULL;
                execute(arglist);
                numargs=0;
            }
        }
    }
    return 0;
}

void execute(char *arglist[]){
    execvp(arglist[0],arglist);
    perror("execvp failed");
    exit(1);
}

char* makestring(char *buf){
    char *cp;
    buf[strlen(buf)-1]='\0';
    cp=(char*)malloc(strlen(buf)+1);
    if(cp==NULL){
        fprintf(stderr,"no memory\n");
        exit(1);
    }
    strcpy(cp,buf);
    return cp;
}

