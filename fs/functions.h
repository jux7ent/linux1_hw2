#ifndef MINIFS_FUNCTIONS_H
#define MINIFS_FUNCTIONS_H

#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "files_managment.h"

// whence == SEEK_SET - Beginning of file
// whence == SEEK_CUR - Current position of the file pointer

unsigned char* full_buffer;
unsigned char* curr_buffer;

void* malloc(size_t size);

char * last = NULL;
char* strtok(const char * inp, const char * delim) {
    if(inp == NULL){
        //try to continue from last.
        inp = last;
    }
    if(inp == NULL){
        //last was null too ;_;
        return NULL;
    }
    int len = 0;
    last = inp;
    while(inp != NULL){
        last++;
        if(inp[len] == *delim){
            bool y = true;
            int j = 0;
            int i;
            for(i = len; delim[i - len] != '\0'; i++)//check if this really is delim.
            {
                if(inp[i] != delim[i - len] || inp[i] == '\0'){
                    y = false;
                    len += i - len;
                    last += i - len - 1;
                    break;
                }
                j++;
            }
            if(y)
            {
                //we really have reached the delim!
                len+= j;
                last += j - 1;
                break;
            }
        }
        if(inp[len] == '\0'){
            //reached end of char array. 
            last = NULL;
            break;
        }
        len++;
    }
    
    if (inp[len - 1] == '/') {
    	--len; // ????
    }
    
    //construct new pointer/array
    char * returned = malloc(len + 1); //+1 as we need the \0 operator
    int i;
    for(i = 0; i < len; i++){
        returned[i] = inp[i]; 
    }
    returned[len] = '\0';
    return returned;
}
/*
char* strcpy(char* strDest, const char* strSrc) {
	char* temp = strDest;
	while(*strDest++ = *strSrc++);
	return temp;
}*/

char* strcpy(char* destination, const char* source) {
    char *start = destination;

    while(*source != '\0') {
        *destination = *source;
        destination++;
        source++;
    }

    *destination = '\0'; // add '\0' at the end
    return start;
}


void set_buffer(unsigned char* buffer) {
	full_buffer = buffer;
	curr_buffer = buffer;
}

int fseek(unsigned char* dev, long pos, int whence) {
	int result = 0;
	
	if (whence == SEEK_SET) {
		curr_buffer = full_buffer + pos;
	} else if (whence == SEEK_CUR) {
		curr_buffer = curr_buffer + pos;
	}
	
	//pr_info("fseek curr_buffer_index %ld\n", (curr_buffer - full_buffer));
	
	return result;
}

int fread(void *ptr, size_t size, size_t count) {
	memcpy(ptr, curr_buffer, size * count); // memcpu(*dest, *src, count)
	return 0;
	//return file_read(stream, current_pos_index, ptr, count * size);
}

int fwrite(void *ptr, size_t size, size_t count) {
	memcpy(curr_buffer, ptr, size * count);
	
	//pr_info("WRITE TO %ld\n", (curr_buffer - full_buffer));
	return 0;
	//return file_write(stream, current_pos_index, ptr, count * size);
}

void* malloc(size_t size) {
	return kmalloc(size, GFP_KERNEL);
}

void* realloc(void *ptr, size_t new_size) {
	return krealloc(ptr, new_size, GFP_KERNEL);
}

void free(void* ptr) {
	kfree(ptr);
}

#endif
