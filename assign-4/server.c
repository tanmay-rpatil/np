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
//misc/threads
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
//files
#include <sys/stat.h>
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
	ssize_t cnt;
	ssize_t read_return;
	char buffer[MAXSIZE] = {0};
	unsigned char read_buffer[MAXSIZE] = {0};
	struct stat st;
	long int req_loops;
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
			exit(1);
		}
		else if(!read_return){	
			printf("\t-> exiting, from tid #%d\n",tid);
			break;
		}
		
		printf("\tC, tid #%d: ", tid);
		printf("%s\n",buffer);
		//open file
		FILE *fp = fopen(buffer,"rb"); //IMP DOUBT 
		if(fp){//exists
			stat(buffer, &st);
			printf("[info] filesize: %ld\n",st.st_size);
			req_loops = st.st_size;
			//send req loops
			sprintf(buffer,"%ld",req_loops);
			if (send(new_sock, buffer, sizeof(buffer), 0) < 0){
				perror("[error] in send");
				exit(1);
			}
			// printf("")
			//send the data
			for (int i = 0; i < req_loops; i++)
			{
				//read upto 1024B
				cnt = fread(read_buffer, sizeof(unsigned char), 1, fp);
				if(cnt){
					if (send(new_sock, read_buffer, cnt, 0) < 0){
						perror("[error] in send");
						exit(1);
					}
					printf("[info] sent %d/%ld chunks\n",i+1,req_loops);
				}
				else if(ferror(fp)){
					//error in reading!
					perror("[error] in reading file");
					exit(EXIT_FAILURE);
				}
			}
			fclose(fp);
		}

	}
	//lock
	sem_wait(&sem);
	close (new_sock);
	client_count--;
	//unlock
	sem_post(&sem);
	pthread_exit(NULL);
}

int net_int(char *host, struct ifaddrs *ifap, int select, int idx){
	//returns num of interfaces if select == 0
	//returns 0 for successful selection. -1 if error in selection
	//returnss -1 for general error
	struct ifaddrs *ll_node = ifap;
	int i = 0, family, s, num=-1;
	while( (ll_node) != NULL ){
		family = ll_node->ifa_addr->sa_family;

		if (family == AF_INET) {
			if(select==0){ //list
				printf("[info] Interface no. %d : %s", i, ll_node->ifa_name);
				s = getnameinfo(ll_node->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
					printf("[error] getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				printf("\taddress: %s\n", host);				
			}
			else if( select == 1 && i == idx ){
				printf("[info] selecting the interface  no. %d : %s\n", i, ll_node->ifa_name );
				s = getnameinfo(ll_node->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
					printf("[error] getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				printf("\taddress: %s\n", host);
				num = idx;
				break;
			}
			i++;
			num = i;
		}
		ll_node = ll_node->ifa_next;
	}
	if(select == 0){
		return num;
	}
	else if(select == 1 && num == idx ){//valid interface
		return 0;
	}
	else //select == 1 and num!=idx
	{
		printf("[error] failed to select. Please re-enter a valid interface number\n");
		return -1; //failed to select an interface!
	}
	
}

int main(int argc, char const *argv[])
{
    client_count = 0;
	struct sockaddr_in address;
	int addrlen,int_num, status;
	sem_init(&sem,0,1);
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
	//list interfaces
	net_int(host,ifap,0,0);
	status = -1;
	while(status==-1)
	{
		printf("[action] please type the interface number you wish to use: ");
		scanf("%d",&int_num);
		status = net_int(host,ifap,1,int_num);
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
		printf("-> rcvd_client, client fd: %d\n", new_sock);

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