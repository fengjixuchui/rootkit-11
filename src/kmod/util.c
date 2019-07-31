#include <util.h>

char * strcat(char *src, char*dest){
	length_src = strlen(src);
	length_dest = strlen(dest);

	char new_string[length_src+length_dest];
	
	int counter = 0;
	while(src[counter] != NULL){
		new_string[counter] = src[counter];
	}
	counter += 1;

	int new_counter = 0;
	while(dest[new_counter] != NULL){
		new_string[counter] = dest[new_counter];
		counter += 1;
		new_counter += 1;
	}
	
	return new_string;
}

int strlen(char * str){
	int length = 0;
		
	while(str[length] != NULL){
		length += 1;
	}

	return length;
}

