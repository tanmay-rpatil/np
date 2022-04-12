#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define MAXSIZE 1024

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    char ip[20] = {0};
    char buffer[MAXSIZE] = {0};
    unsigned char read_buffer[MAXSIZE] = {0};
    int port = 4444;
    int sock = 0;
    long int req_loops;

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
        printf("# addess converesion error\n");
        exit (1);
    }
    int x = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if ( x < 0)
    {
        perror("# Server Unreachable");
        exit(1);
    }

    ssize_t read_return = recv( sock ,(void *) buffer, sizeof(buffer), 0 );
    if(read_return<0){
        perror("# error in reading\n");
        close(sock);
        exit(1);
    }
    else if (read_return == 0)
    {
        printf("# Server refused connection\n");
        close(sock);
        exit(0);
    }
    
    //connection est, keep sending data untill user types "quit"
    printf("# Connection established! type msg after the prompt 'C>' \n");
	while(1)
	{
		//send buffer
		memset(buffer, 0, sizeof(buffer));//flush
		printf("C>");
		scanf("%[^\n]%*c", buffer);
        printf("Scanned: %s\n",buffer);
		if ( (strlen(buffer)==strlen("quit")) && (strncmp(buffer,"quit",strlen("quit"))==0))
		{
			printf("# exiting\n");
			break;
		}
		if (send(sock, buffer, strlen(buffer), 0) < 0){
			perror("# error in send");
			exit(1);
		}
        FILE *fp = fopen(buffer,"wb"); //IMP DOUBT
        if(!fp){
            perror("Error in opening file");
            exit(1);
        } 
        memset(buffer, 0, sizeof(buffer));//flush
        read_return = recv( sock ,(void *) buffer, sizeof(buffer), 0 );//get num loops
		if(read_return<=0){
			perror("\t# error in reading\n");
			exit(1);
		}
        req_loops = atoi(buffer);
        printf("Reading file in %s chunks\n",buffer);
        for (int i = 0; i < req_loops; i++)
        {
            // memset(read_buffer, 0, sizeof(read_buffer));//flush
            read_return = recv( sock ,(void *) read_buffer, 1, 0 );//get num loops
            if(read_return<=0){
                perror("\t# error in reading\n");
                exit(1);
            }
            fwrite(read_buffer,sizeof(unsigned char), 1,fp);
            printf("%d/%ld chunk:\n%s\n",i+1,req_loops,read_buffer);
        }
        fclose(fp);

   }
    close(sock);
    exit(0);
}
