#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "commands.h"
#include "codes.h"

#define FTP_COMMAND_PORT 21
#define RESPONSE_SIZE 1024
#define DATA_SIZE 512
#define FILE_NAME_SIZE 4096

void send_string(int sockfd,const unsigned char *buffer);
void recv_string(int sockfd,unsigned char *buffer);
int create_socket(int port);
int return_data_port(int sockfd,unsigned char *server_response);
int check_return_code(unsigned char *response,unsigned char *code);
char *receive_data(int data_socket,int data_size,int *total_bytes_received);
int enter_file(unsigned char line[],int max_size);

int main(int argc,char *argv[]){

	int sockfd;

	if((sockfd = create_socket(FTP_COMMAND_PORT)) == -1){
		fprintf(stderr,"command socket not created\n");
		return -1;
	}

	char server_response[RESPONSE_SIZE];

	recv_string(sockfd,server_response);

	if(check_return_code(server_response,SERVICE_READY) == -1){
		fprintf(stderr,"%s code not received\n",SERVICE_READY);
		return -1;
	}

	send_string(sockfd,ANONYMOUS_USERNAME);
	recv_string(sockfd,server_response);

	if(check_return_code(server_response,USERNAME_OK) == -1){
		fprintf(stderr,"%s code not received\n",USERNAME_OK);
		return -1;
	}

	send_string(sockfd,BLANK_PASSWORD);
	recv_string(sockfd,server_response);

	if(check_return_code(server_response,SUCCESSFUL_LOGIN) == -1){
		fprintf(stderr,"%s code not received\n",SUCCESSFUL_LOGIN);
		return -1;
	}
	
	int data_port = return_data_port(sockfd,server_response);
	
	int data_socket;

	if((data_socket = create_socket(data_port)) == -1){
		fprintf(stderr,"data socket not created\n");
		return -1;
	}

	send_string(sockfd,LIST_FILES);
	recv_string(sockfd,server_response);

	if(check_return_code(server_response,DATA_CONNECTION_OPEN) == -1){
		fprintf(stderr,"%s code not received\n",DATA_CONNECTION_OPEN);
		return -1;
	}

	int bytes_received;
	char *file_list = receive_data(data_socket,DATA_SIZE,&bytes_received);

	recv_string(sockfd,server_response);

	if(check_return_code(server_response,TRANSFER_COMPLETE) == -1){
		fprintf(stderr,"%s code not received\n",TRANSFER_COMPLETE);
		return -1;
	}

	printf("%s\n",file_list);

	send_string(sockfd,BINARY_TRANSFER);
	recv_string(sockfd,server_response);

	if(check_return_code(server_response,REQUEST_COMPLETED) == -1){
		fprintf(stderr,"%s code not received\n",REQUEST_COMPLETED);
		return -1;
	}

	char file_name[FILE_NAME_SIZE];

	while(enter_file(file_name,FILE_NAME_SIZE)){

		data_port = return_data_port(sockfd,server_response);
	
		if((data_socket = create_socket(data_port)) == -1){
			fprintf(stderr,"data socket not created\n");
			return -1;
		}

		char get_file[RESPONSE_SIZE + FILE_NAME_SIZE] = "RETR ";
		strncat(get_file,file_name,strlen(file_name));

		char eol[] = "\r\n";
		strncat(get_file,eol,strlen(eol));

		send_string(sockfd,get_file);

		recv_string(sockfd,server_response);


		if(check_return_code(server_response,DATA_CONNECTION_OPEN) > 0){

			char *file_data = receive_data(data_socket,DATA_SIZE,&bytes_received);
		
			recv_string(sockfd,server_response);
		
			if(check_return_code(server_response,TRANSFER_COMPLETE) == -1){
				fprintf(stderr,"file was not downloaded : %s\n",server_response);
				return -1;
			}

			printf("\n\n %s successfully retrieved \n\n",file_name);

			FILE *fh;
		
			if((fh = fopen(file_name,"w")) == NULL){
				fprintf(stderr,"could not create file\n");
				return -1;
			}

			char *ptr = file_data;

			while(bytes_received-- > 0){
				putc(*ptr,fh);
				ptr++;
			}

			fclose(fh);
			free(file_data);
		
		}else if(check_return_code(server_response,NO_SUCH_FILE) > 0)
			printf("%s file does not exist\n",file_name);
		else
			printf("%s\n",server_response);

		close(data_socket);

	}
	free(file_list);
	close(sockfd);

	return 0;
}

//send ALL the bytes of buffer via sockfd
void send_string(int sockfd,const unsigned char *buffer){

	int bytes_to_send = strlen(buffer);
	int bytes_sent = 0;

	while(bytes_to_send > 0){

		bytes_sent = send(sockfd,buffer,bytes_to_send,0);
		bytes_to_send -= bytes_sent;
		buffer += bytes_sent;

	}
}

//recv bytes into buffer via sockfd
void recv_string(int sockfd,unsigned char *buffer){

	//zero out buffer;
	memset(buffer,'\0',RESPONSE_SIZE);
	int bytes_received;

	if((bytes_received = recv(sockfd,buffer,RESPONSE_SIZE,0)) <= 0)
		fprintf(stderr,"not bytes were received\n");

}

int create_socket(int port){

	int sockfd;

	if((sockfd = socket(PF_INET,SOCK_STREAM,0)) == -1){
		fprintf(stderr,"could not create socket\n");
		return -1;
	}

	struct sockaddr_in ftp_server;

	ftp_server.sin_family = AF_INET;
	ftp_server.sin_port = htons(port);
	ftp_server.sin_addr.s_addr = INADDR_ANY;

	if(connect(sockfd,(struct sockaddr*)&ftp_server,sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr,"could not connect to ftp server\n");
		return -1;
	}

	return sockfd;
}

int return_data_port(int sockfd,unsigned char *server_response){


	send_string(sockfd,PASSIVE_MODE);
	recv_string(sockfd,server_response);

	if(check_return_code(server_response,ENTERING_PASSIVE_MODE) == -1){
		fprintf(stderr,"%s code not received\n",ENTERING_PASSIVE_MODE);
		return -1;
	}

	unsigned char *host_port = strstr(server_response,"(");

	int host1,host2,host3,host4,port1,port2;

	sscanf(host_port,"(%d,%d,%d,%d,%d,%d)",&host1,&host2,&host3,&host4,&port1,&port2);

	return (256 * port1) + port2;

}

//checks if the first 3 chars of response,match the chars in code
int check_return_code(unsigned char *response,unsigned char *code){

	if(strncmp(response,code,3) == 0)
		return 1;
	else
		return -1;
}

char *receive_data(int data_socket,int data_size,int *total_bytes_received){

	char *data;

	if((data = (char *)malloc(data_size)) == NULL){
		fprintf(stderr,"could not allocate space for data\n");
		return NULL;
	}

	int bytes_received = 0;
	int n;


	while((n = recv(data_socket,data + bytes_received,data_size - bytes_received,0)) > 0){

		bytes_received += n;
		if(bytes_received >= data_size){

			data_size *= 2;
			data = (char *)realloc(data,data_size);
		}

	}
	*total_bytes_received = bytes_received;

	return data;
}

int enter_file(unsigned char line[],int max_size){

	printf("Enter file to retrieve\n");
	int i,c;
	for(i = 0;i < (max_size - 1) && ((c = getchar()) != EOF) && c != '\n';i++)
		line[i] = c;

	line[i] = '\0';

	return i;
}
