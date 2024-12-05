#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /* Do nothing */
	return 0;
}
static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){
    /*Your code here*/
    struct task_struct *task = current;
    struct task_struct *thread;
    int len = 0;
    ssize_t ret;

    if (*offset) {
        return 0;
    }
    else{
        for_each_thread(task, thread) {
            if (thread == task) {
                continue; // Skip the main thread
            }
            len += snprintf(buf + len, sizeof(buf) - len, "PID: %d, TID: %d, Priority: %d, State: %d\n",
                            task->pid, thread->pid, thread->prio, thread->__state);
        }
    }

    // Adjust length to copy based on buffer_len and remaining data
    ret = len;
    if (ret > buffer_len) {
        ret = buffer_len;
    }

    if (copy_to_user(ubuf, buf, ret)) {
        return -EFAULT;
    }

    // Update offset
    *offset += ret;

    return ret;
    /****************/
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void){
    proc_create(procfs_name, 0644, NULL, &Myops);   
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void){
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
