#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
// #include <stdio.h>
// #include <fcntl.h>


#define DUMMY_MAJOR_NUMBER 250
#define STACK_SIZE 256

////////////////////////////////////////////////////////////////
//                                                            //
//                   File Operation Mapping                   //
//                                                            //
////////////////////////////////////////////////////////////////

/***************************************************************
// Declaration of function to be mapped to file_operations
***************************************************************/
int dummy_open(struct inode *inode, struct file *file);
int dummy_release(struct inode *inode, struct file *file);
int dummy_clean(struct file *file, fl_owner_t id);
static long dummy_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
ssize_t dummy_read(struct file *file, char *buffer, size_t length, loff_t *offset);
ssize_t dummy_write(struct file *file, const char *buffer, size_t length, loff_t *offset);

/***************************************************************
// file_operations and function mapping
***************************************************************/
static struct file_operations dummy_fops= {
        open: dummy_open,
        read: dummy_read,
        write: dummy_write,
        release: dummy_release,
		flush: dummy_clean,
		unlocked_ioctl: dummy_ioctl,
};

/***************************************************************
// Stack related structure declaration
***************************************************************/
typedef struct
{
	int head, tail;
	char item[STACK_SIZE];
} ST_t;

static char *device_buf = NULL; // buffer in user area

////////////////////////////////////////////////////////////////
//                                                            //
//          Stack-related function definitions                //
//                                                            //
////////////////////////////////////////////////////////////////

/***************************************************************
// Initialize the head and tail of the Stack
***************************************************************/
static inline void InitST(ST_t *s) 
{
 	s->head = s->tail = 0;
}

/***************************************************************
// Check if the stack buffer is full
***************************************************************/
static inline int IsFull(ST_t *s) 
{
	return ( (s->tail+1)%STACK_SIZE == s->head );
}

/***************************************************************
// Check if the stack buffer is empty
// Return: If fully occupied (1), if there is room (0)
***************************************************************/
static inline int IsEmpty(ST_t *s) 
{
	return (s->head == s->tail);
}

/***************************************************************
// Fill the buffer at the end of the stack with value
// Return: If not declared (-1), if fully occupied (-2)
//         If NULL character (-3), filling is successful (0)
***************************************************************/
int Instack(ST_t *s, char value)
{
	if (s == NULL) return -1;
	if (*value == NULL)	return -3;
	if (s->head >= STACK_SIZE -1) return -2;
	
	s->item[++s->head] = value;
	return 0;
}

/***************************************************************
// Passed as value in the rearmost buffer of the stack
// Return: If the address is incorrect (-1), if the stack is empty (-2)
//         Success (0)
***************************************************************/
int Destack(ST_t *s, char *value)
{
	if(s == NULL) return -1;
	if(s->head < 0) return -2;

	*value = s->item[s->head--];
	return 0;
}

////////////////////////////////////////////////////////////////
//                                                            //
//     Definition of device driver related functions          //
//                                                            //
////////////////////////////////////////////////////////////////

/***************************************************************
// Variable declaration
***************************************************************/
char device_name[20]; // array to store device name
ST_t stack_buffer;    // stack declaration
static struct cdev my_cdev; // cdev structure declaration

/***************************************************************
// Initialize device
***************************************************************/
static int __init dummy_init(void)
{
    dev_t dev=MKDEV(DUMMY_MAJOR_NUMBER,0);

    printk("Dummy Driver : init module\n");
    strcpy(device_name, "Dummy_Driver");

    register_chrdev(DUMMY_MAJOR_NUMBER, device_name, &dummy_fops); // device registration

    cdev_init(&my_cdev, &dummy_fops); // device initialization
    cdev_add(&my_cdev, dev, 128); // device addition
    InitST(&stack_buffer); // stack initialization
    return 0;
}

/***************************************************************
// Release device
***************************************************************/
static void __exit dummy_exit(void)
{
    printk("Dummy Driver : Clean Up Module\n");

    cdev_del(&my_cdev); // device remove
    unregister_chrdev_region(MKDEV(DUMMY_MAJOR_NUMBER,0),128); 	// device release
}

/***************************************************************
// Opne device
***************************************************************/
int dummy_open(struct inode *inode, struct file *file)
{
	printk("Dummy Driver : Open Call\n");
	return 0;
}

/***************************************************************
// Execute by mapping to read of file_operations
// Return: Invalid address (-1), empty stack (-2), success (0)
***************************************************************/
ssize_t dummy_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{	
	printk("Dummy Driver : Here is Read Call, Stack index[%d]\n", stack_buffer.tail);
	
	if (device_buf != NULL)
		kfree(device_buf);
	
	if ((device_buf = kmalloc(length + 1, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	// Implementing using IsEmpty, copy_to_user, and Destack functions
	if(IsEmpty(&stack_buffer)) return -2;

	Destack(&stack_buffer, device_buf);	

	if (copy_to_user(buffer, device_buf, length))	return -1;

	return 0;
}

/***************************************************************
// Execute by mapping with write of file_operations
// Return: Invalid address (-1), stack buffer full (-2), success (0)
***************************************************************/
ssize_t dummy_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	printk("Dummy Driver : Here is Write Call, Stack index[%d]\n", stack_buffer.tail);
	
	if (device_buf != NULL)
		kfree(device_buf);
	
	if ((device_buf = kmalloc(length + 1, GFP_KERNEL)) == NULL)
		return -ENOMEM;


	// Implementing using IsFull, copy_from_user, and Instack functions
    if (copy_from_user(device_buf, buffer, length)) 
        return -1; 

	if (IsFull(&stack_buffer)) 
			return -2;

    Instack(&stack_buffer, *device_buf); 
	
	return 0;
}

/***************************************************************
// Initializing the stack buffer, mapped to flush and executed
***************************************************************/
int dummy_clean(struct file *file, fl_owner_t id)
{
	int i;
	printk("Dummy Driver : Clean Call\n");
	
	for(i = 0; i < STACK_SIZE; i++)
	{
		stack_buffer.item[i] = '\0'; // empty all stack buffer to NULL
	}

	InitST(&stack_buffer); 	// initialize head and tail indices
	printk("Dummy Driver : Success clean\n");
	
	return 0;
}

/***************************************************************
// Executes by mapping with release of file_operations
***************************************************************/
int dummy_release(struct inode *inode, struct file *file)
{
        printk("Dummy Driver : Release Call\n");
        return 0;
}

/***************************************************************
// Called through device driver function ioctl
***************************************************************/
static long dummy_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
		case 0:
			printk("Dummy Driver : ioctl Call\n");

			dummy_fops.flush(file, NULL);  // call clean
		break;
	}

	return 0;
}

////////////////////////////////////////////////////////////////
//                                                            //
//        Device driver registration and release              //
//                                                            //
////////////////////////////////////////////////////////////////

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_AUTHOR("YCLEE");
MODULE_DESCRIPTION("Dummy_Driver");
MODULE_LICENSE("GPL");
