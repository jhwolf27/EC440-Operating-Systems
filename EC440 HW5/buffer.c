/* Documentation macros. */

MODULE_AUTHOR("John Cena");
MODULE_DESCRIPTION("Winning the WWE Championship");
MODULE_LICENSE("WWE");

/* Standard headers */
#include <linux/module.h>       /* Needed by all LKMs */
#include <linux/kernel.h>       /* Needed for KERN_ALERT */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/fs.h>           /* for struct file_operations */
#include <asm/uaccess.h>        /* for put_user */

/*
*  Prototypes - this would normally go in a .h file
*/
static int __init kernalbuff_init_module(void);
static void __exit kernalbuff_exit_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "Buffer"    /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80              /* Max length of the message from the device */

/*
* Global variables are declared as static, so are global within the file.
*/

static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?
								* Used to prevent multiple access to device */
static char KBuffer[BUF_LEN + 1]      /* A variable that holds the very last character in the buffer. */
static char *KBuffer_Ptr;           /* a variable that points to the buffer */
static int KBuffer_Char = 50;       /* Total number of characters stored in th ebuffer */
static int ReadBytes_Count = 0;      /* Count the number of bytes that have been read since opening. */



static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*
* Functions
*/

int kernalbuff_init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk("Registering the character device failed with %d\n",
			Major);
		return Major;
	}

	printk("<1>I was assigned major number %d.  To talk to\n", Major);
	printk("<1>the driver, create a dev file with\n");
	printk("'sudo mknod /dev/buffer c %d 0'.\n", Major);
	printk("<1>Try various minor numbers.  Try to cat and echo to\n");
	printk("the device file.\n");
	printk("<1>Remove the device file and module when done.\n");

	return SUCCESS;
}

void kernalbuff_exit_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
}


/*
* Methods
*/

/*
* Called when a process tries to open the device file, like
* "cat /dev/buffer"
*/
static int device_open(struct inode *inode, struct file *file)
{
	int K, J;
	J = KBuffer_Char - 2;
	K = 0;
	
	char TEMPO;
	
	while (K < J){
		TEMPO = KBuffer[K];
		KBuffer[K] = KBuffer[J];
		KBuffer[J] = temp;
		K++; //increment
		J--; //decrement
	}

	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	KBuffer_Ptr = KBuffer;
	ReadBytes_Count = 0;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
* Called when a process closes the device file.
*/
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;          /* We're now ready for our next caller */

	/*
	* Decrement the usage count, or else once you opened the file, you'll
	* never get get rid of the module.
	*/
	module_put(THIS_MODULE);

	return 0;
}

/*
* Called when a process writes to dev file. Attempts to write to it.
*/
static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t * off)
{
	/*
	* Number of bytes actually written to the buffer
	*/
	KBuffer_Char = 0;

	/*
	* If we're at the end of the message,
	* return 0 signifying end of file
	*/
	if (KBuffer_Char >= BUF_LEN) {
		return 0;
	}

	/*
	* Actually put the data into the buffer
	*/
	while (length && (KBuffer_Char < BUF_LEN)) {

		/*
		* The buffer is in the user data segment, not the kernel
		* segment so "*" assignment won't work.  We have to use
		* put_user which copies data from the kernel data segment to
		* the user data segment.
		*/
		get_user(KBuffer_Ptr[KBuffer_Char], buffer + KBuffer_Char);

		length--;
		KBuffer_Char++;
	}

	/*
	* Most write functions return the number of bytes put into the buffer
	*/
	return KBuffer_Char;
}

/*
* Called when a process, which already opened the dev file, attempts to
* read from it. */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
	char *buffer,        /* buffer to fill with data */
	size_t length,       /* length of the buffer     */
	loff_t * offset)
{
	int already_read = ReadBytes_Count;  /* keep track of how many read already */

	/*
	* If we're at the end of the message,
	* return 0 signifying end of file
	*/
	if (ReadBytes_Count >= KBuffer_Char)
		return 0;

	/*
	* Actually put the data into the buffer
	*/
	while (length && (ReadBytes_Count < KBuffer_Char)) {

		/*
		* The buffer is in the user data segment, not the kernel
		* segment so "*" assignment won't work.  We have to use
		* put_user which copies data from the kernel data segment to
		* the user data segment.
		*/
		put_user(KBuffer_Ptr[ReadBytes_Count], buffer + ReadBytes_Count);

		length--;
		ReadBytes_Count++;
	}

	/*	
	* Most read functions return the number of bytes put into the buffer
	*/
	return ReadBytes_Count - already_read;
}

/* The macros below register the init and exit functions with the kernel */
module_init(kernalbuff_init_module);
module_exit(kernalbuff_exit_module);
