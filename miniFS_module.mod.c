#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x9de7765d, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xf6ba0674, "__register_chrdev" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x9166fada, "strncpy" },
	{ 0x61651be, "strcat" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x37a0cba, "kfree" },
	{ 0xb1e12d81, "krealloc" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x69acdf38, "memcpy" },
	{ 0xedb3c891, "vfs_write" },
	{ 0x4fac3996, "vfs_read" },
	{ 0xd0e40bb7, "filp_close" },
	{ 0xf39042a8, "filp_open" },
	{ 0xebbe12f0, "current_task" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0xc5850110, "printk" },
	{ 0xa8fdfe8, "try_module_get" },
	{ 0x9b858114, "module_put" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "8F915E2035C38CCAE9353E0");
