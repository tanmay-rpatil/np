#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

//ip addr
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
//memset
#include <string.h> 

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

//for tid,ssize_t
#include <sys/types.h>
#include <sys/syscall.h>

#define MAXSIZE 1024
#define MAXCONN 5

int server_fd, client_count;
sem_t sem, sem_done;
char *err = "500";
char *ok  = "200";
struct data_needed{//data to pass to the workers
	int new_sock;
	struct sockaddr_in * new_addr;
};

void intHandler(int dummy) {
    // close(new_sock);
	printf("-> Received  SIGINT, closing socket\n");
	close(server_fd);
	exit(0);
}

void * worker(void *args){
	struct data_needed new_data = *(struct data_needed *)args;
	int new_sock = new_data.new_sock;
	ssize_t read_return;
	char buffer[MAXSIZE] = {0};
	pid_t tid = syscall(SYS_gettid);
	
	strcpy(buffer,ok);//send ok msg to start transmission
	if (send(new_sock, buffer, strlen(buffer), 0) < 0){
		perror("\t-> error in send");
		exit(1);
	}
	while(1)
	{
		//Get data from client
		memset(buffer, 0, sizeof(buffer));//flush
		printf("\t-> waiting for msg from tid #%d\n",tid);
		read_return = recv( new_sock ,(void *) buffer, sizeof(buffer), 0 );
		if(read_return<0){
			perror("\t-> error in reading\n");
			exit(5);
		}
		else if(!read_return){	
			printf("\t-> exiting, from tid #%d\n",tid);
			break;
		}
		
		printf("\tC, tid #%d: ", tid);
		printf("%s\n",buffer);
	}
	//lock
	sem_wait(&sem);
	close (new_sock);
	client_count--;
	//unlock
	sem_post(&sem);
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    client_count = 0;
	struct sockaddr_in address;
	int addrlen,family,s;
	sem_init(&sem,0,1);
	struct hostent *hostnet_str;
	struct in_addr localaddr;
	struct ifaddrs *ifap; // for network interfaces
	int PORT = 4444;
	char host[NI_MAXHOST];

	if (argc >=2)//port
    {
        PORT = atoi(argv[1]);
    }

	//close sockets when ctrl+c
	signal(SIGINT, intHandler);

	//get available interfaces
	if (getifaddrs(&ifap)<0){
		perror("[error] in getting network interfaces");
		exit(EXIT_FAILURE);
	}

	struct ifaddrs *ll_node = ifap;
	struct ifaddrs *old_node;
	while( (ll_node) != NULL ){
		family = ll_node->ifa_addr->sa_family;

		if (family == AF_INET) {
			printf("[info] Interface name: %s\n",ll_node->ifa_name);
			s = getnameinfo(ll_node->ifa_addr,
					sizeof(struct sockaddr_in),
					host, NI_MAXHOST,
					NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}

			printf("\t\taddress: <%s>\n", host);
			if( strcmp(ll_node->ifa_name, "lo") == 0 ){
				printf("[info] ignoring loopback\n");
			}
			else{
				break;
			}
		}
		// old_node = ll_node;
		ll_node = ll_node->ifa_next;
	}


	//create socket with options: domain: AF_INET for ipv4, type: SOCK_STREAM for TCP, protocol: 0 for IP.
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd<0){
		perror("-> failed to create socket\n");
		exit(1);
	}
	printf("[info] using the ip: %s\n", host);
	address.sin_family = AF_INET; //IPv4
	if(inet_pton(AF_INET, host, &address.sin_addr) <= 0) 
    {
        printf("addess converesion error\n");
        exit (1);
    }
	address.sin_port = htons( PORT ); //set port to 4444
	inet_ntop(AF_INET, &(address.sin_addr), host, INET_ADDRSTRLEN);
	printf("[info] attempt to bind to: %s\n", host);
	//bind to a port and ip
	if( bind( server_fd, (struct sockaddr *) &address, sizeof(address) ) < 0 ){
		perror("-> failed to bind\n");
		exit(2);//bind failed
	}
	printf("[success] bound to (ip:port) %s:%d\n",host,PORT);


	//listen at a socket ->  accept connection requests, max qued connections set 1,
	if( listen( server_fd, 3 ) < 0 ){
		perror("-> failed to listen\n");
		exit(3);//listen failed    
	}

	while(1)
	{// NEW CLIENT
		printf("-> waiting for new client\n");
		struct sockaddr_in * new_addr = malloc(sizeof(struct sockaddr_in));
		addrlen = sizeof(*new_addr);
		int new_sock = accept( server_fd, (struct sockaddr *) new_addr, (socklen_t*)&addrlen );
		
		if(new_sock<0){
			perror("-> error in accept\n");
			exit(4);
		}
		printf("-> rcvd_client %d\n", new_sock);

		//lock 
		sem_wait(&sem);
		printf("-> checking if lim of %d reached, current count is %d\n",MAXCONN,client_count);
		if(client_count<MAXCONN){
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
