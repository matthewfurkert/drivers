#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x3b12617d, "module_layout" },
	{ 0x3b621568, "class_destroy" },
	{ 0xf61b0f94, "platform_driver_unregister" },
	{ 0xedf4771f, "__platform_driver_register" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x5e131dd8, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x1a0e95f5, "devm_kfree" },
	{ 0x2b73afef, "device_create" },
	{ 0x7cf5b970, "cdev_add" },
	{ 0x818f4cbd, "cdev_init" },
	{ 0xd440b725, "devm_kmalloc" },
	{ 0xf6803648, "cdev_del" },
	{ 0x504b9a95, "device_destroy" },
	{ 0xc5850110, "printk" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

MODULE_INFO(depends, "");

