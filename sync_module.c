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

static int device_open(struct inode * i, struct file * f)
{
  int old = atomic_xchg(&openend, 1);
  if (old == 1)
  {
    printk(KERN_ERR "file is already open\n");
    return -EBUSY;
  }
  return 0;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t nbytes, loff_t *ppos)
{
  char buff[50];
  unsigned rd;
  unsigned wr;
  unsigned count;
  count = 0;
  do
  {
    wr = nbytes < sizeof(buff) ? nbytes : sizeof(buff);
    if (copy_from_user(buff, buf, wr) != 0)
    {
      printk("failed to get user data\n");
      return -EINVAL;
    }
    count += wr;
    nbytes -= wr;
    buf += wr;
    rd = 0;
    while (rd < wr)
    {
      printk("%c\n", buff[rd++]);
      msleep(1000);
    }
  } while (nbytes);
  return count;
}

static int file_release(struct inode *i, struct file *f)
{
  atomic_xchg(&openend, 0);
  return 0;
}

static const struct file_operations proc_file_fops = { .open = device_open,    //
    .write = device_write, .release = file_release, };

static int sync_init(void)
{
  proc_file_entry = proc_create("sync1", 0, NULL, &proc_file_fops);
  if (proc_file_entry == NULL)
    return -ENOMEM;
  return 0;
}

static void sync_cleanup(void)
{
  remove_proc_entry("sync1", NULL);
}

module_init(sync_init);
module_exit(sync_cleanup);

