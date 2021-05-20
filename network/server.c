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

char buff[100];

char* read_result() {

	//system("cat /dev/miniFSDevice");
	
	FILE* fp;
	fp = fopen("/dev/miniFSDevice", "r");
	
	if (fp == NULL) {
		printf("FILE NULL\n");
		return NULL;
	}
	
	int index = 0;
	char c;
	while ((c = fgetc(fp)) != EOF) {
		buff[index] = c;
		printf("%c", c);
		++index;
	}
	buff[index] = 0;
	
	fclose(fp);
	printf("buff{%s}\n", buff);
	return buff;
}

char command[100];

char template[] = "echo \"%s\n\" > /dev/miniFSDevice";

int main(int argc, char* argv[]) {
	/*if (fork() == 0) {
	setsid();*/

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
	while (destfd) {
	    for (;;) {
		char c;
                
		recv(destfd, &c, sizeof(c), 0);
		
		
		if (c == QUIT) {
		    break;
		}
		if (c == LS) {
			char path[100];
			recv(destfd, path + 1, 99 * sizeof(char), 0);
			path[0] = c;

			sprintf(command, template, path, strlen(path));
			system(command);

			printf("LS {%s}\n", command);
			fflush(stdout);
			
			char* result = read_result();
			printf("RESULT {%s}\n", result);
			fflush(stdout);
			send(destfd, result, strlen(result), 0);
		}
		if (c == MKDIR) {
		    char path[100];
		    char name[12];
		    recv(destfd, path, 100 * sizeof(char), 0);
		    recv(destfd, name, 12 * sizeof(char), 0);
		    
		    char temp[100];
		    sprintf(temp + 1, "%s %s", path, name);
		    
		    temp[0] = c;
		    
		    printf("temp|%s|\n", temp);
		    fflush(stdout);
		    
		    sprintf(command, template, temp, strlen(temp));
		    system(command);
		}
		if (c == RMDIR) {
		    char path[100];
		    recv(destfd, path + 1, 99 * sizeof(char), 0);
		    path[0] = c;
		    
		    sprintf(command, template, path, strlen(path));
		    system(command);
		    //mrmdir(path);
		}
		if (c == RM) {
		    char path[100];
		    recv(destfd, path + 1, 99 * sizeof(char), 0);
		    path[0] = c;
		    
		    sprintf(command, template, path, strlen(path));
		    system(command);
		    //rm(path);
		}
		if (c == GET) { // cat
			char path[100];
			recv(destfd, path + 1, 99 * sizeof(char), 0);
			path[0] = c;

			sprintf(command, template, path, strlen(path));
			system(command);
			
			char* result = read_result();
			printf("CAT RESULT |%s|\n", result);
			fflush(stdout);
			send(destfd, result, strlen(result), 0);
		}
		if (c == CREATE) {
			char message[301];
			
			recv(destfd, message + 1, 300 * sizeof(char), 0);
			message[0] = c;
			
			printf("|%s|\n", message + 1);
			
			printf("FORM MESSAGE: |%s|\n", message);
			fflush(stdout);

			sprintf(command, template, message, strlen(message));
			system(command);
		}
	    }
	    close(destfd);
	    destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
	}
	/*} else {
	printf("Daemon successfully started...\n");
	}*/
    return 0;
}

/*
int main(int argc, char* argv[]) {
	printf("start");
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
        
        fflush(stdout);
        
        int destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
        if (destfd < 0) {
            perror("accept");
            return 1;
        }
        
        printf("WHILE\n");
        
        while (destfd) {
            for (;;) {
                char c;
                FILE *fp;
                fp = fopen("/dev/miniFSDevice", "w+r");
                
                if (fp == NULL) {
                	printf("FOPEN ERROR");
                } else {
                	printf("FOPEN OK\n");
                }
                
                fflush(stdout);
                
                recv(destfd, &c, sizeof(c), 0);
                
                printf("CHAR|%c|\n", c);
                
                fflush(stdout);
                
                if (c == QUIT) {
                    break;
                }
                if (c == LS) {
                	printf("LS\n");
                	
                	fflush(stdout);
                    
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    
                    printf("PATH(%s)\n", path);
                    
                    fflush(stdout);
                    
                    fputs(path, fp);
                    fclose(fp);
                    printf("AFTER FPUTS\n");
                    fflush(stdout);
                    size_t ressize;
                    char list[] = "LS RESUTL";
                    send(destfd, &ressize, sizeof(ressize), 0);
                    send(destfd, list, ressize * sizeof(char), 0);
                    //free(list);
                }
            }
        }
              /*  if (c == MKDIR) {
                    char path[100];
                    char name[12];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    recv(destfd, name, 12 * sizeof(char), 0);
                    mkdir(path, name);
                }
                if (c == RMDIR) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    mrmdir(path);
                }
                if (c == RM) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    rm(path);
                }
                if (c == PUT) {
                    char path[100];
                    char name[12];
                    size_t size;

                    recv(destfd, path, 100 * sizeof(char), 0);
                    recv(destfd, name, 12 * sizeof(char), 0);
                    recv(destfd, &size, sizeof(size), 0);
                    char *file = malloc(size);
                    recv(destfd, file, size, 0);
                    create(path, name, file, size);
                    free(file);
                }
                if (c == GET) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);

                    char *file = cat_remote(path);
                    size_t size = strlen(file);
                    send(destfd, &size, sizeof(size), 0);
                    send(destfd, file, size, 0);
                    free(file);
                } 
            }
            close(destfd);
            destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
        }
        
    if (fork() == 0) {
        //setsid();

        
    } else {
        printf("Daemon successfully started...\n");
    }
    return 0;
}*/

