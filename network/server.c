#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "common.h"
//#include "utils.h"
//#include "fs.h"

char buff[200];

char* read_result() {
	//system("cat /dev/miniFSDevice");
	
	FILE* fp;
	fp = fopen("/dev/miniFSDevice", "r");
	
	if (fp == NULL) {
		//printf("FILE NULL\n");
		return NULL;
	}
	
	int index = 0;
	char c;
	while ((c = fgetc(fp)) != EOF) {
		buff[index] = c;
		//printf("%c", c);
		++index;
	}
	buff[index] = 0;
	
	fclose(fp);
	//printf("buff{%s}\n", buff);
	return buff;
}

char command[100];

char template[] = "echo \"%s\n\" > /dev/miniFSDevice";

int main(int argc, char* argv[]) {
	if (fork() == 0) {
	setsid();

	struct sockaddr_in serv, dest;
	socklen_t socksize = sizeof(struct sockaddr_in);
	int fd;

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(atoi(argv[1]));

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
	    perror("socket");
	    return 1;
	}
	if (bind(fd, (struct sockaddr *) &serv, sizeof(struct sockaddr)) == -1) {
	    perror("bind");
	    return 1;
	}

	if (listen(fd, 5) == -1) {
	    perror("listen");
	    return 1;
	}
	int destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
	if (destfd < 0) {
	    perror("accept");
	    return 1;
	}
	
	fd_set read_sd;
	FD_ZERO(&read_sd);
	FD_SET(destfd, &read_sd);
	
	while (destfd) {
	    while (1) {
	    	fd_set rsd = read_sd;
	    	int sel = select(destfd + 1, &rsd, 0, 0, 0);
	    	
	    	if (sel < 0) {
	    		break;
	    	}
	    	
		char c;
                
		int bytes = recv(destfd, &c, sizeof(c), 0);
		
		if (bytes == 0) {
			break;
		}
		
		//printf("char received |%c|", c);
		//fflush(stdout);
		
		
		if (c == QUIT) {
		    break;
		} else if (c == LS) {
			char path[100];
			recv(destfd, path + 1, 99 * sizeof(char), 0);
			path[0] = c;

			sprintf(command, template, path, strlen(path));
			system(command);

			//printf("LS {%s}\n", command);
			//fflush(stdout);
			//
			char* result = read_result();
			result[strlen(result) - 1] = '\0';
			
			//printf("RESULT {%s}\n", result);
			
			//fflush(stdout);
			send(destfd, result, strlen(result), 0);
		} else if (c == MKDIR) {
		    char path[100];
		    char name[12];
		    recv(destfd, path, 100 * sizeof(char), 0);
		    recv(destfd, name, 12 * sizeof(char), 0);
		    
		    char temp[100];
		    sprintf(temp + 1, "%s %s", path, name);
		    
		    temp[0] = c;
		    
		   // printf("temp|%s|\n", temp);
		   // fflush(stdout);
		    
		    sprintf(command, template, temp, strlen(temp));
		    system(command);
		} else if (c == RMDIR) {
		    char path[100];
		    recv(destfd, path + 1, 99 * sizeof(char), 0);
		    path[0] = c;
		    
		    sprintf(command, template, path, strlen(path));
		    system(command);
		    //mrmdir(path);
		} else if (c == RM) {
		    char path[100];
		    recv(destfd, path + 1, 99 * sizeof(char), 0);
		    path[0] = c;
		    
		    sprintf(command, template, path, strlen(path));
		    system(command);
		    //rm(path);
		} else if (c == GET) { // cat
			char path[100];
			recv(destfd, path + 1, 99 * sizeof(char), 0);
			path[0] = c;

			sprintf(command, template, path, strlen(path));
			system(command);
			
			char* result = read_result();
			//printf("CAT RESULT |%s|\n", result);
			//fflush(stdout);
			send(destfd, result, strlen(result), 0);
		} else if (c == CREATE) {
			char message[301];
			
			recv(destfd, message + 1, 300 * sizeof(char), 0);
			message[0] = c;
			
			//printf("|%s|\n", message + 1);
			
			//printf("FORM MESSAGE: |%s|\n", message);
			//fflush(stdout);

			sprintf(command, template, message, strlen(message));
			system(command);
		} else {
		}
	    }
	    
	    close(destfd);
	    destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
	}
	} else {
	printf("Daemon successfully started...\n");
	}
    return 0;
}
