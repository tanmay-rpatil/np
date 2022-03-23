#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

pid_t pid_child;


int main(int argc, char* argv[] )
{ 
	int pipe_cp_fd[2];
	int pipe_pc_fd[2];

	int send;
	int recv;

	if(pipe(pipe_pc_fd)<0)
	{
		perror("error in pc pipe creation");
		exit(1);
	}
	if(pipe(pipe_cp_fd)<0)
	{
		perror("error in cp pipe creation");
		exit(1);
	}

	pid_child = fork();
	if( pid_child < 0 )
	{ // error in fork 
		printf("error in forking\n");
		return(1);
	}
	else if( pid_child>0 )//parent
	{
		close(pipe_pc_fd[READ]);//close read end of sending pipe
		close(pipe_cp_fd[WRITE]);//close send end of receiving pipe
		
		printf("Parent: my pid=%d. Type an int for child:",getpid());

		scanf("%d", &send);

		if(write(pipe_pc_fd[WRITE], (void *) &send, sizeof(send)+1)<0)
		{
			perror("error in p writing");
			exit(2);
		}
		close(pipe_pc_fd[WRITE]);//close send end of send pipe

		if( read(pipe_cp_fd[READ],(void *) &recv, sizeof(recv))<0 )
		{
			perror("error in p read");
			exit(3);
		}		
		close(pipe_cp_fd[READ]);//close recv end of rec pipe
		printf("%d: recieved: %d\n",getpid(), recv);
		wait(NULL);
	}
	else //child, no sleep. print and exit
	{
		close(pipe_cp_fd[READ]);//close read end of sending pipe
		close(pipe_pc_fd[WRITE]);//close send end of receiving pipe

		if( read(pipe_pc_fd[READ],(void *) &recv, sizeof(recv))<0 )
		{
			perror("error in c read");
			exit(3);
		}	
		close(pipe_pc_fd[READ]);//close recv end of rec pipe
		printf("%d: recieved: %d\n",getpid(), recv);
		
		printf("Child: my pid=%d. Type an int for parent:",getpid());	
		scanf("%d", &send); 
		if(write(pipe_cp_fd[WRITE], (void *) &send, sizeof(send)+1)<0)
		{
			perror("error in c writing");
			exit(2);
		}
		close(pipe_cp_fd[WRITE]);//close send end of send pipe

	}
	
	return(0);
}