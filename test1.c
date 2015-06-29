/*
 * test1.c
 *
 *  Created on: 23 Jun 2015
 *      Author: lester
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

unsigned opened = false;

struct cmd_if {    // command interface
  unsigned wr;
  unsigned rd;
  char op[50];
};

void commandThread(struct cmd_if* cmd)
{
    char c;
    for (;;)
    {
    if (cmd->wr > cmd->rd)
    {
        c = cmd->op[cmd->rd];
        cmd->rd++;
    }
    printk("%c\n",c);
    msleep(1000);
     // i thread closing then close
    }
}

void AddCommand(struct cmd_if* cmd)
{
    if (cmd->rd != cmd->wr)
        //wait
    cmd->wr = 0;
    cmd->rd= 0;
    // copy data from user
    cmd->wr += len;
    // repeat
}

void DoCommand(struct cmd_if* cmd)
{
    int pos =0;
    for (pos=0;pos<size;++pos)
    {
        // out
        sleep(1000);
    }
}

static const struct file_operations deferred_initcalls_fops = {
   .open = device_open, //
   .read = device_read,
   //TODO .write = device_write, // write ascii 1 to do all calls in one go
};

static int module_init_1(void)
{
    proc_create("deferred_initcalls", 0, NULL, &deferred_initcalls_fops);
    return 0;
}

static void cleanup(void)
{
}

module_init(module_init_1);
module_exit(cleanup);

