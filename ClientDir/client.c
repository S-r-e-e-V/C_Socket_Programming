#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 4444
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define BUFFER_SIZE 1024

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

int isValidDate(char* str) {
    int day, month, year;
    if (sscanf(str, "%d-%d-%d", &year, &month, &day) != 3) {
        return 0;
    }
    if (day < 1 || month < 1 || month > 12 || year < 0) {
        return 0;
    }
    int maxDays = 31;
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        maxDays = 30;
    } else if (month == 2) {
        if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
            maxDays = 29;
        } else {
            maxDays = 28;
        }
    }
    if (day > maxDays) {
        return 0;
    }
    return 1;
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

// int validate(){
// 	if(strcmp(commands[0],"findfile")!=0 || 
// 		strcmp(commands[0],"sgetfiles")!=0||
// 		strcmp(commands[0],"dgetfiles")!=0 || 
// 		strcmp(commands[0],"getfiles")!=0 || 
// 		strcmp(commands[0],"gettargz")!=0){
// 			printf("Invalid command\n");
// 			return 0;
// 	}
// 	else if (strcmp(commands[0],"findfile")==0){
// 		if(num_args!=2){
// 			printf("<usage>: findfile <fileName>\n");
// 			return 0;
// 		}
// 		return 1;
// 	}else if(strcmp(commands[0],"sgetfiles")==0){
// 		if(num_args<3 || num_args>4){
// 			printf("<usage>: sgetfiles <size1> <size2> -u\n");
// 			return 0;
// 		}else if(!isdigit(commands[1])||!isdigit(commands[2])){
// 			printf("<usage>: sgetfiles <size1> <size2> -u\n");
// 			printf("Size should be integer\n");
// 			return 0;
// 		}else if(atoi(commands[1])>atoi(commands[2])){
// 			printf("<usage>: sgetfiles <size1> <size2> -u\n");
// 			printf("Size1 should be less than size2\n");
// 			return 0;
// 		}
// 		return 1;
// 	}else if(strcmp(commands[0],"dgetfiles")==0){
// 		if(num_args<3 || num_args>4){
// 			printf("<usage>: dgetfiles <date1> <date2> -u\n");
// 			return 0;
// 		}else if(isValidDate(commands[1])==0||isValidDate(commands[2])==0){
// 			printf("<usage>: dgetfiles <date1> <date2> -u\n");
// 			printf("Enter valid date\n");
// 			return 0;
// 		}else if(atoi(commands[1])>atoi(commands[2])){
// 			printf("<usage>: dgetfiles <date1> <date2> -u\n");
// 			printf("Date1 should be less than date2\n");
// 			return 0;
// 		}
// 		return 1;
// 	}
// 	return 1;
// }

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[MAX_COMMAND_LENGTH];
	char str[MAX_COMMAND_LENGTH];
	char read_buffer[BUFFER_SIZE];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		printf("C$ ");
		scanf(" %[^\n]s",&buffer[0]);
		strcpy(str, buffer);
		split_string(str);

		// int isValid = validate();

		// if(isValid==1){
		// 	send(clientSocket, buffer, strlen(buffer), 0);
		// }else{
		// 	continue;
		// }
		send(clientSocket, buffer, strlen(buffer), 0);
		

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