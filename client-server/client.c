#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 7890
#define MESSAGE_SIZE 50

int main(){

	int sockfd;

	struct sockaddr_in server_address;

	if((sockfd = socket(PF_INET,SOCK_STREAM,0)) == -1){
		fprintf(stderr,"could not create socket\n");
		return -1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if(connect(sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not connect to server\n");
		return -1;
	}

	char server_message[MESSAGE_SIZE];
	memset(server_message,'\0',MESSAGE_SIZE);

	recv(sockfd,server_message,MESSAGE_SIZE,0);

	printf("%s",server_message);

	char message[] = "Hello from client\n";

	if(send(sockfd,message,strlen(message),0) <= 0){
		fprintf(stderr,"could not send message\n");
		return -1;
	}

	close(sockfd);

	return 0;
}
