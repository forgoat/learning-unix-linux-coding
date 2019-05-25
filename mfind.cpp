#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<dirent.h>
#include<regex.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>


/*                    定义全局变量                                     */

#define TRUE 1  
#define FALSE 0
#define MAX_RESULT 500//最多能存储多少个查询结果


/*                    指示是否开启下列选项                    */

int nameToggle=FALSE;//是否开启-name选项
int pruneToggle=FALSE;//是否开启-prune选项
int mtimeToggle=FALSE;//是否开启-mtime选项
int ctimeToggle=FALSE;//是否开启-ctime选项
int printToggle=FALSE;//是否开启-print选项
int execToggle=FALSE;//是否开启-exec选项

int namePosition=0;//-name是第几个参数
int prunePosition=0;//-prune是第几个参数
int mtimePosition=0;//-mtime是第几个参数
int ctimePosition=0;//-ctime是第几个参数
int execPosition=0;//-exec是第几个参数

char* result[MAX_RESULT];//存储查询结果
int resultPosition=0;//result数组下标




/*                         声明函数                      */

int matchName(char* string,char* pattern);//文件名匹配
int matchMtime(struct stat stat_buf,int n);//修改时间匹配
int matchCtime(struct stat stat_buf,int n);//创建时间匹配
char* getPath(char* path,char* filename);//将路径和文件名连接在一起
void printHelp();//获取获取命令的帮助说明
void initArgvPosition(int argc,char*argv[]);//输入名中初始化各参数在数组中的位置
void findFunc(int argc,char* argv[],char * path);//find的关键功能
void execFunc(int argc,char* argv[]);//如果带有-exec，执行相应的程序



/*                main函数                             */

int main (int argc, char *argv[] )
{	
	if(argc<=2){//格式错误时显示帮助信息
		printf("格式错误!\n");
		printHelp();
		exit(EXIT_FAILURE);
	}
	initArgvPosition(argc,argv);
	findFunc(argc,argv,argv[1]);
	if(execToggle==TRUE){
		execFunc(argc,argv);
	}
	return EXIT_SUCCESS;
}



/*                         匹配名字                      */

int matchName(char* string,char* pattern){
	regex_t reg;
	if(regcomp(&reg,pattern,REG_NOSUB|REG_ICASE)!=0){
		perror("compile regex error");
		printf("regex:%s\n",pattern);
		exit(EXIT_FAILURE);
	}
	int status=regexec(&reg,string,(size_t)0,NULL,0);
	regfree(&reg);
	if(status==0)
		return TRUE;
	return FALSE;
}



/*                   匹配时间                      */

int matchMtime(struct stat stat_buf,int n){
	time_t now=time(NULL);
	time_t mtime=stat_buf.st_mtime;
	int day=(int)difftime(now,mtime)/(60*60*24);//上次修改距今天的天数
	if(n<0)
	{
		//搜索今天到n天前之间修改过的文件
		if(day+n<0)	return TRUE;
	}
	else
	{
		//搜索n天之前修改过的文件
		if(day>=n)	return TRUE;
	}
	return FALSE;
}



/*                  匹配创建时间                     */

int matchCtime(struct stat stat_buf,int n){
	time_t now=time(NULL);
	time_t ctime_=stat_buf.st_ctime;
	int day=(int)difftime(now,ctime_)/(60*60*24);//自文件创建到今天的天数
	if(n<0)
	{
		//今天到n天前之间创建的文件
		if(day+n<0) return TRUE;
	}
	else
	{
		//n天前创建的文件
		if(day>=n)	return TRUE;
	}
	return FALSE;
}



/*                         获取路径                      */

char* getPath(char* path,char* filename)
{
	//根据目录名和目录下的文件名获取文件路径
	int len1=strlen(path);
	int len2=strlen(filename);
	char* str=(char*)malloc(len1+len2+2);
	str[0]='\0';
	str=strcat(str,path);
	if(path[len1-1]!='/')	str=strcat(str,"/");
	str=strcat(str,filename);
	return str;
}


/*                  输出帮助信息                */

void printHelp()
{
	printf("用法：find 路径 -选项 正则表达式\n");
	printf("选项说明：\n");
	printf("-name \"文件\"\n\t指定要查找的文件名\n");
	printf("-prune 目录\n\t指出搜索时不搜索该目录\n");
	printf("-mtime +n或-n\n\t按时间查找\n");
	printf("\t+n表示n天之前修改过的文件\n");
	printf("\t-n表示n今天到n天之前修改过的文件\n");
	printf("-ctime +n或-n\n\t按时间查找\n");
	printf("\t+n表示n天之前创建的文件\n");
	printf("\t-n表示今天到n天前之间创建的文件\n");
	printf("-print\n\t将搜索结果输出到标准输出\n");
	printf("-exec 程序名\n\t对查找到的结果执行指定的程序\n");
}


/*                确定参数的位置                           */

void initArgvPosition(int argc,char*argv[])
{
	int i;
	if(argv[2][0]!='-')
	{
		printHelp();
		exit(EXIT_FAILURE);
	}
	for(i=2;i<argc;i++){
		if(argv[i][0]!='-')
			continue;
		if(strcmp(argv[i],"-name")==0){
			nameToggle=TRUE;
			namePosition=i;
		}
		else if(strcmp(argv[i],"-prune")==0){
			pruneToggle=TRUE;
			prunePosition=i;
		}
		else if(strcmp(argv[i],"-mtime")==0){
			mtimeToggle=TRUE;
			mtimePosition=i;
		}
		else if(strcmp(argv[i],"-ctime")==0){
			ctimeToggle=TRUE;
			ctimePosition=i;
		}
		else if(strcmp(argv[i],"-print")==0){
			printToggle=TRUE;
		}
		else if(strcmp(argv[i],"-exec")==0){
			execToggle=TRUE;
			execPosition=i;
		}
	}
}



/*     find功能函数                      */

void findFunc(int argc,char* argv[],char * path){
	struct dirent *dir_struct;//保存读取目录的结果
	DIR* dir;//目录流
	struct stat stat_buf;//用来存储文件属性的结构体
	if(path==NULL)	path=argv[1];//path表示要搜索的目录
	char* filepath;//目录下文件的路径	

	//在目录中搜索
	if((dir=opendir(path))==NULL)
	{
		//打开目录
		perror("function opendir:");
		printf("dir:%s\n",path);
		return; 
	}
	do
	{
		//读取目录下的文件，直到读完为止
		errno=0;//number of last error
		if((dir_struct=readdir(dir))!=NULL){//读取目录下当前文件
			int matchToggle=TRUE;//是否匹配
			filepath=getPath(path,dir_struct->d_name);
			if(lstat(filepath,&stat_buf)!=0){				
				perror("function stat error");
				printf("stat path:%s\n",filepath);
				continue;
			}
			if(nameToggle==TRUE){
				//-name
				int len=strlen(argv[namePosition+1]);
				if(argv[namePosition+1][0]=='"'&&argv[namePosition+1][len-1]=='"'){
					int index=0;
					for(;index<len-2;index++)
						argv[namePosition+1][index]=argv[namePosition+1][index+1];
					argv[namePosition+1][len-2]='\0';
				}
				if(!matchName(dir_struct->d_name,argv[namePosition+1]))
					matchToggle=FALSE;
			}
			if(mtimeToggle==TRUE){
				//-mtime
				if(!matchMtime(stat_buf,atoi(argv[mtimePosition+1])))
					matchToggle=FALSE;
			}
			if(ctimeToggle==TRUE){
				//-ctime
				if(!matchCtime(stat_buf,atoi(argv[ctimePosition+1])))
					matchToggle=FALSE;
			}
			if(matchToggle==TRUE){
				if(execToggle==TRUE){
					if(resultPosition<500)
						result[resultPosition++]=filepath;//将查询结果存储到result数组
				}
				else//若同时满足-name,-mtime,-ctime选项，且没有-exec选项则输出
					printf("%s\n",filepath);
			}
			if(S_ISDIR(stat_buf.st_mode)
					&&strcmp(dir_struct->d_name,".")!=0
					&&strcmp(dir_struct->d_name,"..")!=0){
				//若是子目录，则判断是否应该进入其中搜索
				if(pruneToggle==TRUE){
					struct stat stat_buf1,stat_buf2;
					if(lstat(argv[prunePosition+1],&stat_buf1)!=0){
						perror("function lstat error");
						printf("stat path:%s\n",argv[prunePosition+1]);
						exit(EXIT_FAILURE);
					}
					if(lstat(filepath,&stat_buf2)!=0){
						perror("function lstat error");
						printf("stat path:%s\n",filepath);
						exit(EXIT_FAILURE);
					}
					if(stat_buf1.st_ino==stat_buf2.st_ino
							&&stat_buf1.st_dev==stat_buf2.st_dev)
						continue;//若该目录为-prune选项指定的目录，则不进入该目录搜索
				}
				findFunc(argc,argv,filepath);//进入子目录搜索
			}
		}
	}while(dir_struct!=NULL);
	if(errno!=0){
		perror("readdir error");
		exit(EXIT_FAILURE);
	}
	closedir(dir);
}





/*                           对查询结果执行指定程序                */

void execFunc(int argc,char* argv[]){//对查询结果执行指定程序/* {{{ */
	pid_t pid;	
	pid=fork();
	if(pid==0){//在子进程执行指定程序
		char* cmd[resultPosition+2];
		int i,j=0;
		for(i=execPosition+1;i<argc&&argv[i][0]!='-';i++){
			cmd[j++]=argv[i];
		}
		for(i=0;i<resultPosition-1;i++){
			cmd[j++]=result[i];
		}
		cmd[resultPosition]=(char*)0;
		execvp(cmd[0],cmd);
	}
	else if(pid>0){
		wait(NULL);
	}
}

