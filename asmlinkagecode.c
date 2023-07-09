#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/pipe_fs_i.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Chain Smoker's Problem Module");
MODULE_VERSION("0.1");

asmlinkage long sys_chain_smokers(void);
static int agent_process(void *data);
static int smoker_process(void *data);

static struct task_struct *agent_thread;
static struct task_struct *smoker_threads[3];
static int agent_ready;
static int smoker_ready[3];
static int agent_pipes[3][2];

static int agent_process(void *data) {
    while (!kthread_should_stop()) {
        if (agent_ready) {
            // Agent process acquires ingredients
            printk(KERN_INFO "Agent is putting tobacco, paper, and matches on the table.\n");
            msleep(1000);  // Simulate agent's action delay
            agent_ready = 0;
            wake_up_process(smoker_threads[0]);
        }
        msleep(100);
    }
    return 0;
}

static int smoker_process(void *data) {
    int item = *(int *)data;
    char *item_names[] = { "tobacco", "paper", "matches" };

    while (!kthread_should_stop()) {
        if (smoker_ready[item]) {
            // Smoker process checks if it can smoke
            printk(KERN_INFO "Smoker %d is waiting for %s.\n", current->pid, item_names[item]);

            // Check if the child has the required ingredient
            if (item != 0) {
                printk(KERN_INFO "Smoker %d has the required ingredient.\n", current->pid);
                printk(KERN_INFO "Smoker %d is smoking now.\n", current->pid);
                msleep(1000);  // Simulate smoking delay
            }

            smoker_ready[item] = 0;
            wake_up_process(agent_thread);
        }
        msleep(100);
    }
    return 0;
}

asmlinkage long sys_chain_smokers(void) {
    int i;

    for (i = 0; i < 3; i++) {
        if (pipe2(agent_pipes[i], O_CLOEXEC) == -1) {
            printk(KERN_ALERT "Pipe creation failed.\n");
            return -1;
        }
    }

    agent_ready = 0;
    smoker_ready[0] = 1;
    smoker_ready[1] = 0;
    smoker_ready[2] = 0;

    agent_thread = kthread_run(agent_process, NULL, "agent_thread");
    smoker_threads[0] = kthread_run(smoker_process, &smoker_ready[0], "smoker_thread0");
    smoker_threads[1] = kthread_run(smoker_process, &smoker_ready[1], "smoker_thread1");
    smoker_threads[2] = kthread_run(smoker_process, &smoker_ready[2], "smoker_thread2");

    while (!kthread_should_stop()) {
        for (i = 0; i < 3; i++) {
            if (smoker_ready[i]) {
                write_pipe(agent_pipes[i][1], "Ingredients", sizeof("Ingredients"));
                msleep(100);
            }
        }
        msleep(100);
    }

    kthread_stop(agent_thread);
    kthread_stop(smoker_threads[0]);
    kthread_stop(smoker_threads[1]);
    kthread_stop(smoker_threads[2]);

    return 0;
}

static int __init smoker_module_init(void) {
    printk(KERN_INFO "Smoker Module Loaded.\n");
    return 0;
}

static void __exit smoker_module_exit(void) {
    printk(KERN_INFO "Smoker Module Unloaded.\n");
}

module_init(smoker_module_init);
module_exit(smoker_module_exit);

