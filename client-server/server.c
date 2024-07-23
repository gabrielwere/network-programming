#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 7890
#define MAX_CONNECTIONS 1
#define MESSAGE_SIZE 50

int main(){

	int sockfd;
	int new_sockfd;

	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	if((sockfd = socket(PF_INET,SOCK_STREAM,0)) == -1){
			fprintf(stderr,"could not create socket\n");
			return -1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = 0;//use my ip

	if(bind(sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not bind socket\n");
		return -1;
	}

	if(listen(sockfd,MAX_CONNECTIONS) == -1){
		fprintf(stderr,"could not listen for incoming connections\n");
		return -1;
	}

	int len = sizeof(struct sockaddr_in);

	char message[] = "Hello from server\n";
	char client_message[MESSAGE_SIZE];

	if((new_sockfd = accept(sockfd,(struct sockaddr*)&client_address,&len)) > 0){

		printf("Received a connection from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));

		if(send(new_sockfd,message,strlen(message),0) <= 0){
			fprintf(stderr,"message was not sent\n");
			return -1;
		}

		memset(client_message,'\0',MESSAGE_SIZE);

		recv(new_sockfd,client_message,MESSAGE_SIZE,0);

		printf("%s",client_message);
	}

	close(new_sockfd);
	close(sockfd);
			

	return 0;
}
