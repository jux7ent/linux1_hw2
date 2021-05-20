#ifndef MINIFS_UTILS_H
#define MINIFS_UTILS_H

#include "../fs/fs.h"
#include "../fs/functions.h"

char* ls(char*, size_t*);
void mkdir(char* path, char* name);
void create(char* path, char* name, void* file, size_t size);

char* ls(char* path, size_t* ressize) {
    size_t indx = find_file(path); 
    size_t size = get_inode(indx)->size_of_file;
    dir_record* files = read_file(indx);

    char* buf = malloc(sizeof(char));
    buf[0] = 0;
    size_t bufsize = 1;
    
    int i;
    for (i = 0; i < size / sizeof(dir_record); ++i) {
        bufsize += strlen(files[i].name) + 1;
        buf = realloc(buf, bufsize);
        strcat(buf, files[i].name);
        strcat(buf, "\n");
        
        pr_info("APPEND TO LS {%s}\n", files[i].name);
    }
    free(files);
    if (ressize) {
        *ressize = bufsize;
    }
    pr_info("LS RESULT BUFF {%s}", buf);
    return buf;
}

void mkdir(char* path, char* name) {
    size_t parent_indx = find_file(path);
    
    printk(KERN_INFO "INSIDE mkdir find_file(%s) create_file(%s) index: %ld", path, name, parent_indx);
    
    create_file(NULL, 0, name, parent_indx, DIR);
}

void create(char* path, char* name, void* file, size_t size) {
    size_t parent_indx = find_file(path);
    printk(KERN_INFO "create (%s) index: %ld", file, parent_indx);
    create_file(file, size, name, parent_indx, TEXT);
}

char* cat(char* path) {
    size_t indx = find_file(path);
    printk(KERN_INFO "CAT find_file(%s) index: %ld", path, indx);
    char* text = read_file(indx);
    printk(KERN_INFO "CAT read_file(%ld) text: |%s|", indx, text);
    text[get_inode(indx)->size_of_file] = 0;
    pr_info("CAT size_of_file(%ld)\nTEXT:{%s}\n", get_inode(indx)->size_of_file, text);
    return text;
}

int get_last_delim(const char* path) {
    int last_delim = 0;
    int i;
    for (i = 0; path[i]; ++i) {
        if (path[i] == '/') {
            last_delim = i;
        }
    }
    return last_delim;
}

void remove_impl(char* path, int type) {
    int delim = get_last_delim(path);
    char* parent = malloc(100);
    strncpy(parent, path, delim);

    size_t parent_indx = find_file(parent);
    size_t indx = find_file(path);
    if (get_inode(indx)->type_of_file == type) {
        return;
    }
    
    remove_file(indx, parent_indx);
}

void rm(char* path) {
    remove_impl(path, DIR);
}

void mrmdir(char* path) {
    remove_impl(path, TEXT);
}

#endif //MINIFS_UTILS_H
