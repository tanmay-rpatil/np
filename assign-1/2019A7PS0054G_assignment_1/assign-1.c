#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t pid_child;

void sig_handler(int signum){
  printf("\nMy child has exited\n");
  printf("My child's PID is=%d\n", pid_child);
//   printf("my pid=%d, my parent pid=%d\n",getpid(),getppid());
  signal(SIGINT,SIG_DFL);   // Re Register signal handler for default action
}

int main(int argc, char* argv[] )
{ 
	//if we want a zombie process, we allow parent to sleep, and don't provide an handler for SIGCHILD
	//if we want to avoid zombie process, then, we allow parent to sleep, and provide SIGCHILD handler 
	char input; 
	printf("Press 'y' or 'Y' if you want to create zombie process\n");
	scanf("%c", &input);
	if (input!='y' && input != 'Y')
	{
		// Register signal handler
		signal(SIGCHLD,sig_handler);
	}
	
	pid_child = fork();
	
	if( pid_child < 0 )
	{ // error in fork 
		printf("error in forking\n");
		return(1);
	}
	else if( pid_child>0 )//put parent to sleep
	{
		printf("my pid=%d, my child's pid=%d\n",getpid(),pid_child);
		sleep(10);
	}
	else //child, no sleep. print and exit
	{
		printf("my pid=%d, my parent pid=%d\n",getpid(),getppid());
	}
	
	return(0);
}