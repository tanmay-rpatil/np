#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#define MAXSIZE 1024
#define MAXCONN 4

int server_fd, client_count, clients_read;
sem_t sem, sem_done;
struct data_needed{
	int new_sock;
	struct sockaddr_in * new_addr;
};

void intHandler(int dummy) {
    // close(new_sock);
	close(server_fd);
	exit(0);
}

void * worker(void *args){
	struct data_needed new_data = *(struct data_needed *)args;
	int new_sock = new_data.new_sock;
	ssize_t read_return;
	char buffer[MAXSIZE] = {0};
	sem_init(&sem,0,MAXCONN);
	while(1)
	{
		//Get rno from client
		memset(buffer, 0, sizeof(buffer));//flush
		read_return = recv( new_sock ,(void *) buffer, sizeof(buffer), 0 );
		if(read_return<0){
			perror("error in reading\n");
			exit(5);
		}
		else if(!read_return){	
			printf("-> exiting\n");
			break;
		}
		
		printf("C: ");
		for (int i = strlen(buffer)-1; i >=0; i--)
		{
			printf("%c",buffer[i]);
		}

		//scan only if all are done
		// sem_wait(&sem_done);
		// sem_wait(&sem);
		// if(client_count==clients_read){
			printf("\nS(provide ip):");
			scanf("%[^\n]%*c", buffer);
			if ( send( new_sock, buffer, read_return, 0) < 0 ){
				perror("faied to send\n");
				exit(6);
			}
		// }
		// sem_post(&sem_done);
		// sem_post(&sem);
	}
	close (new_sock);
	//lock
	// sem_wait(&sem);
	client_count--;
	//unlock
	// sem_post(&sem);
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    client_count = 0;
	clients_read = 0;
	struct sockaddr_in address;
	int addrlen;
	sem_init(&sem,0,1);
	sem_init(&sem_done,0,1);
	// char buffer[MAXSIZE] = {0};
	int PORT = 4444;
	
	if (argc >=2)//port
    {
        PORT = atoi(argv[1]);
    }

	//close sockets when ctrl+c
	signal(SIGINT, intHandler);
	//create socket with options: domain: AF_INET for ipv4, type: SOCK_STREAM for TCP, protocol: 0 for IP.
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd<0){
		perror("failed to create socket\n");
		exit(1);
	}

	address.sin_family = AF_INET; //IP
	address.sin_addr.s_addr = INADDR_ANY; // localhost/ kernel choses ip
	address.sin_port = htons( PORT ); //set port to 4444
	//bind to a port and ip
	if( bind( server_fd, (struct sockaddr *) &address, sizeof(address) ) < 0 ){
		perror("failed to bind\n");
		exit(2);//bind failed
	}

	//listen at a socket ->  accept connection requests, max qued connections set 1,
	if( listen( server_fd, 3 ) < 0 ){
		perror("failed to listen\n");
		exit(3);//listen failed    
	}

	while(1)
	{// NEW CLIENT
		printf("-> waiting for new\n");
		struct sockaddr_in * new_addr = malloc(sizeof(struct sockaddr_in));
		addrlen = sizeof(*new_addr);
		int new_sock = accept( server_fd, (struct sockaddr *) new_addr, (socklen_t*)&addrlen );
		
		printf("-> rcvd_client %d\n", new_sock);
		if(new_sock<0){
			perror("error in accept\n");
			exit(4);
		}
		//lock 
		sem_wait(&sem);
		printf("-> checking_client\n");
		if(client_count<4){
			struct data_needed new_data;
			new_data.new_sock = new_sock;
			new_data.new_addr = new_addr;
			client_count++;
			pthread_t * new_thr = (pthread_t *)malloc(sizeof(pthread_t));
			pthread_create(new_thr,NULL,worker,(void *) &new_data);
		}
		else{
			close(new_sock);
			printf("-> REJECTED, max clients\n");
		}
		//unlock
		sem_post(&sem);
	}

	close(server_fd);
    exit(0);

}
