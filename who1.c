/*who1.c -a first version of the who program
 *   open,read UTMP file, and show results
 */
#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#define USER_PROCESS 7
void show_info(struct utmp *utbufp){
    if(utbufp->ut_type!=USER_PROCESS)
        return;
    printf("%-8.8s",utbufp->ut_name);
    printf("\n");
    printf("%-8.8s",utbufp->ut_line);
    printf(" ");
    printf("%10d",utbufp->ut_time);
    printf(" ");
    printf("(%s)",utbufp->ut_host);
    printf("\n");
}
int main(){
    struct utmp current_record;
    int utmpfd;
    int reclen=sizeof(current_record);
    if((utmpfd=open(UTMP_FILE,O_RDONLY))==-1){
        perror(UTMP_FILE);
        exit(1);
    }
    while(read(utmpfd,&current_record,reclen)==reclen){
        show_info(&current_record);
    }
    close(utmpfd);
    return 0;
}
