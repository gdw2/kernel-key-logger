#include <linux/module.h>	/* Needed by all modules */
#include <linux/keyboard.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>

#include <linux/list.h>

MODULE_LICENSE("GPL");

/* 
 * Slab cache structure used to hold list items
 */
static struct kmem_cache *my_cachep;

/*
 * Name of proc file used to read keylogger stats
 */
static const char keycount[] = "keycount"; 

/*
 * Semaphore used to restrict access to datastructures
 * to one thread only
 */
static DEFINE_SEMAPHORE(data_lock);

/*
 * Structure which stores stats for individual letters
 */
struct key_statistic {
    unsigned int value; // Associated keycode (eg 30)
    char c; // Associated charater (eg "A")
    int count; // Number of times letter has been pressed (non-case sensative)
    struct list_head list; // Needed to use kernel linked list
};

static struct key_statistic stat_list;

/*
 * hello_notify
 *
 * Keyboard handler.  If keycode matches one in the list we're listening for,
 * increment the counter for that keycode.
 */
int hello_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    struct key_statistic *tmp;

    int ret = NOTIFY_OK;

    // If event is a keycode and it is a key down (as opposed to key up)
    if (code == KBD_KEYCODE && param->down) {
        printk(KERN_DEBUG "KEYLOGGER %i \n", param->value);

        // Grab the semaphore
        down(&data_lock);

        // Loop through all items in stat list
        list_for_each_entry(tmp, &stat_list.list, list)
        {
            // if key code matches the pressed key
            if (tmp->value == param->value)
            {
                printk(KERN_DEBUG "KEYLOGGER FOUND!: %c \n", tmp->c);
                // Increment
                tmp->count++;

            }
        }  
        // Release semaphore
        up(&data_lock);
    }

    return ret;
}

// Required for some reason
static struct notifier_block nb = {
    .notifier_call = hello_notify
};

/*
 * keycount_read
 *
 * Callback for proc read.  Prints out stat list
 */
static int keycount_read(char *buf, char **start, off_t offset,
        int count, int *eof, void *data)
{
    int len_cnt= 0; // Total number of characters in report to be printed.
    struct key_statistic *tmp;

    // For some reason, this function is called three times (I think incorrect
    // behavior on the part of some user-land apps).  We only want to proceed
    // if offset == 0.
    if( offset > 0 ) return 0;


    // Grab the semaphore
    down(&data_lock);

    // Loop through each item in stat_list and compile output
    list_for_each_entry(tmp, &stat_list.list, list)
    {
        len_cnt += sprintf(buf + len_cnt, "%c: %d\n", tmp->c, tmp->count);
    }

    // Release semaphore
    up(&data_lock);

    return len_cnt;
}

/* add_char
 *
 * Helper method to aid in creation of initial stat table
 *
 * param value - keycode to listen for
 * param c - character (e.g. "A" associated with the keycode)
 * param list - Kernel linked list to add entry to
 */
static void add_char(unsigned int value, char c, struct list_head * list)
{
    struct key_statistic *tmp;
    tmp = (struct key_statistic *)kmem_cache_alloc(my_cachep, GFP_KERNEL);
    tmp->c = c;
    tmp->value = value;
    tmp->count = 0;
    list_add_tail(&(tmp->list), list);

}

/*
 * key_stat_init
 *
 * Initializes kernel module
 */
static int key_stat_init(void)
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

/*
 * key_stat_release
 *
 * Releases kernel module and does cleanup.  Removes proc entry and
 * releases slab allocated memory
 */
static void key_stat_release(void)
{
    struct char_list *tmp;
    struct list_head *pos, *q;

    unregister_keyboard_notifier(&nb);
    remove_proc_entry(keycount, NULL); // remove proc item
    
    // Remove items in linked list
    list_for_each_safe(pos, q, &stat_list.list)
    {
        list_del(pos);
        kmem_cache_free(my_cachep, tmp);
    }

    if (my_cachep) kmem_cache_destroy( my_cachep);
}

module_init(key_stat_init);
module_exit(key_stat_release);
