#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t pid_child;

void sig_handler(int signum){
  printf("\nInside handler function\n");
  printf("My child's PID is=%d\n", pid_child);
  printf("my pid=%d, my parent pid=%d\n",getpid(),getppid());
  signal(SIGINT,SIG_DFL);   // Re Register signal handler for default action
}

int main(int argc, char* argv[] )
{ 
	//if we want a zombie process, we allow parent to sleep, and don't provide an handler for SIGCHILD
	//if we want to avoid zombie process, then, we allow child to sleep, 
	char input; 
	int z = 0, parent=0; //z=0 => non zombie, z=1=> zombie
	printf("Press 'y' or 'Y' if you want to create zombie process\n");
	scanf("%c", &input);
	if (input=='y' || input == 'Y')
	{
		z = 1;
	}
	else 
	{ // Register signal handler
		// printf("signal\n");
		signal(SIGCHLD,sig_handler);
	}
	
	pid_child = fork();
	if (pid_child>0)
	{
		parent=1; //determines if parent or child
	}

	
	if( pid_child < 0 )
	{ // error in fork 
		printf("error in forking\n");
		return(1);
	}
	else if( (parent^z)==0 )//sleep
	{
		printf("s: p=%d, z=%d, p^z=%d\n",parent,z,parent^z);
		printf("my pid=%d, my parent pid=%d\n",getpid(),getppid());
		sleep(15);
	}
	else
	{
		printf("ns: p=%d, z=%d, p^z=%d\n",parent,z,parent^z);
		printf("my pid=%d, my parent pid=%d\n",getpid(),getppid());
	}
	
}