#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /* Do nothing */
	return 0;
}

struct task_struct *thread1;
struct task_struct *thread2;

static int thread_fn(void *data) {
    while (!kthread_should_stop()) {
        ssleep(1); // Sleep for a while
    }
    return 0;
}

static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset) {
    char info[256];
    int len = 0;
    ssize_t ret;
    struct task_struct *task;

    // Create two threads
    thread1 = kthread_create(thread_fn, NULL, "thread1");
    thread2 = kthread_create(thread_fn, NULL, "thread2");

    if (IS_ERR(thread1) || IS_ERR(thread2)) {
        pr_info("Failed to create threads\n");
        return -EFAULT;
    }

    // Gather information about the threads
    // for_each_thread(task, thread) {
    //     len += snprintf(info + len, sizeof(info) - len, "PID: %d, TID: %d, Priority: %d, State: %d\n",
    //                     task->pid, thread->pid, thread->prio, thread->__state);
    // }
    len += snprintf(info + len, sizeof(info) - len, "Thread 1: PID: %d, State: %d\n", thread1->pid, thread1->__state);
    len += snprintf(info + len, sizeof(info) - len, "Thread 2: PID: %d, State: %d\n", thread2->pid, thread2->__state);

    // Check if offset is beyond the length of the string
    if (*offset >= len) {
        return 0;
    }

    // Adjust length to copy based on buffer_len and remaining data
    ret = len - *offset;
    if (ret > buffer_len) {
        ret = buffer_len;
    }

    // Copy data to user space
    if (copy_to_user(ubuf, info + *offset, ret)) {
        return -EFAULT;
    }

    // Update offset
    *offset += ret;

    return ret;
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