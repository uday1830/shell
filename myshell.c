#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
char** parseInput(char* input)
{
    char **commands;
	char *command;
	commands=malloc(64*sizeof(char*));
	int i=0;
	while((command=strsep(&input," \t\r\n\a"))!=NULL)
	{
	   commands[i]=command;
	   i++;               
    }
    commands[i]=NULL;
    return commands;
}
void executeCommand(char **fcommands)
{
	if(strcmp(fcommands[0],"cd")==0)  //for changing the directory
	{
		chdir(fcommands[1]);
	}
	else
	{
		int rc=fork();
		if(rc<0) //fork failed
		{
			exit(1);
		}
		else if(rc==0)  ///child process
		{
			if(execvp(fcommands[0],fcommands)<0)
			{
			    printf("Shell: Incorrect command\n");
                exit(1);
			}
		}
		else  //parent process
		{
			wait(NULL);
		}
		return;
	}
}
void executeParallelCommands(char **fcommands)
{
	
	int i=0;
	int j=i;
	pid_t pid;
	while(fcommands[i]!=NULL)
	{
	
	while(fcommands[i]!=NULL&&strcmp(fcommands[i],"&&")!=0)
	{
		i++;
	}
	fcommands[i]=NULL;
	if(strcmp(fcommands[j],"cd")==0)
	{
		chdir(fcommands[j+1]);
	}
	else
	{
	
	pid=fork();
	if(pid<0)
	{
		//fork failed
		exit(1);
	}
	else if(pid==0)//child process
	{
		if(execvp(fcommands[j],&fcommands[j])<0)
		{
			printf("Shell: Incorrect command\n");
            exit(1);
		}
	} 
    }
    i++;
    j=i;
}
	while(wait(NULL)>0);
}
void executeSequentialCommands(char **fcommands)
{
	
	int i=0;
	int j=i;
	pid_t pid;
	while(fcommands[i]!=NULL)
	{
		while(fcommands[i]!=NULL&&strcmp(fcommands[i],"##")!=0)
		{
			i++;
		}
		fcommands[i]=NULL;
		if(strcmp(fcommands[j],"cd")==0)
	{
		chdir(fcommands[j+1]);
	}
	else{
		pid=fork();
		if(pid<0)
		{
			//fork failed
			exit(1);
		}
		else if(pid==0) //child process
		{
			if(execvp(fcommands[j],&fcommands[j])<0)
			{
				printf("Shell: Incorrect command\n");
                exit(1);
			}
		}
		else
		{
			while(wait(NULL)>0);
		}
		}
		i++;
		j=i;
	}
}
void executeCommandRedirection(char **fcommands)
{
	int i=0;
	int j=i;
	pid_t pid,pid1;
	while(fcommands[i]!=NULL&&strcmp(fcommands[i],">")!=0)
	{
		i++;
	}
	fcommands[i]=NULL;
	pid=fork();
	if(pid<0)
	{
		//fork failed
		exit(1);
	}
	else if(pid==0) //child process
	{
		close(STDOUT_FILENO);//for closing the output.
		open(fcommands[i+1],O_CREAT|O_WRONLY|O_APPEND);
		if(execvp(fcommands[0],fcommands)<0)
		{
			printf("Shell: Incorrect command\n");
			exit(1);
		}
	}
	else //parent process
	{
	    pid1=wait(NULL);
	}
}



void printdir()  //function to print the current directory and $.
{
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	printf("%s$",cwd);
}

int main()
{
	char* input;
	long int size=0;
	input=(char*)malloc(size);
	
	char **fcommands;
	
	//used for handling ctrl+c and ctrl+z
	signal(SIGINT,SIG_IGN); 
	signal(SIGTSTP,SIG_IGN);
	while(1)
	{
	int i=0;
	printdir();// to print the prompt as currentdirectoryname and $ beside that.
	getline(&input,&size,stdin); //reading input using getrline function.
	
    while(input[i]!='\0')
     	{
		if(input[i]==EOF||input[i]=='\n')
		{
			input[i]='\0';
		}
		i++;
	    }
	    fcommands=parseInput(input);
	    if(strcmp(fcommands[0],"exit")==0)
	    {
	    	printf("Exiting shell...\n");
			break;
		}
	int retval=0;
	i=0;
	while(fcommands[i]!=NULL)
	{
		if(strcmp(fcommands[i],"&&")==0)
		{
			retval=1;
		}
		else if(strcmp(fcommands[i],"##")==0)
		{
			retval=2;
		}
		else if(strcmp(fcommands[i],">")==0)
		{
			retval=3;
		}
		i++;
	}
	if(retval==1)
	{
		executeParallelCommands(fcommands);
	}
	else if(retval==2)
	{
		executeSequentialCommands(fcommands);
	}
	else if(retval==3)
	{
		executeCommandRedirection(fcommands);
	}
	else
	{
		executeCommand(fcommands);
	}
	}
	return 0;
	
}
