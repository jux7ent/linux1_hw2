#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros

#include "fs/fs.h"
#include "utils/utils.h"
#include "common.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lakshmanan");
MODULE_DESCRIPTION("A Simple Hello World module");

static char   message[256] = {0};

#define DEVICE_NAME "miniFSDevice"

static int major_num;
static int device_open_count = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);

static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations file_ops = {
  .open = device_open,
  .release = device_release,
  .write = device_write,
  .read = device_read
};

char* result_from_fs;

static ssize_t device_read(struct file* flip, char* buffer, size_t len, loff_t* offset) {
	printk(KERN_INFO "read\n");
	
	if (result_from_fs != NULL) {
		int res_len = strlen(result_from_fs);
		strcpy(buffer, result_from_fs);
		
		pr_info("DEVICE READ READED{%s}\n", buffer);
		
		result_from_fs = NULL;
		
		return res_len;
	} else {
		return 0;
	}
	
	//memcpy(buffer, test, strlen());
  
  // TODO

  
}

static ssize_t device_write(struct file* flip, const char* buffer, size_t len, loff_t* offset) {
  // printk(KERN_ALERT "This operation is not supported...\nBut you try to write: %s\n", buffer);

  char *message = buffer;
  message[len - 2] = 0;
  
  char command = message[0];
  ++message;
  
  pr_info("device_write:{%s}\n", message);
  
  if (command == LS) {
  	pr_info("command is LS\n");
  	char* path = message;
  	
  	char* list = ls(path, NULL);
  	
  	result_from_fs = list;
  } else if (command == MKDIR) {
  	pr_info("command is MKDIR\n");
  	
  	char* path = strtok(message, " ");
  	path[strlen(path) - 1] = 0;
  	char* name = strtok(NULL, " ");
  	
  	pr_info("mkdir |%s| |%s|\n", path, name);
  	
  	mkdir(path, name);
  	
  	result_from_fs = " ";
  } else if (command == GET) { // cat
  	pr_info("command is CAT\n");
  	
  	char* path = message;
  	pr_info("cat %s", path);
  	result_from_fs = cat(path);
  } else if (command == CREATE) {
  	pr_info("command is create\n");
  	pr_info("message: |%s|\n", message);
  	
  	char* path = strtok(message, " ");
  	path[strlen(path) - 1] = 0;
  	char* name = strtok(NULL, " ");
  	name[strlen(name) - 1] = 0;
  	char* file_content = strtok(NULL, "");
  	
  	pr_info("create_file |%s| |%s| |%s|\n", path, name, file_content);
  	
  	create(path, name, file_content, strlen(file_content) + 1);
  } else if (command == RMDIR) {
  	pr_info("command rm_dir\n");
  	
  	char* path = message;
  	mrmdir(path);
  } else if (command == RM) {
  	pr_info("command rm\n");
  	
  	char* path = message;
  	rm(path);
  } else {
  	pr_info("unknown command\n");
  }
  
  pr_info("result:|%s|\n", result_from_fs);

  
  
  // TODO

  return len;
}

static int device_open(struct inode* inode, struct file* file) {
  if (device_open_count)
    return -EBUSY;
    
  try_module_get(THIS_MODULE);
  return 0;
}

static int device_release(struct inode* inode, struct file* file) {
  module_put(THIS_MODULE);
  return 0;
}

static int __init hello_init(void) {
	// ================= start symbol device ============================
	major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
	if (major_num < 0) {
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num; 
	}
	printk(KERN_INFO "CHAR DIVEICE loaded with\nNAME:(%s)\nMAJOR_NUMBER:(%d)\n", DEVICE_NAME, major_num);

	// ================= start symbol device ============================
	
	if (start_fs("/home/jux7ent/Documents/Linux1/hw2/fs_container") == 0) {
		printk(KERN_INFO "FS started\n");
	} else {
		printk(KERN_INFO "FS FAILURE\n");
	}
	
    
	return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void) {
	unregister_chrdev(major_num, DEVICE_NAME);
  	printk(KERN_INFO "Goodbye, world!\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
