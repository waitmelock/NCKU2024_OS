#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE]; //kernel buffer
static unsigned long procfs_buffer_size = 0;

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /*Your code here*/
    struct task_struct *task = current;

    procfs_buffer_size = min(buffer_len,BUFSIZE);
    if (buffer_len > BUFSIZE) {
        pr_info("Buffer size is too large\n");
        return -EFAULT;
    }

    if (copy_from_user(buf, ubuf, procfs_buffer_size)) {
        return -EFAULT;
    }
    procfs_buffer_size+=snprintf(buf + procfs_buffer_size, sizeof(buf) - procfs_buffer_size, "PID: %d, TID: %d, time: %llu\n",
                current->tgid, current->pid, current->utime/100/1000);

    *offset += procfs_buffer_size ;
    pr_info("Data written to proc file: %s\n", buf);

    return procfs_buffer_size;
    /****************/
}


static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){
    /*Your code here*/
    if(*offset>0){
        return 0;
    }
    procfs_buffer_size=min(procfs_buffer_size,BUFSIZE);

    if (copy_to_user(ubuf, buf, procfs_buffer_size)) {
        return -EFAULT;
    }

     *offset += procfs_buffer_size ;
    return procfs_buffer_size;
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
