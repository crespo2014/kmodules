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



static int module_init_1(void)
{
    //proc_create("deferred_initcalls", 0, NULL, &deferred_initcalls_fops);
    return 0;
}

static void cleanup(void)
{
}

module_init(module_init_1);
module_exit(cleanup);

