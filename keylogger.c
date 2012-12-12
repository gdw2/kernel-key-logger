#include <linux/module.h>	/* Needed by all modules */
#include <linux/keyboard.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#include <linux/list.h>

MODULE_LICENSE("GPL");

static struct kmem_cache *my_cachep;
static const char keycount[] = "keycount"; // name of proc file

struct key_statistic {
    unsigned int value;
    char c;
    int count;
    struct list_head list;
};

static struct key_statistic stat_list;



int hello_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    struct key_statistic *tmp;

    int ret = NOTIFY_OK;

    if (code == KBD_KEYCODE && param->down) {
        printk(KERN_DEBUG "KEYLOGGER %i \n", param->value);
        list_for_each_entry(tmp, &stat_list.list, list)
        {
            if (tmp->value == param->value)
            {
                printk(KERN_DEBUG "KEYLOGGER FOUND!: %c \n", tmp->c);
                tmp->count++;

            }
        }  
    }

    return ret;
}

static struct notifier_block nb = {
    .notifier_call = hello_notify
};

static int keycount_read(char *buf, char **start, off_t offset,
        int count, int *eof, void *data)
{
    printk(KERN_DEBUG "count: %d", count);

    int len_cnt  = 0;
    struct key_statistic *tmp;
    if( offset > 0 ) return 0;


    list_for_each_entry(tmp, &stat_list.list, list)
    {
        len_cnt += sprintf(buf + len_cnt, "%c: %d\n", tmp->c, tmp->count);
    }

    return len_cnt;
}

static void add_char(unsigned int value, char c, struct list_head * list)
{
    struct key_statistic *tmp;
    tmp = (struct key_statistic *)kmem_cache_alloc(my_cachep, GFP_KERNEL);
    tmp->c = c;
    tmp->value = value;
    tmp->count = 0;
    list_add_tail(&(tmp->list), list);

}

static int hello_init(void)
{
    struct proc_dir_entry *keycount_proc;
    printk(KERN_INFO "Hello my friendly world 1.\n");
    printk("Greg was here.");

    // Initialize data structures

    // Initializing cache
    my_cachep = kmem_cache_create(
            "hello_cache",
            sizeof(struct key_statistic),
            0,
            SLAB_HWCACHE_ALIGN,
            NULL);

    // init list
    INIT_LIST_HEAD(&stat_list.list);

    add_char(30,'A',&stat_list.list);
    add_char(48,'B',&stat_list.list);
    add_char(46,'C',&stat_list.list);
    add_char(32,'D',&stat_list.list);
    add_char(18,'E',&stat_list.list);
    add_char(33,'F',&stat_list.list);
    add_char(34,'G',&stat_list.list);
    add_char(35,'H',&stat_list.list);
    add_char(23,'I',&stat_list.list);
    add_char(36,'J',&stat_list.list);
    add_char(37,'K',&stat_list.list);
    add_char(38,'L',&stat_list.list);
    add_char(50,'M',&stat_list.list);
    add_char(49,'N',&stat_list.list);
    add_char(24,'O',&stat_list.list);
    add_char(25,'P',&stat_list.list);
    add_char(16,'Q',&stat_list.list);
    add_char(19,'R',&stat_list.list);
    add_char(31,'S',&stat_list.list);
    add_char(20,'T',&stat_list.list);
    add_char(22,'U',&stat_list.list);
    add_char(47,'V',&stat_list.list);
    add_char(17,'W',&stat_list.list);
    add_char(45,'X',&stat_list.list);
    add_char(21,'Y',&stat_list.list);
    add_char(44,'Z',&stat_list.list);

    // Create proc stuff
    keycount_proc = create_proc_entry(keycount, 0, NULL);
    keycount_proc->read_proc = keycount_read;

    // Register keyboard listener
    register_keyboard_notifier(&nb);

    return 0;
}

static void hello_release(void)
{
    unregister_keyboard_notifier(&nb);
    remove_proc_entry(keycount, NULL); // remove proc item
    if (my_cachep) kmem_cache_destroy( my_cachep);
}

module_init(hello_init);
module_exit(hello_release);
