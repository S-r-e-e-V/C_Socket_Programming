#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>

#define PORT 8888
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define BUFFER_SIZE 1024

int totalClients = 0;
// Home directory
char *home;

char *commands[MAX_ARGS];
int num_args = 0;

void split_string(char *str)
{
	num_args = 0;
	char *token = strtok(str, " ");
	while (token != NULL)
	{
		commands[num_args] = token;
		token = strtok(NULL, " ");
		num_args++;
	}
}

int search_file(int newSocket, char *filename, char *path)
{
	struct stat st;
	DIR *dir;
	struct dirent *entry;
	char fullpath[MAX_COMMAND_LENGTH];

	if (stat(path, &st) == -1)
	{
		// perror("stat");
		return 1;
	}

	if (S_ISDIR(st.st_mode))
	{
		if ((dir = opendir(path)) == NULL)
		{
			// perror("opendir");
			return 1;
		}

		while ((entry = readdir(dir)) != NULL)
		{
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}

			snprintf(fullpath, MAX_COMMAND_LENGTH, "%s/%s", path, entry->d_name);

			if (search_file(newSocket, filename, fullpath) == 0)
			{
				return 0;
			}
		}

		closedir(dir);
	}
	else
	{
		if (strcmp(filename, path + strlen(path) - strlen(filename)) == 0)
		{
			if (stat(path, &st) == -1)
			{
				// perror("stat");
				return 1;
			}

			printf("Found file %s\n", path);
			printf("Size: %jd bytes\n", (intmax_t)st.st_size);
			printf("Date created: %s", ctime(&st.st_ctime));

			char *message = malloc(MAX_COMMAND_LENGTH * sizeof(char));
			sprintf(message, "File: %s\nSize: %jd bytes\nDate created: %s", path, (intmax_t)st.st_size, ctime(&st.st_ctime));

			send(newSocket, message, strlen(message), 0);

			return 0;
		}
	}
	return 1;
}

// unused code

// char * findFile(){
// 	struct stat fileStat;
// 	char command[MAX_COMMAND_LENGTH];
//     char output[MAX_COMMAND_LENGTH];
// 	snprintf(command, sizeof(command), "find ~ -name \"%s\" -type f -print -exec ls -lh --time-style=+\"%%Y-%%m-%%d %%H:%%M:%%S\" {} \\; 2>/dev/null | head -n1", commands[1]);
//     FILE *fp = popen(command, "r");
//     if (fp == NULL) {
//         perror("popen failed");
//     }
//     if (fgets(output, sizeof(output), fp) != NULL) {
// 		stat("/Users/sreekanthv/Personal/Windsor/Sem_2/ASP/Lab/input.txt", &fileStat);
// 		printf( "File: %s\nSize: %ld bytes\nDate created: %s", output, fileStat.st_size, ctime(&fileStat.st_ctime));
// 		char *message=malloc(MAX_COMMAND_LENGTH * sizeof(char));
// 		sprintf(message, "File: %s\nSize: %ld bytes\nDate created: %s", output, fileStat.st_size, ctime(&fileStat.st_ctime));
// 		printf("%s", message);
// 		return message;
// 	}
// 	else
// 	{
// 		printf("File not found.\n");
// 	}
// 	pclose(fp);
// }

void getFiles(int newSocket, char *file)
{
	char command[MAX_COMMAND_LENGTH];
	char output[MAX_COMMAND_LENGTH];
	char filename[500];
	char files[500];
	int sendFile = 0;

	if (strcmp(file, "sfiles") == 0)
	{
		// find Command
		char *findCommand = malloc(MAX_COMMAND_LENGTH * sizeof(char));
		sprintf(findCommand, "find %s -type f -name '*.*' -not -path '%s/Library/*' -size +%dc -size -%dc -print0 ", home, home, atoi(commands[1]), atoi(commands[2]));
		// complete command with tar
		sprintf(command, "%s | if grep -q . ; then %s | tar -czf %s_%d.tar.gz --null -T -  ; fi", findCommand, findCommand, file, newSocket);
	}
	else if (strcmp(file, "dfiles") == 0)
	{
		// find Command
		char *findCommand = malloc(MAX_COMMAND_LENGTH * sizeof(char));
		sprintf(findCommand, "find %s -type f -name '*.*' -not -path '%s/Library/*' -newermt '%s 00:00:00' ! -newermt '%s 23:59:59' -print0", home, home, commands[1], commands[2]);
		sprintf(command, "%s | if grep -q . ; then %s | tar -czf %s_%d.tar.gz --null -T - ; fi", findCommand, findCommand, file, newSocket);
	}
	else if (strcmp(file, "getfiles") == 0)
	{
		sprintf(command, "find %s -type f '('", home);
		if (strcmp(commands[num_args - 1], "-u") == 0)
		{
			for (int i = 1; i < num_args - 1; i++)
			{
				if (i != num_args - 2)
				{
					sprintf(files, " -name '%s' -o", commands[i]);
				}
				else
				{
					sprintf(files, " -name '%s'", commands[i]);
				}
				strcat(command, files);
			}
		}
		else
		{
			for (int i = 1; i < num_args; i++)
			{
				if (i != num_args - 1)
				{
					sprintf(files, " -name '%s' -o", commands[i]);
				}
				else
				{
					sprintf(files, " -name '%s'", commands[i]);
				}
				strcat(command, files);
			}
		}
		sprintf(files, " ')' -print0 ");
		strcat(command, files);
		sprintf(files, " | if grep -q . ; then %s |", command);
		strcat(command, files);
		sprintf(files, " tar -czf %s_%d.tar.gz --null -T - ; fi", file, newSocket);
		strcat(command, files);
	}
	else if (strcmp(file, "gettargz") == 0)
	{
		sprintf(command, "find %s -type f '('", home);
		if (strcmp(commands[num_args - 1], "-u") == 0)
		{
			for (int i = 1; i < num_args - 1; i++)
			{
				if (i != num_args - 2)
				{
					sprintf(files, " -name '*.%s' -o", commands[i]);
				}
				else
				{
					sprintf(files, " -name '*.%s'", commands[i]);
				}
				strcat(command, files);
			}
		}
		else
		{
			for (int i = 1; i < num_args; i++)
			{
				if (i != num_args - 1)
				{
					sprintf(files, " -name '*.%s' -o", commands[i]);
				}
				else
				{
					sprintf(files, " -name '*.%s'", commands[i]);
				}
				strcat(command, files);
			}
		}
		sprintf(files, " ')' -print0 ");
		strcat(command, files);
		sprintf(files, " | if grep -q . ; then %s |", command);
		strcat(command, files);
		sprintf(files, "tar -czf %s_%d.tar.gz --null -T - ; fi", file, newSocket);
		strcat(command, files);
	}
	printf("%s", command);
	// Execute find command
	int status = system(command);
	if (status != 0)
	{
		printf("\nFailed to run command...\n");
		exit(0);
	}
	else
	{
		printf("\nCommand successful...\n");
	}

	// copy file name
	sprintf(filename, "%s_%d.tar.gz", file, newSocket);

	// check if file exists
	if (access(filename, F_OK) != -1)
	{
		sendFile = 1;
	}
	else
	{
		sendFile = 0;
	}

	if (sendFile)
	{
		// send the sendFile flag to client like SENDFILE=0
		char flag[10] = "SENDFILE=1";
		while (1)
		{
			printf("\nSending flag: %s\n", flag);
			send(newSocket, flag, strlen(flag), 0);
			// wait for acknowledgement
			char ack[12];
			printf("Waiting for acknowledgement...\n");
			recv(newSocket, ack, 12, 0);
			printf("Acknowledgement received: %s\n", ack);
			if (strncmp(ack, "flagReceived", 12) == 0)
			{
				break;
			}
		}

		// Open temp.tar.gz archive for reading
		int fd = open(filename, O_RDONLY);
		if (fd == -1)
		{
			printf("Error opening archive...\n");
			exit(0);
		}

		// send file size
		struct stat st;
		if (stat(filename, &st) >= 0)
		{
			off_t buffer_size = st.st_size;
			send(newSocket, &buffer_size, sizeof(buffer_size), 0);
		}
		else
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}

		// Read archive into buffer and send to client over socket
		char buffer[BUFFER_SIZE];

		ssize_t bytes_read;
		while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0)
		{
			ssize_t bytes_sent = send(newSocket, buffer, bytes_read, 0);
			if (bytes_sent < bytes_read)
			{
				printf("Error sending data...\n");
				exit(0);
			}
		}

		// remove tar after sending it to client
		remove(filename);

		// Close file and socket
		close(fd);
		printf("File sent successfully...\n");
	}
	else
	{
		// send the sendFile flag to client like SENDFILE=0
		char flag[10] = "SENDFILE=0";
		while (1)
		{
			printf("\nSending flag: %s\n", flag);
			send(newSocket, flag, strlen(flag), 0);
			// wait for acknowledgement
			char ack[12];
			printf("Waiting for acknowledgement...\n");
			recv(newSocket, ack, 12, 0);
			printf("Acknowledgement received: %s\n", ack);
			if (strncmp(ack, "flagReceived", 12) == 0)
			{
				break;
			}
		}

		// send the error message to client
		char *error = "Error: File not found";
		send(newSocket, error, strlen(error), 0);
		printf("Message sent successfully...\n");
	}
}


int main()
{
	home = getenv("HOME");
	int sockfd, ret;
	int opt = 1;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Mirror Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt failed");
		exit(1);
	}

	ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (ret < 0)
	{
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", PORT);

	if (listen(sockfd, 10) == 0)
	{
		printf("[+]Listening....\n");
	}
	else
	{
		printf("[-]Error in binding.\n");
	}

	while (1)
	{
		newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
		if (newSocket < 0)
		{
			exit(1);
		}
		printf("%d. Connection accepted from %s:%d\n", totalClients + 1, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		totalClients++;

		pid_t childPid = fork();
		if (childPid == 0)
		{
			close(sockfd);
			while (1)
			{
				// clear the buffer
				memset(buffer, '\0', sizeof(buffer));
				recv(newSocket, buffer, BUFFER_SIZE, 0);
				if (strlen(buffer) == 0)
				{
					continue;
				}
				printf("\nClient request: %s\n", buffer);

				if (strcmp(buffer, "quit") == 0)
				{
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					close(sockfd);
					exit(0);
					break;
				}
				else
				{
					split_string(buffer);

					char *cmd = malloc(strlen(commands[0]) + 1); // +1 for the null terminator
					strcpy(cmd, commands[0]);
					cmd[strlen(cmd)] = '\0';
					char *message = malloc(MAX_COMMAND_LENGTH * sizeof(char));
					if (strcmp(cmd, "findfile") == 0)
					{
						// strcpy(message, findFile());
						if (search_file(newSocket, commands[1], home) != 0)
						{
							message = "No files found";
							send(newSocket, message, strlen(message), 0);
						}
					}
					else if (strcmp(cmd, "sgetfiles") == 0)
					{
						getFiles(newSocket, "sfiles");
					}
					else if (strcmp(cmd, "dgetfiles") == 0)
					{
						getFiles(newSocket, "dfiles");
					}
					else if (strcmp(cmd, "getfiles") == 0)
					{
						getFiles(newSocket, "getfiles");
					}
					else if (strcmp(cmd, "gettargz") == 0)
					{
						getFiles(newSocket, "gettargz");
					}
					memset(message, 0, sizeof(char) * sizeof(message));
				}
			}
		}
	}
	close(newSocket);
	return 0;
}