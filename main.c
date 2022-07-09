#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#define CMD_LEN 512
#define ARGS_LEN 256
#define LONG_LEN 4096
int mystdin;//文件描述符定向
int mystdout;//文件描述符定向
int cmdnum=0;//命令切割个数
int fds[2];//管道命令数组
char cmd[CMD_LEN];//输入字符
char *args[ARGS_LEN];//分割完成的字符串
char nowpath[LONG_LEN]="";//当前路径
char cmd2[CMD_LEN]="";//CMD数组的复制
char *inputBuf;//管道子进程输入
char *outputBuf;//管道父进程输入
void mydel();//删除函数
void ls();//列表函数
void cd();//进入函数
void pwd();//当前路径
void make();//新建文件
int CAT();//打开函数
void docommand();//执行命令
void cmdcutline(char *cmd);//读与分割命令
int cmdoutrelo();//输入重定向
int cmdpipe();//管道
int cmdback();//后台
void core();//主函数调用
int cmdreout();//追加重定向
void cmdcutline(char *cmd)//读输入数据并拆分到args数组
{	int i=0;
	args[0]=cmd;
	/* 拆解命令行 */
    args[0] = cmd;
    for (i = 0; *args[i]; i++)
        for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
            if (*args[i+1] == ' ') {
                *args[i+1] = '\0';
                args[i+1]++;
                break;
            }
						args[i]='\0';
						cmdnum=i;
        args[i+1] = NULL;
		
		

        /* 没有输入命令 */
        if (!args[0])
            return;
}
void docommand()//执行命令
{int r=0;
	pid_t pid;
	/*判断是否为后台运行*/
		for (r = 0; r < cmdnum; r++) {
        if (strcmp(args[r], "&") == 0) {
            
            int flag = cmdback();
            return;
        }
    }

	/*判断是否为重定向符号*/
	for(r=0;r<cmdnum;r++)
	{
	
		if(strcmp(args[r],">")==0)
		{
			
			int flag=cmdoutrelo();
			
			return;
		}
	}
		/*判断是否为管道符号*/
	for(r=0;r<cmdnum;r++)
	{
		if(strcmp(args[r],"|")==0)
		{		//pipebufcode=-1;
				int flag=cmdpipe();
				//int flag=cmdpipw()
				//exit(0);
				return;
		}
	}
		
		for (r = 0; r< cmdnum; r++) {
        if (strcmp(args[r], ">>") == 0) {
            
            int temp = cmdreout();
            return;
        }
    }

/*
一系列内建命令
*/
	 if(strcmp(args[0],"cd")==0)
	{
		cd();
		return;
	}
	else if(strcmp(args[0],"pwd")==0)
	{
		if(args[1]!=NULL)
		{
			printf("存在不合法参数，已经忽略\n");
		}
		pwd();
		return;
	}else if(strcmp(args[0],"ls")==0)
	{if(args[1]!=NULL)
		{
			printf("存在不合法参数，已经忽略\n");
		}
			ls();
			return;
	}else if(strcmp(args[0],"drop")==0)
	{
			mydel();
			return;
	}else if(strcmp(args[0],"make")==0)
	{
			make();
			return;
	}else if(strcmp(args[0],"cat")==0)
	{
		int f= CAT();
		return;
	}

	else if(strcmp(args[0],"exit")==0)
	{
		exit(0);
		
	}/*
	调用execvp执行命令
	*/
	
	
	else 
	{
		pid=fork();
		 switch(pid)
		 {
			case -1:
		{	 printf("创建子进程未成功");
                return;
		}
			case 0:
			{
			execvp(args[0], args);
			printf("%s: 命令错误\n", args[0]);
			 return ;
			}
			default:{
				int status;
                    waitpid(pid, &status, 0);      // 等待子进程返回
                    int err = WEXITSTATUS(status); // 读取子进程的返回码

                    if (err) { 
                        printf("commandError: %s\n", strerror(err));
												exit(0);
			}
		 }

	}
           
	}
}
void mydel()
{
	remove(args[1]);
}
void pwd()
{
		char wd[4096];
    puts(getcwd(wd, 4096));
}
void ls()
{	getcwd(nowpath,4096);
	DIR *dirp;
	dirp  = opendir(nowpath);
	struct dirent d,*dp = &d;
	while((dp=readdir(dirp))!=NULL){//读取目录
		
       printf("%s\n",dp->d_name);
			 
    }
	closedir(dirp);
	
}
void cd()
{char cdpath[4096];
getcwd(cdpath,4096);

struct stat st;

if (args[1]){
    stat(args[1],&st);
    if (S_ISDIR(st.st_mode))//判断是否为目录
        chdir(args[1]);
    else{
        printf("[Error]: cd '%s': No such directory\n",args[1]);
        
    }
}
}
int CAT(){//阅读文件
		getcwd(nowpath,4096);
		
    char buf[CMD_LEN];
		memset(buf,0,sizeof(char)*CMD_LEN);
    int fd = open(args[1],'r'),n;
		
    while((n=read(fd,buf,CMD_LEN))>0){
       // write(STDOUT_FILENO,buf,n);
			 
				printf("%s",buf);
    }
		//free(buf);
    return 0;
}	
void make()
{
	FILE *fp=fopen(args[1],"w");
	if(fp==NULL)
	{
		printf("[Error]: make '%s': File create error\n",args[1]);
	}else{
	fclose(fp);
} 
}
int cmdoutrelo()
{
	int RediNum = 0;                        // 重定向符号数量
    for ( int i = 0; i + 1 < strlen(cmd2); i++) {
        if (cmd2[i] == '>' && cmd2[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
		if (RediNum != 1) {
        printf("输出重定向指令输入有误!");
        return 0;
    }

	 int sp=0;
	// strcmp(cmd2,cmd);
	 char outFile[CMD_LEN];
	 memset(outFile, 0x00, CMD_LEN);
	 for(int i=0;i<cmdnum;i++)
	 {
		if(strcmp(args[i], ">") == 0)
	{	if(i+1<cmdnum)
		{
			strcpy(outFile, args[i + 1]);
		}else{
			printf("缺少输出文件!");
                return 0;
		}
		}
	 }
for(sp=0;sp<strlen(cmd2);sp++)//分割
{
	if((cmd2[sp])=='>')
	{
		break;
	}
}
cmd2[sp-1]='\0';
cmd2[sp]='\0';
cmdcutline(cmd2);
pid_t pid;
pid=fork();//子进程重定向；
switch(pid)
{
	case -1:
	{
		 printf("创建子进程未成功");
            return 0;
	}
	case 0://子进程
	{
	
 char buff[512];
 memset(buff,0,sizeof(char)*512);
	int myf;
		int fd;
	myf=open(args[0],O_RDONLY);
					
				
					if(myf>0)
					{
					fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
				if (fd < 0) {
                exit(1);
            }		
							read(myf,buff,500);
						
							//dup(fd);
							write(fd,buff,strlen(buff));
							close(fd);
							close(myf);
						exit(0);
					}
				else //父进程
				{	close(myf);
							fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);//返回目标文件文件描述符
						 
		  if (fd < 0) {
                exit(1);
            }		
							
							dup2(fd, STDOUT_FILENO);
							execvp(args[0], args);//执行命令
							if (fd != STDOUT_FILENO) {  // 关闭fd, 还原标准输出
               close(fd);
								printf("command%s: 命令输入错误fd != STDOUT_FILENO\n", args[0]);
            
            return -1;
						
						}
				}
		}

default:
 {
            int status;
            waitpid(pid, &status, 0);       // 等待子进程返回
            int err = WEXITSTATUS(status);  // 读取子进程的返回码
            if (err) { 
                printf("default:Error: %s\n", strerror(err));
            } 
        }               
}

}

int  cmdback()//后台运行
{
char backgroundBuf[strlen(cmd2)];//提取数组
    memset(backgroundBuf, 0x00, strlen(cmd2));
    // 将&前面的命令提取出来
    for (int i = 0; i < strlen(cmd2); i++) {
        backgroundBuf[i] = cmd2[i];
        if (cmd2[i] == '&') {
            backgroundBuf[i] = '\0';
            backgroundBuf[i - 1] = '\0';
            break;
        }
    }

    pid_t backp;
    backp = fork();
    if (backp < 0) {
		
        perror("fork()");
        exit(1);
    }

    else if (backp == 0) {
       
        signal(SIGCHLD,SIG_IGN);
				cmdcutline(backgroundBuf);
       pid_t childp=fork();
			 if(childp==0)
        {//Sleep(5000);
					execvp(args[0], args);
					printf("执行错误\n");
					return -1;}
				else if(childp>0)
				{
					wait(NULL);
					printf("后台进程已经完成\n");
					exit(0);
				}
				}else {
			 printf("后台进程pid是%d\n",backp);
        // 父进程不等待子进程结束就返回
	
	return 1;
    }
}

int main()//主函数
{	mystdout=dup(STDOUT_FILENO);//恢复原始指向
mystdin=dup(STDIN_FILENO);//恢复原始指向
	while(1)
  {	dup2(mystdout,STDOUT_FILENO);
		dup2(mystdin,STDIN_FILENO);
	core();
	}
  
  return 0;
}
void core()
{
	getcwd(nowpath,4096);
		printf("[shell]%s:$ ",nowpath);
	memset(cmd,0,CMD_LEN);
  fflush(stdin);
  fgets(cmd, 256, stdin);
	if(cmd[0]==' '||cmd[0]=='\n')
	{
		return ;
	}
	cmd[strlen(cmd) - 1] = '\0';
	strcpy(cmd2,cmd);
	cmdcutline(cmd);

	docommand();
	cmdnum=0;

	
  
}
int cmdpipe()
{
	int i,j=0;
	for(j = 0; cmd2[j] != '\0'; j++) {
        if (cmd2[j] == '|')
            break;
    }
char outputBuf[j];

    memset(outputBuf, 0, j);
    char inputBuf[strlen(cmd2) - j];
		memset(inputBuf, 0, strlen(cmd2) - j);
    memset(inputBuf, 0, strlen(cmd2) - j);
    for (i = 0; i < j - 1; i++) {
        outputBuf[i] = cmd2[i];
    }
    for (i = 0; i < strlen(cmd2) - j - 1; i++) {
        inputBuf[i] = cmd2[j + 2 + i];
    }

    pipe(fds);
    
   // int result = 0;
	
    pid_t pid = fork();
		if (pid == -1) {
        return -1;
    } else if (pid == 0) { // 子进程执行单个命令
				close(fds[0]);
			  cmdcutline(outputBuf);
				dup2(fds[1],STDOUT_FILENO);
				execvp(args[0], args);
				close(fds[1]);
				return -1;
        if (fds[1] != STDOUT_FILENO) {
            close(fds[1]);
						}
						} else { // 父进程执行后续命令
        int status;
			//	char buf1[500];
        waitpid(pid, &status, 0);
		   int err = WEXITSTATUS(status);
				 if (err) { 
            printf("Error: %s\n", strerror(err));
        }	
			 close(fds[1]);
				dup2(fds[0],STDIN_FILENO);
				//	dup2(STDOUT_FILENO,mystdout);
				cmdcutline(inputBuf);
					//read(fds[0],buf1,strlen(inputBuf));
					pid_t pid1=fork();
				if(pid1==0)//子进程执行命令
			{	
				  execvp(args[0], args);
					close(fds[0]);
					return -1;
					if (fds[0] != STDIN_FILENO) {
            close(fds[0]);
					}
			}
			else if(pid1>0)//父进程等待
			{
				 int status;
			//	char buf1[500];
        waitpid(pid1, &status, 0);
			
			}else if(pid1<0)
			{
				return -1;
			}
			}   return 1;

}
int cmdreout()
{
	int RediNum = 0;                        // 重定向符号数量
    for ( int i = 0; i + 1 < strlen(cmd2); i++) {
        if (cmd2[i] == '>' && cmd2[i + 1] == '>'&&cmd2[i + 2]==' ') {
            RediNum++;
            break;
        }
    }
		if (RediNum != 1) {
        printf("输出重定向指令输入有误!\n");
        return 0;
    }

	 int sp=0;
	// strcmp(cmd2,cmd);
	 char outFile[CMD_LEN];
	 memset(outFile, 0x00, CMD_LEN);
	 for(int i=0;i<cmdnum;i++)
	 {
		if(strcmp(args[i], ">>") == 0)
	{	if(i+1<cmdnum)
		{
			strcpy(outFile, args[i + 1]);
		}else{
			printf("缺少输出文件!");
                return 0;
		}
		}
	 }
for(sp=0;sp<strlen(cmd2);sp++)//分割
{
	if((cmd2[sp])=='>'&&cmd2[sp+1]=='>')
	{
		break;
	}
}
cmd2[sp-1]='\0';
cmd2[sp]='\0';
//cmd2[sp]='\0';
cmdcutline(cmd2);
pid_t pid;
pid=fork();//子进程重定向；
switch(pid)
{
	case -1:
	{
		 printf("创建子进程未成功");
            return 0;
	}
	case 0://子进程
	{
	
 char buff[512];
 memset(buff,0,sizeof(char)*512);
	int myf;
		int fd;
	myf=open(args[0],O_RDONLY);
					
				
					if(myf>0)
					{
					fd = open(outFile, O_WRONLY|O_APPEND|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
				if (fd < 0) {
                exit(1);
            }		
							read(myf,buff,500);
						
							//dup(fd);
							write(fd,buff,strlen(buff));
							close(fd);
							close(myf);
						exit(0);
					}
				else //父进程
				{	close(myf);
							fd = open(outFile, O_WRONLY|O_APPEND|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);//返回目标文件文件描述符
						 
		  if (fd < 0) {
                exit(1);
            }		
							
							dup2(fd, STDOUT_FILENO);
							execvp(args[0], args);//执行命令
							if (fd != STDOUT_FILENO) {  // 关闭fd, 还原标准输出
               close(fd);
								printf("command%s: 命令输入错误fd != STDOUT_FILENO\n", args[0]);
            
            return -1;
						
						}
				}
		}

default:
 {
            int status;
            waitpid(pid, &status, 0);       // 等待子进程返回
            int err = WEXITSTATUS(status);  // 读取子进程的返回码
            if (err) { 
                printf("default:Error: %s\n", strerror(err));
            } 
        }               
}

}