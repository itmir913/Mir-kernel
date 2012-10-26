/* arch/arm/mach-msm/smd_smd_mvs.c
 *    FEATURE_PANTECH_VT_SUPPORT
 *
 * SMD smd_mvs Driver -- Provides a smd_mvs SMD port interface.
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/ioctls.h>
#include <mach/msm_smd.h>
#include "smd_private.h"

#ifdef SMD_MVS_DUMP
#include <linux/fs.h>
#include "pantech_fileio.h"
#endif

#define MAX_RX_BUF_SIZE (1*1024)
#define MAX_TX_BUF_SIZE (1*1024)

static DEFINE_MUTEX(smd_mvs_ch_lock);
static DEFINE_MUTEX(smd_mvs_rx_buf_lock);
static DEFINE_MUTEX(smd_mvs_tx_buf_lock);
static DEFINE_SPINLOCK(smd_mvs_read_lock);

static DECLARE_WAIT_QUEUE_HEAD(smd_mvs_wait_queue);

struct smd_mvs_device_t {
    struct miscdevice misc;

    struct smd_channel *ch;

    unsigned char tx_buf[MAX_TX_BUF_SIZE];
    unsigned char rx_buf[MAX_RX_BUF_SIZE];
    unsigned int read_avail;
    unsigned int open_count;
    unsigned int open_flag;
    unsigned int pcm_ctl_cnt;
    unsigned char mvs_mode;
};

struct smd_mvs_device_t *smd_mvs_devp;

static void smd_mvs_notify(void *priv, unsigned event)
{
    unsigned long flags;

    switch (event) {
        case SMD_EVENT_DATA: {
            int sz;
         
            if( (smd_mvs_devp->open_count != 0) && (smd_mvs_devp->ch != 0)){    // SMD bug fix. 0811
                sz = smd_cur_packet_size(smd_mvs_devp->ch);
                if ((sz > 0) && (sz <= smd_read_avail(smd_mvs_devp->ch))) {
                    /* queue_work(smd_mvs_wq, &smd_mvs_work); */
                    spin_lock_irqsave(&smd_mvs_read_lock, flags);
                    if (smd_mvs_devp->read_avail == 0) {
                        smd_mvs_devp->read_avail = sz;
                        wake_up_interruptible(&smd_mvs_wait_queue);
                    }
                    spin_unlock_irqrestore(&smd_mvs_read_lock, flags);
                }
            }
            break;
        }
        case SMD_EVENT_OPEN:
            smd_mvs_devp->open_flag =1;
            printk(KERN_INFO "smd_mvs: smd opened\n");
            break;
        case SMD_EVENT_CLOSE:
            printk(KERN_INFO "smd_mvs: smd closed\n");
            break;
    }
}

static ssize_t smd_mvs_read(struct file *fp, char __user *buf,
			 size_t count, loff_t *pos)
{
    int r = 0;
    int bytes_read = 0;
    unsigned long flags;
    int sz, avail;

    //printk(KERN_INFO "smd_mvs_read (%d)\n", count);
    if(count > MAX_RX_BUF_SIZE)   count = MAX_RX_BUF_SIZE;
        
    for (;;) {
        mutex_lock(&smd_mvs_rx_buf_lock);
  
        if( (smd_mvs_devp->open_count == 0) || (smd_mvs_devp->ch == 0)
            || (smd_mvs_devp->open_flag==0) || (count <=0)){
            mutex_unlock(&smd_mvs_rx_buf_lock);
            return 0;
        }

        spin_lock_irqsave(&smd_mvs_read_lock, flags);
        smd_mvs_devp->read_avail = 0;
        spin_unlock_irqrestore(&smd_mvs_read_lock, flags);

        sz = smd_cur_packet_size(smd_mvs_devp->ch);

        if (sz > MAX_RX_BUF_SIZE) {    // garbage  data
            smd_read(smd_mvs_devp->ch, 0, sz);
            mutex_unlock(&smd_mvs_rx_buf_lock);
            continue;
        }
        avail = smd_read_avail(smd_mvs_devp->ch);
        
        if((sz == avail) && (sz != 0)){
            bytes_read = smd_read(smd_mvs_devp->ch, smd_mvs_devp->rx_buf, sz);

            if(bytes_read > count) {
                bytes_read = count;
            }
            break;
        }else if( sz < avail ){  // garbage data 
            smd_read(smd_mvs_devp->ch, 0, avail);
            mutex_unlock(&smd_mvs_rx_buf_lock);
            continue;    
        }
        // else is data not ready wait data. 

        mutex_unlock(&smd_mvs_rx_buf_lock);
        r = wait_event_interruptible_timeout(smd_mvs_wait_queue,
						 smd_mvs_devp->read_avail,HZ/10); /* 100ms timeout*/
        if (r <= 0) {    // timeout or error
            /* qualify error message */
            if (r != -ERESTARTSYS) {
                /* we get this anytime a signal comes in */
                printk(KERN_ERR "ERROR:%s:%i:%s: "
                            "wait_event_interruptible ret %i\n",
                                __FILE__,
                                __LINE__,
                                __func__,
                                r
                                );
            }
            return 0; //r;
        }
    }

    if(smd_mvs_devp->mvs_mode == 0)
    {
      printk(KERN_INFO "Ignore smd_read. mvs mode closed. read cnt %d\n",bytes_read);
      bytes_read = 0;
    }
    else
    {
      r = copy_to_user(buf, smd_mvs_devp->rx_buf, bytes_read);
      if (r > 0) {
          printk(KERN_ERR "ERROR:%s:%i:%s: "
                 "copy_to_user could not copy %i bytes.\n",
                 __FILE__,
                 __LINE__,
                 __func__,
                 r);
          return r;
      }
    }
    mutex_unlock(&smd_mvs_rx_buf_lock);

    return bytes_read;
}

#ifdef SMD_MVS_DUMP
typedef struct file *fs_handle_type;
#endif
static ssize_t smd_mvs_write(struct file *file,
				const char __user *buf,
				size_t count,
				loff_t *ppos)
{
    int ret=0; 
    int avail=0;
    int len=0;

    //printk(KERN_INFO "smd_mvs_write (%d)\n", count);        
    if(count > MAX_TX_BUF_SIZE)   count = MAX_TX_BUF_SIZE;

    mutex_lock(&smd_mvs_tx_buf_lock);

    // SMD bug fix. 0811
    if( (smd_mvs_devp->open_count == 0) || (smd_mvs_devp->ch == 0)
        ||(smd_mvs_devp->open_flag ==0)|| (count <=0)){
        mutex_unlock(&smd_mvs_tx_buf_lock);
        return 0;
    }

    ret = copy_from_user(smd_mvs_devp->tx_buf, buf, count);
    if (ret > 0) {
        printk(KERN_ERR "ERROR:%s:%i:%s: "
               "copy_from_user could not copy %i bytes.\n",
               __FILE__,
               __LINE__,
               __func__,
               ret);
        mutex_unlock(&smd_mvs_tx_buf_lock);
        return ret;
    }

    /* mvs control data */
    /*if(count==3) printk(KERN_INFO "-------smd_mvs_write (tx_buf[0]=%d)(tx_buf[1]=%d)(tx_buf[2]=%d)\n", 
        smd_mvs_devp->tx_buf[0], smd_mvs_devp->tx_buf[1], smd_mvs_devp->tx_buf[2]);*/
    
    if((count == 3) && (smd_mvs_devp->tx_buf[0] == 1) && (smd_mvs_devp->tx_buf[1] == 0))
    {
      if((smd_mvs_devp->tx_buf[2] == 'P') || (smd_mvs_devp->tx_buf[2] == 'A')) /* PCM or AMR open */
      {
        smd_mvs_devp->pcm_ctl_cnt++;
        //printk(KERN_INFO "$$$smd_mvs_write open- pcm_ctl_cnt=%d\n", smd_mvs_devp->pcm_ctl_cnt);
        if(smd_mvs_devp->pcm_ctl_cnt > 1)
        {
          if(smd_mvs_devp->mvs_mode == smd_mvs_devp->tx_buf[2])
          {
            mutex_unlock(&smd_mvs_tx_buf_lock);
            printk(KERN_INFO "Ignore smd_write. mvs control open. open cnt %d\n",smd_mvs_devp->pcm_ctl_cnt);
            return 0;
          }
          else if(smd_mvs_devp->mvs_mode != 0)
          {
            unsigned char mvs_control[3] = {1,0,0}; /* MVS close data */

            smd_write(smd_mvs_devp->ch, mvs_control, 3);

            smd_mvs_devp->mvs_mode = 0;
            printk(KERN_INFO "MVS mode changeded. send mvs close\n");
            msleep(10);
          }
        }
        smd_mvs_devp->mvs_mode = smd_mvs_devp->tx_buf[2];
      }
      else if((smd_mvs_devp->tx_buf[2] == 0) && (smd_mvs_devp->pcm_ctl_cnt > 0))  /* PCM or AMR close */
      {
        smd_mvs_devp->pcm_ctl_cnt--;
        //printk(KERN_INFO "$$$smd_mvs_write close- pcm_ctl_cnt=%d\n", smd_mvs_devp->pcm_ctl_cnt);
        if(smd_mvs_devp->pcm_ctl_cnt > 0)
        {
          mutex_unlock(&smd_mvs_tx_buf_lock);
          printk(KERN_INFO "Ignore smd_write. mvs control close. close cnt %d\n",smd_mvs_devp->pcm_ctl_cnt);
          return 0;
        }
        smd_mvs_devp->mvs_mode = 0;
      }
    }
    else if(smd_mvs_devp->mvs_mode == 0)
    {
      mutex_unlock(&smd_mvs_tx_buf_lock);
      printk(KERN_INFO "Ignore smd_write. mvs mode closed. wr cnt%d\n",count);
      return 0;
    }
    
    avail = smd_write_avail(smd_mvs_devp->ch);
    if(avail < count){
        len =avail;
    } else{
        len =count;
    }
    if(len <= 0){  
        mutex_unlock(&smd_mvs_tx_buf_lock);
        return 0;
    }
    
    ret = smd_write(smd_mvs_devp->ch, smd_mvs_devp->tx_buf, len);

#ifdef SMD_MVS_DUMP
    if(count>3)
    {
            fs_handle_type file;
            file = pantech_fopen("data/rx_pcm_rec.bin", O_WRONLY | O_CREAT | O_APPEND , S_IRUSR | S_IWUSR );
		if(file != NULL) {
                    pantech_fwrite(file, &smd_mvs_devp->tx_buf[2], 160/*len*/);
                    pantech_fclose(file);
		}
    }
#endif
    mutex_unlock(&smd_mvs_tx_buf_lock);
    //if(count==3) printk(KERN_INFO "smd_mvs_write - smd_write success!\n");    
    return ret;
}

static int smd_mvs_open(struct inode *ip, struct file *fp)
{
    int r = 0;
    int try_count =0;
    
    printk(KERN_INFO "smd_mvs_open cnt %d\n",smd_mvs_devp->open_count);

    mutex_lock(&smd_mvs_ch_lock);
    if ((smd_mvs_devp->ch == 0) || (smd_mvs_devp->open_count == 0)) {
        r = smd_open("DATA10", &smd_mvs_devp->ch,0, smd_mvs_notify);
        while( (r == 0) && (smd_mvs_devp->open_flag ==0)){
            if(100 < try_count++){
                r = -ENODEV;
                break;
            }
            msleep(10);
        }
        smd_mvs_devp->pcm_ctl_cnt = 0;
        smd_mvs_devp->mvs_mode = 0;
    }

    if( r ==  0){    // opened success or already opened.
        smd_mvs_devp->open_count++;
    }
    //printk(KERN_INFO "$$$smd_mvs_open- open_count=%d\n", smd_mvs_devp->open_count);
    if(smd_mvs_devp->open_count == 3)
    {
      unsigned char mvs_control[3] = {1,0,0}; /* MVS close data */

      mutex_lock(&smd_mvs_tx_buf_lock);
      smd_write(smd_mvs_devp->ch, mvs_control, 3);

      smd_mvs_devp->mvs_mode = 0;
      mutex_unlock(&smd_mvs_tx_buf_lock);

      printk(KERN_INFO "MVS other mode opened. send mvs close\n");
      msleep(10);
    }
    mutex_unlock(&smd_mvs_ch_lock);
    return r;
}

static int smd_mvs_release(struct inode *ip, struct file *fp)
{
    int r = 0;
    int sz;
    unsigned long flags;

    printk(KERN_INFO "smd_mvs_release %d\n",smd_mvs_devp->open_count);
    
    mutex_lock(&smd_mvs_ch_lock);
    
    smd_mvs_devp->open_count--;
    
    if ((smd_mvs_devp->open_count == 0) && (smd_mvs_devp->ch != 0)) {

        mutex_lock(&smd_mvs_rx_buf_lock);
  
        spin_lock_irqsave(&smd_mvs_read_lock, flags);
        smd_mvs_devp->read_avail = 0;
        spin_unlock_irqrestore(&smd_mvs_read_lock, flags);

        sz = smd_cur_packet_size(smd_mvs_devp->ch);

        while((sz != 0) && (sz <= smd_read_avail(smd_mvs_devp->ch))) {
            if (sz > MAX_RX_BUF_SIZE) {
                smd_read(smd_mvs_devp->ch, smd_mvs_devp->rx_buf, MAX_RX_BUF_SIZE);
                sz= sz -MAX_RX_BUF_SIZE;
            }
            else{
                smd_read(smd_mvs_devp->ch, smd_mvs_devp->rx_buf, sz);
                sz =0;
            }
        }
        mutex_unlock(&smd_mvs_rx_buf_lock);

        if(smd_mvs_devp->mvs_mode != 0)
        {
          unsigned char mvs_control[3] = {1,0,0}; /* MVS close data */

          mutex_lock(&smd_mvs_tx_buf_lock);
          smd_write(smd_mvs_devp->ch, mvs_control, 3);
          mutex_unlock(&smd_mvs_tx_buf_lock);

          smd_mvs_devp->mvs_mode = 0;
          printk(KERN_INFO "MVS mode opened. send mvs close\n");
        }

        r = smd_close(smd_mvs_devp->ch);
        smd_mvs_devp->ch = 0;
        smd_mvs_devp->open_flag=0;
    }
    mutex_unlock(&smd_mvs_ch_lock);

    return r;
}

static int smd_mvs_ioctl(struct inode *inode,
		    struct file *file,
		    unsigned int cmd,
		    unsigned long arg)
{
    int ret;

    // SMD bug fix. 0811
    if((smd_mvs_devp->open_count == 0) ||( smd_mvs_devp->ch == 0)) return -1;

    switch (cmd) {
        case TIOCMGET:
            ret = smd_tiocmget(smd_mvs_devp->ch);
            break;
        case TIOCMSET:
            ret = smd_tiocmset(smd_mvs_devp->ch, arg, ~arg);
            break;
        default:
            ret = -1;
    }

    return ret;
}

static struct file_operations smd_mvs_fops = {
    .owner = THIS_MODULE,
    .read = smd_mvs_read,
    .write = smd_mvs_write,
    .open = smd_mvs_open,
    .release = smd_mvs_release,
    .ioctl = smd_mvs_ioctl,
};

static struct smd_mvs_device_t smd_mvs_device = {
    .misc = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "smd_mvs",
        .fops = &smd_mvs_fops,
    }
};

static void __exit smd_mvs_exit(void)
{
    misc_deregister(&smd_mvs_device.misc);
}

static int __init smd_mvs_init(void)
{
    int ret;

    smd_mvs_devp = &smd_mvs_device;
    
    smd_mvs_devp->ch =0;
    smd_mvs_devp->open_count =0;
    smd_mvs_devp->open_flag =0;
    smd_mvs_devp->pcm_ctl_cnt =0;
    smd_mvs_devp->mvs_mode = 0;
    
    ret = misc_register(&smd_mvs_device.misc);
    return ret;
}

module_init(smd_mvs_init);
module_exit(smd_mvs_exit);

MODULE_DESCRIPTION("MSM Shared Memory smd_mvs Driver");
MODULE_LICENSE("GPL v2");
