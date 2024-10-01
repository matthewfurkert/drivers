# include<linux/module.h>

static int __init helloworld_init(void)
{
    pr_info("Hello_World");
    return 0;
}

static void __exit helloworld_cleanup(void)
{
    pr_info("Goodbye_World");
}

module_init(helloworld_init);
module_exit(helloworld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FURKY");
MODULE_DESCRIPTION("A SIMPLE HELLO WORLD KERNEL MODULE");
MODULE_INFO(board,"Beaglebone Black");