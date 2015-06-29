/*
 * sync_module.c
 *
 *  Created on: 29 Jun 2015
 *      Author: lester
 *
 *  A simple module that creates a file in /proc and do write in a synchronize way
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/debugfs.h>
#include <linux/mm.h>  // mmap related stuff
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/kthread.h>  // for threads
#include <linux/string.h>

static atomic_t openend = ATOMIC_INIT(0);
struct proc_dir_entry *proc_file_entry;

struct cmd_if
{
  unsigned wr;
  unsigned rd;
  char op[50];
  wait_queue_head_t trigger;
  struct task_struct * thread;
};

static int thread(void * v)
{
  char c;
  struct cmd_if* cmd;
  printk(KERN_ERR "thread start\n");
  cmd = v;
  for (;;)
  {
    if (cmd->rd < cmd->wr)
    {
      c=cmd->op[cmd->rd];
      rmb();
      cmd->rd++;
      if (cmd->rd == cmd->wr)
        wake_up(&cmd->trigger);
      printk("%c\n",c);
    }
    msleep_interruptible(1000);   // if non-zero then signal
    if (kthread_should_stop())
      break;
  }
  printk(KERN_ERR "thread stop\n");
  return 0;
}


static int device_open(struct inode * i, struct file * f)
{
  struct cmd_if* pthis;
  int old = atomic_xchg(&openend, 1);
  if (old == 1)
  {
    printk(KERN_ERR "file is already open\n");
    return -EBUSY;
  }
  if ((pthis = kmalloc(sizeof(struct cmd_if), GFP_KERNEL)) == NULL)
  {
    return -ENOMEM;
  }
  f->private_data = pthis;
  memset(pthis,0,sizeof(*pthis));
  init_waitqueue_head(&pthis->trigger);
  if ((pthis->thread = kthread_create(&thread, (void *)pthis, "async-thread")) == NULL)
    goto failed;

  wake_up_process(pthis->thread);
  return 0;

  failed:
  if (pthis != NULL)
  {
    if (pthis->thread != NULL) kthread_stop(pthis->thread);
    kfree(pthis);
       f->private_data = NULL;

  }
  return -ENOMEM;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t nbytes, loff_t *ppos)
{
  struct cmd_if* cmd;
  unsigned count,max;
  cmd = (struct cmd_if*) file->private_data;
  count = 0;
  do
  {
    // wait for buffer become empty
    if (wait_event_interruptible(cmd->trigger,cmd->rd == cmd->wr) != 0)
        return -EINVAL;
    cmd->wr = 0;
    wmb();
    cmd->rd = 0;
    max = nbytes < sizeof(cmd->op) ? nbytes : sizeof(cmd->op);
    if (copy_from_user(cmd->op, buf, max) != 0)
    {
      printk("failed to get user data\n");
      return -EINVAL;
    }
    wmb();
    cmd->wr = max;
    count += max;
    nbytes -= max;
    buf += max;
  } while (nbytes);
  return count;
}

static int file_release(struct inode *i, struct file *f)
{
  struct cmd_if* pthis = (struct cmd_if*)f->private_data;
  // wait for buffer become empty
  wait_event_interruptible(pthis->trigger,pthis->rd == pthis->wr);
  printk("stopping ...\n");
  kthread_stop(pthis->thread);
  kfree(pthis);
  f->private_data = NULL;
  atomic_xchg(&openend, 0);
  return 0;
}

static const struct file_operations proc_file_fops = { //
    .open = device_open,    //
    .write = device_write, //
    .release = file_release, //
    };

static int sync_init(void)
{
  proc_file_entry = proc_create("async", 0, NULL, &proc_file_fops);
  if (proc_file_entry == NULL)
    return -ENOMEM;
  return 0;
}

static void sync_cleanup(void)
{
  remove_proc_entry("async", NULL);
}

module_init(sync_init);
module_exit(sync_cleanup);

