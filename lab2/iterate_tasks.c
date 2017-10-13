#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>

/*
sched.h中定義以下(但並不限於)
task_struct: 包含pid, pname, state, sibling ..., circular linked-list(point to another task)
sibling: linkage in task's parent's children
for_each_process: iterate每個process
*/

void dfs(struct task_struct *task)
{
	struct task_struct *next_task;
	struct list_head *list;
    // 遇到就print, 如果要以最底層優先就擺到最後一行
    printk(KERN_INFO "pid: %d pname: %s\n", task->pid, task->comm);
    /*
    list_for_each(arg1, arg2):
        arg2 repeatedly assigned to arg1
        @ arg1: hold the current value of the list
        @ arg2: head node of the list which has to be traversed
    */
	list_for_each(list, &task->children) {
        /*
        list_entry(ptr, type, member):
            return a pointer to struct "type" that contains the "member" in list "ptr"
        */
        // sibling: name of the list_head structure in struct task_struct
        //          that corresponds to the parent's children list
		next_task = list_entry(list, struct task_struct, sibling);		
		dfs(next_task);
	}	
    
}

void linear(void)
{
    struct task_struct *task;
    // task init為init_task, 每次loop完會指向task->next直到task->next=init_task
    for_each_process(task) {
        printk(KERN_INFO "pid: %d pname: %s\n",task->pid, task->comm);
    }
}

int iterate_tasks_init(void)
{
	printk(KERN_INFO "Loading Module\n");
    printk(KERN_INFO "LINEAR:\n");
    linear();
    printk(KERN_INFO "DFS:\n");
	dfs(&init_task);
	return 0;
}	

void iterate_tasks_exit(void)
{
	printk(KERN_INFO "Removing Module\n");
}

module_init(iterate_tasks_init);
module_exit(iterate_tasks_exit);


