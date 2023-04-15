#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>

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
int is_digit_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0; // not a digit string
        }
    }
    return 1; // digit string
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

int is_date2_greater(char* date1, char* date2) {
    int day1, month1, year1, day2, month2, year2;
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);

    if (year1 > year2) {
        return 0; // date1 is greater
    } else if (year1 == year2 && month1 > month2) {
        return 0; // date1 is greater
    } else if (year1 == year2 && month1 == month2 && day1 > day2) {
        return 0; // date1 is greater
    } else {
        return 1; // date2 is greater
    }
}
int validate(){
	if (strcmp(commands[0], "findfile") == 0 ||
		strcmp(commands[0], "sgetfiles") == 0 ||
		strcmp(commands[0], "dgetfiles") == 0 ||
		strcmp(commands[0], "getfiles") == 0 ||
		strcmp(commands[0], "gettargz") == 0 ||
		strcmp(commands[0], "quit") == 0)
	{
		if (strcmp(commands[0], "findfile") == 0)
		{
			if (num_args != 2)
			{
				printf("\n<usage>: findfile <filename>\n\n");
				return 0;
			}
			return 1;
		}
		else if (strcmp(commands[0], "sgetfiles") == 0)
		{
			if (num_args < 3 || num_args > 4)
			{
				printf("\n<usage>: sgetfiles <size1> <size2> <-u>\n\n");
				return 0;
			}
			else if (is_digit_string(commands[1]) == 0 || is_digit_string(commands[2]) == 0)
			{
				printf("\n<usage>: sgetfiles <size1> <size2> <-u>\n");
				printf("Size should be integer\n\n");
				return 0;
			}
			else if (atoi(commands[1]) >= atoi(commands[2]))
			{
				printf("\n<usage>: sgetfiles <size1> <size2> <-u>\n");
				printf("Size1 should be less than size2\n\n");
				return 0;
			}else if (num_args==4 && strcmp(commands[num_args-1], "-u")!=0)
			{
				printf("\n<usage>: sgetfiles <size1> <size2> <-u>\n\n");
				return 0;
			}
			return 1;
		}
		else if (strcmp(commands[0], "dgetfiles") == 0)
		{
			if (num_args < 3 || num_args > 4)
			{
				printf("\n<usage>: dgetfiles <date1> <date2> <-u>\n\n");
				return 0;
			}
			else if (isValidDate(commands[1]) == 0 || isValidDate(commands[2]) == 0)
			{
				printf("\n<usage>: dgetfiles <date1> <date2> <-u>\n");
				printf("Enter valid date\n\n");
				return 0;
			}
			else if(is_date2_greater(commands[1],commands[2])==0){
				printf("\n<usage>: dgetfiles <date1> <date2> <-u>\n");
				printf("Enter valid date with date1 should be older than date2\n\n");
				return 0;
			}else if (num_args==4 && strcmp(commands[num_args-1], "-u")!=0)
			{
				printf("\n<usage>: sgetfiles <size1> <size2> <-u>\n\n");
				return 0;
			}
			return 1;
		}
		else if (strcmp(commands[0], "getfiles") == 0)
		{
			if (num_args < 2 || num_args > 8)
			{
				printf("\n<usage>: getfiles <file1> <file2> <file3> <file4> <file5> <file6> <-u>\n\n");
				return 0;
			}else if (num_args==8 && strcmp(commands[num_args-1], "-u")!=0)
			{
				printf("\n<usage>: getfiles <file1> <file2> <file3> <file4> <file5> <file6> <-u>\n\n");
				return 0;
			}
			return 1;
		}
		else if (strcmp(commands[0], "gettargz") == 0)
		{
			if (num_args < 2 || num_args > 8)
			{
				printf("\n<usage>: gettargz <extension1> <extension2> <extension3> <extension4> <extension5> <extension6> <-u>\n\n");
				return 0;
			}else if (num_args==8 && strcmp(commands[num_args-1], "-u")!=0)
			{
				printf("\n<usage>: gettargz <extension1> <extension2> <extension3> <extension4> <extension5> <extension6> <-u>\n\n");
				return 0;
			}
			return 1;
		}
		return 1;
	}
	else
	{
		printf("Invalid command\n");
		return 0;
	}
}
void getTarFile(int clientSocket,char *fileName){
	char read_buffer[BUFFER_SIZE];
	off_t file_size;
	off_t total_bytes_read=0;
	off_t buffer_size;
	char read_buffer_for_Flag[BUFFER_SIZE];

	// Read dataFlag from socket
	printf("Waiting for dataFlag...\n");
	recv(clientSocket, &read_buffer_for_Flag, sizeof(read_buffer_for_Flag), 0);
	char receivedFlag[10];
	strncpy(receivedFlag, read_buffer_for_Flag, 10);
	printf("Received the Flag!\n");
	// wait for 1 second
	sleep(1);
	// send acknowledgement to server
	send(clientSocket, "flagReceived", 12, 0);

	char fileTypeFlag[10] = "SENDFILE=1";
	char dataFlag[10] = "SENDFILE=0";

	// if dataFlag SENDFILE=0
	if (strncmp(receivedFlag, dataFlag, 10)==0)
	{
		// read data from socket and print
		recv(clientSocket, &read_buffer, sizeof(read_buffer), 0);
		printf(">>%s\n",read_buffer);
		return;
	}
	else if (strncmp(receivedFlag, fileTypeFlag, 10)==0)
	{
		int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (fd == -1) {
			printf("Error opening file...\n");
			close(fd);
		}
		else
		{
			// Read file size from socket
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
				remove(fileName);
			}
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
		// testing : set buffer to quit to exit
		//strcpy(buffer, "getfiles check.txt");

		strcpy(str, buffer);
		split_string(str);

		// command validation
		int isValid = validate();
		if(isValid==1){
			send(clientSocket, buffer, strlen(buffer), 0);
		}
		else
		{
			continue;
		}

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
		//printf("\n++++++++++++++++++++++\n");
	}
	return 0;
}