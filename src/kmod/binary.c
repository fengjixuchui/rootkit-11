#include <utils.h>
#include <binary.h>
#include <debug.h> 

void boot_binary(char *path){	
	char * boot_path = strcat(BINARY_PATH, path);
	LOGI("Attempting to boot %s\n", boot_path);
	syscall()
}


void kill_binary(char *path){
	return;
}

#endif /* NETWORK_H */
