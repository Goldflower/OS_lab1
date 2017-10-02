#include <linux/list.h> // doubly linked list
#include <linux/init.h>
#include <linux/kernel.h> // KERN_INFO defined here
#include <linux/module.h> // all module needed
#include <linux/slab.h> // kmalloc defined here

// struct list_head birthday_list = { &(birthday_list), &(birthday_list) }; 即建立空list
static LIST_HEAD(birthday_list);

struct birthday {
	int day;
	int month;
	int year;
	struct list_head list;
};

int birthday_list_init(void) 
{
	printk(KERN_INFO "Loading Module\n"); // if returns type is not int, you're not able to use rmmod
	struct birthday *date;
	int num_elements = 5;
	int i = 0;
	for(i = 0; i < num_elements; i++) {
		date = kmalloc(sizeof(*date), GFP_KERNEL); // 最多128KB, GFP_KERNEL:process contex, can sleep
		printk(KERN_INFO "Add element %d\n", i + 1);
		date -> day = 6 * i;
		date -> month = i + 1;
		date -> year = 2000 + i;
		
		INIT_LIST_HEAD(&date -> list); // 初始化node
		list_add_tail(&date -> list, &birthday_list); // 加入list
	}
	
	list_for_each_entry(date, &birthday_list, list) {
		printk(KERN_INFO "Birth: %d/%d/%d\n", date -> year, date -> month, date -> day);
	}

	return 0;
}	

int j = 0;
void birthday_list_exit(void)
{
	printk(KERN_INFO "Removing Module\n");
	struct birthday *date, *next;
	/**
	 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
	 * @pos:    the type * to use as a loop counter.
	 * @n:        another type * to use as temporary storage
	 * @head:    the head for your list.
	 * @member:    the name of the list_struct within the struct.
	 */
	list_for_each_entry_safe(date, next, &birthday_list, list) {
		list_del(&date -> list);
		printk(KERN_INFO "Delete element %d\n", j++);
		kfree(date);
	}
}

module_init(birthday_list_init);
module_exit(birthday_list_exit);

