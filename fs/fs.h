#ifndef MINIFS_FS_H
#define MINIFS_FS_H

#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>

#include "files_managment.h"
#include "functions.h"
#include "bitmap.h"

#define TEXT 1
#define DIR 2

#define NUMBER_OF_PTRS 5
#define MAGIC_NUMBER 1337
#define NUMBER_OF_INODES 16
#define BLOCK_SIZE 1024
#define NUMBER_OF_BLOCKS 64


typedef struct {
    size_t magic_number;
    size_t number_of_blocks;
    long size_of_block;
    size_t size_of_inode_table;
    long head_of_blocks;
    long long bitmap_inodes;
} super_block;

typedef struct {
    int type_of_file;
    size_t size_of_file;
    long pointers[NUMBER_OF_PTRS + 1];
} inode;

typedef struct {
    int index; // индекс в массиве i-nodes
    char name[12];
} dir_record;

super_block sb;
unsigned char* dev;
inode inode_table[NUMBER_OF_INODES];
struct file* f;
//struct file* dev;

long* write_ptrs(size_t size, void* file);
void write_file(size_t indx, void* file);
void* read_file(size_t indx);
void read_ptrs(long* ptrs, size_t size, void* file);
void create_root(void);
void make_block_list(long begin);
int create_fs(void);
int start_fs(const char* fs_file_path);
int open_fs(void);
void fs_read(long pos, void* ptr, size_t size, size_t nmemb);
void fs_write(long pos, void* ptr, size_t size, size_t nmemb);
long get_block(void);
void save_dev(void);
inode* get_inode(size_t indx);
size_t find_file(char* path);
void create_file(void* file, size_t size, char* name, size_t parent_indx, int type);
void remove_file(size_t indx, size_t parent_indx);
void save_fs(void);

void remove_file(size_t indx, size_t parent_indx) {
    inode* node = &inode_table[indx];
    int i;
    for (i = 0; i < (NUMBER_OF_PTRS - 1) && node->pointers[i + 1]; ++i) {
        fs_write(node->pointers[i], &node->pointers[i + 1], sizeof(long), 1);
    }
    if (node->pointers[NUMBER_OF_PTRS] != 0) {
        long* ptrs = malloc(sb.size_of_block / sizeof(long) * sizeof(long));
        fs_read(node->pointers[NUMBER_OF_PTRS], ptrs, sb.size_of_block, 1);
        int j;
        for (j = 0; ptrs[j + 1]; ++j) {
            fs_write(ptrs[j], &ptrs[j + 1], sizeof(long), 1);
        }
        fs_write(node->pointers[NUMBER_OF_PTRS - 1], &ptrs[0], sizeof(long), 1);
        fs_write(ptrs[j], &sb.head_of_blocks, sizeof(long), 1);
        free(ptrs);
    } else {
        fs_write(node->pointers[i], &sb.head_of_blocks, sizeof(long), 1);
    }
    sb.head_of_blocks = node->pointers[0];
    free_index(&sb.bitmap_inodes, indx);

    // delete from parent catalog
    inode* parent = &inode_table[parent_indx];
    dir_record* files = read_file(parent_indx);
    dir_record* new_files = malloc(sizeof(dir_record) * (parent->size_of_file - sizeof(dir_record)));
    int j = 0;
    for (i = 0; i < parent->size_of_file / sizeof(dir_record); ++i) {
        if (files[i].index == indx) {
            continue;
        }
        new_files[j++] = files[i];
    }
    free(files);
    dir_record* tmp = new_files;
    size_t new_size = parent->size_of_file - sizeof(dir_record);
    parent->size_of_file = new_size;
    for (i = 0; parent->pointers[i]; ++i) {
        size_t wsize = new_size > sb.size_of_block ? sb.size_of_block : new_size;
        new_size -= wsize;
        fs_write(parent->pointers[i], tmp, wsize, 1);
        tmp += wsize;
    }
    save_dev();
}


void create_file(void* file, size_t size, char* name, size_t parent_indx, int type) {
    printk(KERN_INFO "INSIDE CREATE_FILE(%s) NAME(%s) PARENT_INDX(%ld)", file, name, parent_indx);
    
    size_t indx = set_index(&sb.bitmap_inodes);
    inode* node = &inode_table[indx];
    node->type_of_file = type;
    if (type == DIR) {
        dir_record root[2];
        root[0].index = indx;
        root[1].index = parent_indx;
        strcpy(root[0].name, ".");
        strcpy(root[1].name, "..");
        node->size_of_file = 2 * sizeof(dir_record);
        write_file(indx, root);
    } else {
        node->size_of_file = size;
        write_file(indx, file);
    }

    // add to parent directory
    dir_record new_record;
    new_record.index = indx;
    strcpy(new_record.name, name);
    printk(KERN_INFO "NEW_RECORD.NAME(%s)", new_record.name);
    dir_record* parent = read_file(parent_indx);
    inode* parent_node = &inode_table[parent_indx];
    size_t new_size = parent_node->size_of_file + sizeof(dir_record);
    parent = realloc(parent, new_size);
    parent[parent_node->size_of_file / sizeof(dir_record)] = new_record;
    size_t count_blocks = (parent_node->size_of_file + sb.size_of_block) / sb.size_of_block;
    size_t new_count_blocks = (new_size + sb.size_of_block) / sb.size_of_block;
    parent_node->size_of_file = new_size;

    dir_record* tmp = parent;
    int i;
    for (i = 0; i < NUMBER_OF_PTRS && parent_node->pointers[i]; ++i) {
        size_t wsize = new_size > sb.size_of_block ? sb.size_of_block : new_size;
        new_size -= wsize;
        fs_write(parent_node->pointers[i], tmp, wsize, 1);
        tmp += wsize;
    }
    if (new_count_blocks > count_blocks) {
        long new_block = get_block();
        fs_write(new_block, tmp, new_size, 1);
        parent_node->pointers[i] = new_block;
    }
    free(parent);
    save_dev();
}


void fs_read(long pos, void* ptr, size_t size, size_t nmemb) {
    fseek(dev, pos, SEEK_SET);
    fread(ptr, size, nmemb);
}

void fs_write(long pos, void* ptr, size_t size, size_t nmemb) {
    fseek(dev, pos, SEEK_SET);
    fwrite(ptr, size, nmemb);
}

void save_dev(void) {
    fs_write(0, &sb, sizeof(super_block), 1);
    fs_write(BLOCK_SIZE, inode_table, sizeof(inode), NUMBER_OF_INODES);
}

void read_ptrs(long* ptrs, size_t size, void* file) {
	size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
	size_t i;
	for (i = 0; i < count_ptrs; ++i) {
		size_t rsize = size < sb.size_of_block ? size : sb.size_of_block;
		fs_read(ptrs[i], file, rsize, 1);
		file += rsize;
		size -= rsize;
	}
}

long get_block(void) {
    long block = sb.head_of_blocks;
    fseek(dev, block, SEEK_SET);
    fread(&sb.head_of_blocks, sizeof(block), 1);
    return block;
}


long* write_ptrs(size_t size, void* file) {
    size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
    long* ptrs = malloc((count_ptrs + 1) * sizeof(size_t));
    size_t i;
    for (i = 0; i < count_ptrs; ++i) {
        long next_block = get_block();
        size_t wsize = size < sb.size_of_block ? size : sb.size_of_block;
        fs_write(next_block, file, wsize, 1);
        size -= wsize;
        file += wsize;
        ptrs[i] = next_block;
    }
    ptrs[count_ptrs] = 0;
    return ptrs;
}

void write_file(size_t indx, void* file) {
    inode* node = &inode_table[indx];
    size_t size = node->size_of_file;
    long* ptrs = write_ptrs(size, file);
    size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
    size_t i;
    for (i = 0; i < NUMBER_OF_PTRS; ++i) {
        node->pointers[i] = ptrs[i];
    }
    if (count_ptrs < NUMBER_OF_PTRS) {
        node->pointers[NUMBER_OF_PTRS] = 0;
    } else {
        long new_block = get_block();
        node->pointers[NUMBER_OF_PTRS] = new_block;
        fs_write(new_block, ptrs + NUMBER_OF_PTRS, sizeof(long), count_ptrs - NUMBER_OF_PTRS + 1);
    }
    free(ptrs);
}

void* read_file(size_t indx) {
    inode* node = &inode_table[indx];
    void* file = malloc(node->size_of_file + 1);
    if (node->pointers[NUMBER_OF_PTRS] == 0) {
        read_ptrs(node->pointers, node->size_of_file, file);
    } else {
        long* ptrs = malloc((sb.size_of_block + NUMBER_OF_PTRS) * sizeof(long));
        int i;
        for (i = 0; i < NUMBER_OF_PTRS; ++i) {
            ptrs[i] = node->pointers[i];
        }
        fs_read(node->pointers[NUMBER_OF_PTRS], ptrs + NUMBER_OF_PTRS, sb.size_of_block, 1);
        read_ptrs(ptrs, node->size_of_file, file);
        free(ptrs);
    }
    return file;
}

inode* get_inode(size_t indx) {
    return &inode_table[indx];
}

size_t find_file(char* path) {
    size_t indx = 0;
    
    printk(KERN_INFO "BEFORE STRTOK: PATH |%s|", path);
    printk(KERN_INFO "strtok_0 |%s|", strtok(path, "/"));
    
    char* next;
    for (next = strtok(NULL, "/"); next; next = strtok(NULL, "/")) {
        printk(KERN_INFO "strtok_next |%s|", next);
        dir_record* files = read_file(indx);
        int flag = 0;
        int i;
        for (i = 0; i < inode_table[indx].size_of_file / sizeof(dir_record); ++i) {
            if (strcmp(files[i].name, next) == 0) {
                indx = files[i].index;
                flag = 1;
                break;
            }
        }
        if (flag) {
            continue;
        }
        return 0;
    }
    return indx;
}

int open_fs(void) {
	fs_read(0, &sb, sizeof(sb), 1);
	if (sb.magic_number != MAGIC_NUMBER) {
		printk(KERN_INFO "INVALID SUPER BLOCK. Magic number: %ld\n", sb.magic_number);
		return 1;
	} else {
		printk(KERN_INFO "VALID SUPER BLOCK\n");
	}
	fs_read(sb.size_of_block, inode_table, sizeof(inode), NUMBER_OF_INODES);
	return 0;
}

void create_root(void) {
	dir_record root[2];
	size_t indx = set_index(&sb.bitmap_inodes);
	inode* node = &inode_table[indx];
	node->type_of_file = DIR;
	root[0].index = indx;
	root[1].index = indx;
	strcpy(root[0].name, ".");
	strcpy(root[1].name, "..");
	node->size_of_file = 2 * sizeof(dir_record);
	write_file(indx, root);
}

void make_block_list(long begin) {
    fseek(dev, begin, SEEK_SET); // переключаю на два блока вперед указатель
    long i;
    for (i = 0; i < sb.number_of_blocks; ++i) {
        long next_pos = begin + (i + 1) * sb.size_of_block;
       // pr_info("next_pos %ld\n", next_pos);
        fwrite(&next_pos, sizeof(next_pos), 1); // записываю цепочку, чтобы каждый блок содержал индекс следующего блока
        fseek(dev, sb.size_of_block, SEEK_CUR);
    }
}

int create_fs(void) {
	dev = malloc(2 * BLOCK_SIZE + NUMBER_OF_BLOCKS * BLOCK_SIZE);
	set_buffer(dev);
	
	sb.number_of_blocks = NUMBER_OF_BLOCKS;
    	sb.magic_number = MAGIC_NUMBER;
    	sb.size_of_block = BLOCK_SIZE;
    	sb.bitmap_inodes = 0; // long long
    	sb.size_of_inode_table = NUMBER_OF_INODES * sizeof(inode);
    	sb.head_of_blocks = 2 * BLOCK_SIZE;
    	
    	make_block_list(2 * BLOCK_SIZE);
    	create_root();
    	
    	save_dev();
    	
    	return 0;
}


void save_fs(void) {
	file_write(f, 0, full_buffer, 2 * BLOCK_SIZE + NUMBER_OF_BLOCKS * BLOCK_SIZE);
}


int start_fs(const char* fs_file_path) {
	printk(KERN_INFO "StartFC\n");
	
	struct file* ndev = file_open(fs_file_path, O_RDWR, 0);
	f = ndev;
	
	if (ndev == NULL) {
		printk(KERN_INFO "OPEN FILE ERROR %s\n", fs_file_path);
		return -1;
	}
	
	dev = malloc(2 * BLOCK_SIZE + NUMBER_OF_BLOCKS * BLOCK_SIZE);
	set_buffer(dev);
	
	file_read(ndev, 0, dev, 2 * BLOCK_SIZE + NUMBER_OF_BLOCKS * BLOCK_SIZE);
	
	if (open_fs()) {
		create_fs();
		open_fs();
	}
	
	
	
	return 0;
}

#endif
