#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PRIMARY_SERVER_PORT 4444
#define MIRROR_SERVER_PORT 8888
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define BUFFER_SIZE 1024

char *PRIMARY_SERVER_IP = "127.0.0.1";
char *MIRROR_SERVER_IP = "127.0.0.1";

char *commands[MAX_ARGS];
int num_args = 0;

void split_string(char *str) {
	num_args = 0;
	char *token = strtok(str, " ");
	while (token != NULL) {
        commands[num_args] = token;
        token = strtok(NULL, " ");
        num_args++;
    }
}

void isEmptyTar(char * fileName){
	FILE* tarfile = fopen(fileName, "rb");
    if (tarfile != NULL) {
        int c = fgetc(tarfile);
        if (c == EOF) {
            printf("The tar file is empty.\n");
        } else {
            printf("The tar file is not empty.\n");
        }
        fclose(tarfile);
    } else {
        printf("Failed to open the tar file.\n");
        perror("fopen");
    }
}

void getTarFile(int clientSocket,char *fileName){
	char read_buffer[BUFFER_SIZE];
	off_t file_size;
	off_t total_bytes_read=0;
	off_t buffer_size;
	int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error opening file...\n");
		close(fd);
	}
	else
	{
		recv(clientSocket, &buffer_size, sizeof(buffer_size), 0);

		// Read data from socket and write to file
		while (total_bytes_read<buffer_size)
		{
			ssize_t bytes_read = recv(clientSocket, read_buffer, BUFFER_SIZE, 0);
			if(bytes_read==-1){
				perror("Error in receiving data");
				break;
			}
			ssize_t bytes_written = write(fd, read_buffer, bytes_read);
			if(bytes_written==-1){
				perror("Error in writing data");
				break;
			}
			if (bytes_written < bytes_read) {
				printf("Error writing data to file...\n");
				break;
			}
			total_bytes_read =total_bytes_read+ bytes_read;
		}
		close(fd);

		// isEmptyTar(fileName);

		if (strcmp(commands[num_args-1], "-u") == 0)
		{
			char str[200];
			sprintf(str, "tar -xzf %s",fileName);
			system(str);
		}
	}
}

int main(){

	int clientSocket, ret;
	struct sockaddr_in primaryServerAddr;
	struct sockaddr_in mirrorServerAddr;
	char buffer[MAX_COMMAND_LENGTH];
	char str[MAX_COMMAND_LENGTH];
	char read_buffer[BUFFER_SIZE];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");


		// Try connecting to Primary Server
	memset(&primaryServerAddr, '\0', sizeof(primaryServerAddr));
	primaryServerAddr.sin_family = AF_INET;
	primaryServerAddr.sin_port = htons(PRIMARY_SERVER_PORT);
	primaryServerAddr.sin_addr.s_addr = inet_addr(PRIMARY_SERVER_IP);

	ret = connect(clientSocket, (struct sockaddr*)&primaryServerAddr, sizeof(primaryServerAddr));
	if(ret < 0){
		printf("[-]Error in connection with PRIMARY SERVER\n");
		exit(1);
	}

	// Check if Primary Server has responded to connect with MIRROR SERVER or PRIMARY SERVER
	write(clientSocket, "c", 1);
	int n = read(clientSocket, read_buffer, 1);
    read_buffer[n] = '\0';
	// check if the server responded with P or M in first char of read_buffer
	if(strcmp(read_buffer,"P") == 0){
		printf("[+]Connected to PRIMARY SERVER.\n");
	}
	else if (strcmp(read_buffer,"M")==0)
	{
		close(clientSocket);
		clientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(clientSocket < 0){
			printf("[-]Error in connection.\n");
			exit(1);
		}
		printf("[+]Client Socket is created.\n");
		memset(&mirrorServerAddr, '\0', sizeof(mirrorServerAddr));
		mirrorServerAddr.sin_family = AF_INET;
		mirrorServerAddr.sin_port = htons(MIRROR_SERVER_PORT);
		mirrorServerAddr.sin_addr.s_addr = inet_addr(MIRROR_SERVER_IP);
		ret = connect(clientSocket, (struct sockaddr*)&mirrorServerAddr, sizeof(mirrorServerAddr));
		if(ret < 0){
			printf("[-]Error in connection with MIRROR SERVER\n");
			exit(1);
		}
		printf("[+]Connected to MIRROR SERVER.\n");
	}
	else
	{
		printf("[-]PRIMARY SERVER malfunction: Responded with unknown command\n");
		exit(1);
	}

	while(1){
		printf("C$ ");
		scanf(" %[^\n]s",&buffer[0]);
		strcpy(str, buffer);
		split_string(str);

		// char command[MAX_COMMAND_LENGTH]="";
        // strcpy(command,buffer[0]);
        // char *arg = strtok(command, " ");

		// if(!strcmp(command,"findfile") || 
		// !strcmp(command,"sgetfiles")||
		// !strcmp(command,"dgetfiles") || 
		// !strcmp(command,"getfiles") || 
		// !strcmp(command,"gettargz")){
		// 	printf("Incorrect command");
		// }
		// else
		// {
		send(clientSocket, buffer, strlen(buffer), 0);
		// }

		if(strcmp(buffer, "quit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		if(strcmp(commands[0],"sgetfiles")==0){
			getTarFile(clientSocket, "sfiles.tar.gz");
		}else if(strcmp(commands[0],"dgetfiles")==0){
			getTarFile(clientSocket, "dfiles.tar.gz");
		}else if(strcmp(commands[0],"getfiles")==0){
			getTarFile(clientSocket, "listOfFiles.tar.gz");
		}else if(strcmp(commands[0],"gettargz")==0){
			getTarFile(clientSocket, "gettargz.tar.gz");
		}else{
			if(recv(clientSocket, read_buffer, BUFFER_SIZE, 0) < 0){
				printf("[-]Error in receiving data.\n");
			}else{
				printf("Server: %s\n", read_buffer);
			}
		}
	}
	return 0;
}