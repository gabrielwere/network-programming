#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define FTP_COMMAND_PORT 21
#define RESPONSE_SIZE 1024

int main(int argc,char *argv[]){

	int sockfd;

	if((sockfd = socket(PF_INET,SOCK_STREAM,0)) == -1){
		fprintf(stderr,"could not create socket\n");
		return -1;
	}

	char *ip_addr = argv[1];
	struct sockaddr_in ftp_server;

	ftp_server.sin_family = AF_INET;
	ftp_server.sin_port = htons(FTP_COMMAND_PORT);
	ftp_server.sin_addr.s_addr = INADDR_ANY;

	if(connect(sockfd,(struct sockaddr*)&ftp_server,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not connect to ftp server\n");
		return -1;
	}

	char server_response[RESPONSE_SIZE];
	memset(server_response,'\0',RESPONSE_SIZE);

	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char user[] = "USER anonymous\r\n";

	send(sockfd,user,strlen(user),0);

	memset(server_response,'\0',RESPONSE_SIZE);
	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char password[] = "PASS \r\n";
	send(sockfd,password,strlen(password),0);

	memset(server_response,'\0',RESPONSE_SIZE);
	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char pasv[] = "PASV\r\n";

	send(sockfd,pasv,strlen(pasv),0);

	memset(server_response,'\0',RESPONSE_SIZE);
	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char *host_port = strstr(server_response,"(");

	printf("%s",host_port);
	int host1,host2,host3,host4,port1,port2;

	sscanf(host_port,"(%d,%d,%d,%d,%d,%d)",&host1,&host2,&host3,&host4,&port1,&port2);

	int data_port = (256 * port1) + port2;
	printf("%d\n",data_port);

	int data_socket = socket(PF_INET,SOCK_STREAM,0);
	if(data_socket == -1){
		fprintf(stderr,"could not create data socket\n");
		return -1;
	}

	ftp_server.sin_family = AF_INET;
	ftp_server.sin_port = htons(data_port);
	ftp_server.sin_addr.s_addr = INADDR_ANY;

	if(connect(data_socket,(struct sockaddr*)&ftp_server,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not connect to ftp data port\n");
		return -1;
	}


	char list[] = "LIST\r\n";
	send(sockfd,list,strlen(list),0);

	memset(server_response,'\0',RESPONSE_SIZE);
	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char *file_list;
	int file_size = 10;

	if((file_list = (char*)malloc(file_size)) == NULL){
		fprintf(stderr,"could not allocate memory for file list\n");
		return -1;
	}

	int bytes_received = 0;
	int n;

	while(n = recv(data_socket,file_list + bytes_received,(file_size - bytes_received),0)){

		bytes_received += n;
		if(bytes_received >= file_size){
			file_size *= 2;
			file_list = (char *)realloc(file_list,file_size);
		}
	}

	printf("%s\n",file_list);

	free(file_list);

	memset(server_response,'\0',RESPONSE_SIZE);

	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);


	char binary[] = "TYPE I\r\n";
	send(sockfd,binary,strlen(binary),0);

	memset(server_response,'\0',RESPONSE_SIZE);

	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	send(sockfd,pasv,strlen(pasv),0);

	memset(server_response,'\0',RESPONSE_SIZE);
	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	host_port = strstr(server_response,"(");

	printf("%s",host_port);

	sscanf(host_port,"(%d,%d,%d,%d,%d,%d)",&host1,&host2,&host3,&host4,&port1,&port2);

	data_port = (256 * port1) + port2;
	printf("%d\n",data_port);

	data_socket = socket(PF_INET,SOCK_STREAM,0);
	if(data_socket == -1){
		fprintf(stderr,"could not create data socket\n");
		return -1;
	}

	ftp_server.sin_family = AF_INET;
	ftp_server.sin_port = htons(data_port);
	ftp_server.sin_addr.s_addr = INADDR_ANY;

	if(connect(data_socket,(struct sockaddr*)&ftp_server,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not connect to ftp data port\n");
		return -1;
	}
	char get[] = "RETR image.jpeg\r\n";
	send(sockfd,get,strlen(get),0);

	memset(server_response,'\0',RESPONSE_SIZE);

	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	char *file;
	file_size = 10;

	if((file = (char*)malloc(file_size)) == NULL){
		fprintf(stderr,"could not allocate memory for file list\n");
		return -1;
	}

	bytes_received = 0;
	n;

	while(n = recv(data_socket,file + bytes_received,(file_size - bytes_received),0)){

		bytes_received += n;
		if(bytes_received >= file_size){
			file_size *= 2;
			file = (char *)realloc(file,file_size);
		}
	}

	memset(server_response,'\0',RESPONSE_SIZE);

	recv(sockfd,server_response,RESPONSE_SIZE,0);

	printf("%s",server_response);

	FILE *fh;

	if((fh = fopen("image.jpeg","w")) == NULL){
		fprintf(stderr,"could not create file\n");
		return -1;
	}

	while(bytes_received-- > 0){
		putc(*file,fh);
		file++;
	}



	close(data_socket);
	close(sockfd);

	return 0;
}
