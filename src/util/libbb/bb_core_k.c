#ifdef __KERNEL__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/cdev.h>

#else

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#endif /* __KERNEL__ */


#include "bb_core.h"
#include "bb_core_k.h"
#include "bb_utils.h"

#define LOG(x, ...)

struct bb_operations k_bb_ops;

static int k_bb_msgq_get(S_BB_T * bb, int create)
{
	return BB_OK;
}

static int k_bb_lock(volatile S_BB_T * bb)
{
	return BB_OK;
}
static int k_bb_unlock(volatile S_BB_T *bb)
{
	return BB_OK;
}
static int k_bb_sem_get(S_BB_T * bb, int create)
{
	return BB_OK;
}

static int k_bb_shmem_detach(S_BB_T ** bb)
{
	return BB_OK;
}

#ifdef __KERNEL__
static int k_bb_shmem_attach(S_BB_T ** bb, const char *name)
{
	return BB_OK;
}
#else
static int k_bb_shmem_attach(S_BB_T ** bb, const char *name)
{
	int fd;
        fd = open(name, O_RDWR | O_SYNC);
        if( fd == -1) {
                printf("open error...\n");
                exit(0);
        }

        *bb = mmap(0, 1024, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
        if(*bb == MAP_FAILED) {
                printf("mmap() failed\n");
                exit(1);
        }
	return BB_OK;
}
#endif /* __KERNEL__ */

#ifdef __KERNEL__
static int allocate_bb(S_BB_T ** bb, const char *name, int n_data,
			     int data_size, int create)
{
	int shm_size;
	void *kmalloc_ptr;
	unsigned long virt_addr;

	shm_size = bb_size(n_data, data_size) + PAGE_SIZE;
	/* 
	 * create the shared memory pool. we add two pages to get 
	 * the desired amount of aligned memory on a PAGE boundary,
	 * even in the worst case
	 */
	kmalloc_ptr = kmalloc(shm_size + 2 * PAGE_SIZE, GFP_ATOMIC);
	if (!kmalloc_ptr) {
		printk("kmalloc failed\n");
		return BB_NOK;
	}
	/* Align the kmallocated area on a page boundary */
	*bb =
	    (S_BB_T *) (((unsigned long) kmalloc_ptr + PAGE_SIZE - 1)
			      & PAGE_MASK);
	/* reserve all pages to make them remapable */
	for (virt_addr = (unsigned long) *bb;
	     virt_addr < (unsigned long) *bb + shm_size;
	     virt_addr += PAGE_SIZE) {
		SetPageReserved(virt_to_page(virt_addr));
	}
        if (create) {
                /* Init the freshly allocated structure */
                memset(*bb, 0, shm_size);
                (*bb)->bb_version_id = BB_VERSION_ID;
                strncpy((*bb)->name, name,
                        BB_NAME_MAX_SIZE + 1);
                (*bb)->max_data_desc_size = n_data;
                (*bb)->data_desc_offset = sizeof(S_BB_T);

                (*bb)->n_data = 0;
                (*bb)->max_data_size = data_size;
                (*bb)->data_offset = (*bb)->data_desc_offset +
                    ((*bb)->max_data_desc_size) * sizeof(S_BB_DATADESC_T);
                (*bb)->data_free_offset = 0;
                (*bb)->status = BB_STATUS_GENUINE;
                (*bb)->priv.k.shm_size = shm_size;
                (*bb)->priv.k.kmalloc_ptr = kmalloc_ptr;
        }
        (*bb)->type = BB_KERNEL;
	return BB_OK;
}

/* Array holding the used BBs, this is usefull
 * at unload time, where we want to clean up every
 * in-use bb before leaving */
S_BB_T *present_bbs[BB_DEV_MAX];
/* Tracks the used/unused BBs */
DECLARE_BITMAP(present_devices, BB_DEV_MAX);
/* protects access to the two bb tracking tools above */
DEFINE_SPINLOCK(pdeviceslock);

static int k_bb_shmem_get(S_BB_T ** bb, const char *name, int n_data,
			     int data_size, int create)
{
	struct bb_device *dev;
	int err, index;
	dev_t devno;

	/* register and allocate a new character device */	
	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		printk("Not enough memory to allocate device\n");
		return BB_NOK;
	}

	cdev_init(&dev->cdev, &bb_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &bb_fops;

	spin_lock(&pdeviceslock);
	index = find_first_zero_bit(present_devices, BB_DEV_MAX);
	if (index >= BB_DEV_MAX) {
		printk("No BBs left :%d\n", index);
		goto err_unlock;
	}
	set_bit(index, present_devices);
	err = allocate_bb(bb, name, n_data, data_size, create);
	if (err != BB_OK) {
		printk("Cannot allocate BB\n");
		goto err_unlock;
	}
	present_bbs[index] = *bb;
	spin_unlock(&pdeviceslock);

	devno = MKDEV(bb_major, bb_minor + index);
	err = cdev_add(&dev->cdev, devno, 1);
	if(err != 0) 
		goto err_destroy_bb;
	
	printk("New bb device created major: %d, minor: %d\n",
				MAJOR(devno), MINOR(devno));
	dev->bb = *bb;
	(*bb)->priv.k.dev = dev;
	(*bb)->priv.k.index = index;

	return BB_OK;

err_destroy_bb:
	bb_destroy(bb);
err_unlock:
	spin_unlock(&pdeviceslock);
	kfree(dev);
	return BB_NOK;
}
#else
static int k_bb_shmem_get(S_BB_T ** bb, const char *name, int n_data,
			     int data_size, int create)
{
	return BB_OK;
}
#endif /* __KERNEL__ */


#ifdef __KERNEL__
static int32_t k_bb_shmem_destroy(S_BB_T ** bb)
{
	unsigned long virt_addr;

	cdev_del(&(*bb)->priv.k.dev->cdev);
	spin_lock(&pdeviceslock);
	clear_bit((*bb)->priv.k.index, present_devices);
	spin_unlock(&pdeviceslock);

	for (virt_addr = (unsigned long) *bb;
	     virt_addr < (unsigned long) *bb + (*bb)->priv.k.shm_size;
	     virt_addr += PAGE_SIZE) {
		/* clear all pages */
		ClearPageReserved(virt_to_page(virt_addr));
	}
	kfree((*bb)->priv.k.kmalloc_ptr);
	*bb = NULL;

	return BB_OK;
} /* end of k_bb_shmem_destroy */
#else
static int32_t k_bb_shmem_destroy(S_BB_T ** bb)
{
	return BB_OK;
}
#endif /* __KERNEL__ */

static int32_t k_bb_msgq_recv(volatile S_BB_T * bb, S_BB_MSG_T * msg)
{
	return BB_OK;
}

static int32_t k_bb_msgq_send(volatile S_BB_T * bb, S_BB_MSG_T * msg)
{
	return BB_OK;
}

static int32_t k_bb_msgq_isalive(S_BB_T * bb)
{
	return BB_OK;
}

static int k_bb_sem_destroy(S_BB_T *bb)
{
	return BB_OK;
}

static int k_bb_msgq_destroy(S_BB_T *bb)
{
	return BB_OK;
}

struct bb_operations k_bb_ops = {
	.bb_shmem_get = k_bb_shmem_get,
	.bb_shmem_attach = k_bb_shmem_attach,
	.bb_shmem_detach = k_bb_shmem_detach,
	.bb_shmem_destroy = k_bb_shmem_destroy,

	.bb_sem_get = k_bb_sem_get,
	.bb_lock = k_bb_lock,
	.bb_unlock = k_bb_unlock,
	.bb_sem_destroy = k_bb_sem_destroy,

	.bb_msgq_get = k_bb_msgq_get,
	.bb_msgq_send = k_bb_msgq_send,
	.bb_msgq_recv = k_bb_msgq_recv,
	.bb_msgq_isalive = k_bb_msgq_isalive,
	.bb_msgq_destroy = k_bb_msgq_destroy,
};