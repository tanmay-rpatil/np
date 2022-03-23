#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define MAXSIZE 1024





int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    char ip[20] = {0};
    char buffer[MAXSIZE] = {0};
    int port = 4444;
    int sock = 0;
    char read_buffer[1024] = {0};
	ssize_t read_return;

    if (argc >=3)//ip,port
    {
        strcpy(ip,argv[1]);
        port = atoi(argv[2]);
    }
    else if (argc==2)//,ip
    {
        strcpy(ip,argv[1]);            
    }
    else//def ip
    {
        strcpy(ip,"127.0.0.1");
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit (1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);    
    //convert IP addr binary
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) 
    {
        printf("addess converesion error\n");
        exit (1);
    }
    int x = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if ( x < 0)
    {
        perror("Server Unreachable");
        exit(1);
    }
    // printf("conn:  %d",x);
	while(1)
	{
		//send buffer
		memset(buffer, 0, sizeof(buffer));//flush
		printf("C: ");
		scanf("%[^\n]%*c", buffer);
		if ( (strlen(buffer)==strlen("exit")) && (strncmp(buffer,"exit",strlen("exit"))==0))
		{
			printf("# exiting\n");
			break;
		}
		if (send(sock, buffer, strlen(buffer), 0) < 0){
			perror("error in send:");
			exit(1);
		}

		//get str from server
		memset(read_buffer, 0, sizeof(read_buffer));//flush
		read_return = recv(sock, read_buffer, sizeof(read_buffer), 0);
		if(read_return<0){
			perror("error in reading\n");
			exit(1);
		}
        else if (read_return==0){
            printf("# server rejected/closed connection\n");
            break;
        }

		printf("S: ");
		for (int i = strlen(read_buffer)-1; i >=0; i--)
		{
			printf("%c",read_buffer[i]);
		}
		printf("\n");
   }
    close(sock);
    exit(0);
}
