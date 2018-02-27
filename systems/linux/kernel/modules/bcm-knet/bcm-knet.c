/*
 * $Id: bcm-knet.c 1.35.2.3 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 */

/*
 * This module implements a Linux network driver for Broadcom
 * XGS switch devices. The driver simultaneously serves a
 * number of vitual Linux network devices and a Tx/Rx API
 * implemented in user space.
 *
 * Packets received from the switch device are sent to either
 * a virtual Linux network device or the user mode Rx API
 * based on a set of packet filters.
 *
 * Packets from the virtual Linux network devices and the user
 * mode Tx API are multiplexed with priority given to the Tx API.
 *
 * A message-based IOCTL interface is used for managing packet
 * filters and virtual Linux network interfaces.
 *
 * A virtual network interface can be configured to work in RCPU
 * mode, which means that packets from the switch device will
 * be encasulated with a RCPU header and a block of meta data
 * that basically contains the core DCB information. Likewise,
 * packets received from the Linux network stack are assumed to
 * be RCPU encapsulated when going out on an interface in RCPU
 * mode.
 *
 * For a list of supported module parameters, please see below.
 */

#include <gmodule.h> /* Must be included first */
#include <linux-bde.h>
#include <kcom.h>
#include <bcm-knet.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/seq_file.h>


MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Network Device Driver for Broadcom BCM TxRx API");
MODULE_LICENSE("GPL");

static int debug;
LKM_MOD_PARAM(debug, "i", int, 0);
MODULE_PARM_DESC(debug,
"Debug level (default 0)");

static char *mac_addr = NULL;
LKM_MOD_PARAM(mac_addr, "s", charp, 0);
MODULE_PARM_DESC(mac_addr,
"Ethernet MAC address (default 02:10:18:xx:xx:xx)");

static int rx_buffer_size = 9216;
LKM_MOD_PARAM(rx_buffer_size, "i", int, 0);
MODULE_PARM_DESC(rx_buffer_size,
"Size of RX packet buffers (default 9216)");

static char *base_dev_name = NULL;
LKM_MOD_PARAM(base_dev_name, "s", charp, 0);
MODULE_PARM_DESC(base_dev_name,
"Base device name (default bcm0, bcm1, etc.)");

static int rcpu_mode = 0;
LKM_MOD_PARAM(rcpu_mode, "i", int, 0);
MODULE_PARM_DESC(rcpu_mode,
"Enable RCPU encapsulation (default 0)");

static char *rcpu_dmac = NULL;
LKM_MOD_PARAM(rcpu_dmac, "s", charp, 0);
MODULE_PARM_DESC(rcpu_dmac,
"RCPU destination MAC address (by default use L2 destination MAC address)");

static char *rcpu_smac = NULL;
LKM_MOD_PARAM(rcpu_smac, "s", charp, 0);
MODULE_PARM_DESC(rcpu_smac,
"RCPU source MAC address (by default use L2 source MAC address)");

static int rcpu_ethertype = 0xde08;
LKM_MOD_PARAM(rcpu_ethertype, "i", int, 0);
MODULE_PARM_DESC(rcpu_ethertype,
"RCPU EtherType (default DE08h)");

static int rcpu_signature = 0xb840;
LKM_MOD_PARAM(rcpu_signature, "i", int, 0);
MODULE_PARM_DESC(rcpu_signature,
"RCPU Signature (default B840h)");

static int rcpu_vlan = 1;
LKM_MOD_PARAM(rcpu_vlan, "i", int, 0);
MODULE_PARM_DESC(rcpu_vlan,
"RCPU VLAN ID (default 1)");

static int use_rx_skb = 0;
LKM_MOD_PARAM(use_rx_skb, "i", int, 0);
MODULE_PARM_DESC(use_rx_skb,
"Use socket buffers for receive operation (default 0)");

static int num_rx_prio = 1;
LKM_MOD_PARAM(num_rx_prio, "i", int, 0);
MODULE_PARM_DESC(num_rx_prio,
"Number of filter priorities per Rx DMA channel");

static int check_rcpu_signature = 0;
LKM_MOD_PARAM(check_rcpu_signature, "i", int, 0);
MODULE_PARM_DESC(check_rcpu_signature,
"Check RCPU Signature for Tx packets from RCPU interfaces");

/* Debug levels */
#define DBG_LVL_VERB    0x1
#define DBG_LVL_DCB     0x2
#define DBG_LVL_PKT     0x4
#define DBG_LVL_SKB     0x8
#define DBG_LVL_CMD     0x10
#define DBG_LVL_EVT     0x20
#define DBG_LVL_IRQ     0x40
#define DBG_LVL_NAPI    0x80
#define DBG_LVL_PDMP    0x100
#define DBG_LVL_FLTR    0x200
#define DBG_LVL_KCOM    0x400
#define DBG_LVL_RCPU    0x800

#define DBG_VERB(_s)    do { if (debug & DBG_LVL_VERB) gprintk _s; } while (0)
#define DBG_DCB(_s)     do { if (debug & DBG_LVL_DCB)  gprintk _s; } while (0)
#define DBG_PKT(_s)     do { if (debug & DBG_LVL_PKT)  gprintk _s; } while (0)
#define DBG_SKB(_s)     do { if (debug & DBG_LVL_SKB)  gprintk _s; } while (0)
#define DBG_CMD(_s)     do { if (debug & DBG_LVL_CMD)  gprintk _s; } while (0)
#define DBG_EVT(_s)     do { if (debug & DBG_LVL_EVT)  gprintk _s; } while (0)
#define DBG_IRQ(_s)     do { if (debug & DBG_LVL_IRQ)  gprintk _s; } while (0)
#define DBG_NAPI(_s)    do { if (debug & DBG_LVL_NAPI) gprintk _s; } while (0)
#define DBG_PDMP(_s)    do { if (debug & DBG_LVL_PDMP) gprintk _s; } while (0)
#define DBG_FLTR(_s)    do { if (debug & DBG_LVL_FLTR) gprintk _s; } while (0)
#define DBG_KCOM(_s)    do { if (debug & DBG_LVL_KCOM) gprintk _s; } while (0)
#define DBG_RCPU(_s)    do { if (debug & DBG_LVL_RCPU) gprintk _s; } while (0)

/* Module Information */
#define MODULE_MAJOR 122
#define MODULE_NAME "linux-bcm-knet"

/*
 * Only the old device-dependent NAPI interface is currently
 * supported and it should only be enabled if the BCM Rx API
 * is not used.
 */
#ifndef NAPI_SUPPORT
#define NAPI_SUPPORT 0
#endif

#if NAPI_SUPPORT

static int use_napi = 0;
LKM_MOD_PARAM(use_napi, "i", int, 0);
MODULE_PARM_DESC(use_napi,
"Use NAPI interface (default 0)");

#else

static int use_napi = 0;

#endif

/*
 * If proxy support is compiled in the module will attempt to use
 * the user/kernel message service provided by the linux-uk-proxy
 * kernel module, otherwise device IOCTL will be used.
 */
#ifndef PROXY_SUPPORT
#define PROXY_SUPPORT 1
#endif

#if PROXY_SUPPORT

#include <linux-uk-proxy.h>

static int use_proxy = 1;
LKM_MOD_PARAM(use_proxy, "i", int, 0);
MODULE_PARM_DESC(use_proxy,
"Use Linux User/Kernel proxy (default 1)");

#define PROXY_SERVICE_CREATE(_s,_q,_f)  linux_uk_proxy_service_create(_s,_q,_f)
#define PROXY_SERVICE_DESTROY(_s)       linux_uk_proxy_service_destroy(_s); 
#define PROXY_SEND(_s,_m,_l)            linux_uk_proxy_send(_s,_m,_l)
#define PROXY_RECV(_s,_m,_l)            linux_uk_proxy_recv(_s,_m,_l)

#else

static int use_proxy = 0;

#define PROXY_SERVICE_CREATE(_s,_q,_f)
#define PROXY_SERVICE_DESTROY(_s)
#define PROXY_SEND(_s,_m,_l)
#define PROXY_RECV(_s,_m,_l) (-1)

#endif

/* Compatibility */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
#define skb_copy_to_linear_data(_skb, _pkt, _len) \
    eth_copy_and_sum(_skb, _pkt, _len, 0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#define SKB_PADTO(_skb,_len) (((_skb = skb_padto(_skb,_len)) == NULL) ? -1 : 0)
#else
#define SKB_PADTO(_skb,_len) skb_padto(_skb,_len)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
#define skb_header_cloned(_skb) \
    skb_cloned(_skb)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,27)
static inline void *netdev_priv(struct net_device *dev)
{
        return dev->priv;
}
#endif /* KERNEL_VERSION(2,4,27) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
/* Special check for MontaVista 2.4.20 MIPS */
#if !(defined(MAX_USER_RT_PRIO) && defined(CONFIG_MIPS))
static inline void free_netdev(struct net_device *dev)
{
        kfree(dev);
}
#endif
#if NAPI_SUPPORT
static inline void netif_poll_disable(struct net_device *dev)
{
        while (test_and_set_bit(__LINK_STATE_RX_SCHED, &dev->state)) {
                /* No hurry. */
                current->state = TASK_INTERRUPTIBLE;
                schedule_timeout(1);
        }
}

static inline void netif_poll_enable(struct net_device *dev)
{
        clear_bit(__LINK_STATE_RX_SCHED, &dev->state);
}
#endif /* NAPI_SUPPORT */
#endif /* KERNEL_VERSION(2,4,23) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,21)
static struct sk_buff *skb_pad(struct sk_buff *skb, int pad)
{
        struct sk_buff *nskb;
        
        /* If the skbuff is non linear tailroom is always zero.. */
        if(skb_tailroom(skb) >= pad)
        {
                memset(skb->data+skb->len, 0, pad);
                return skb;
        }
        
        nskb = skb_copy_expand(skb, skb_headroom(skb), skb_tailroom(skb) + pad, GFP_ATOMIC);
        kfree_skb(skb);
        if(nskb)
                memset(nskb->data+nskb->len, 0, pad);
        return nskb;
}       
static inline struct sk_buff *skb_padto(struct sk_buff *skb, unsigned int len)
{
        unsigned int size = skb->len;
        if(likely(size >= len))
                return skb;
        return skb_pad(skb, len-size);
}
#endif /* KERNEL_VERSION(2,4,21) */

#ifndef ETH_P_FCOE
#define ETH_P_FCOE              0x8906
#endif

/* RCPU Protocol Types */
#ifndef PM_ETH_TYPE
#define PM_ETH_TYPE             ETH_P_802_3
#endif
#ifndef PM_FC_TYPE
#define PM_FC_TYPE              ETH_P_FCOE
#endif

/* RCPU operations */
#define RCPU_OPCODE_RX          0x10
#define RCPU_OPCODE_TX          0x20

/* RCPU flags */
#define RCPU_F_MODHDR           0x4

/* RCPU encapsulation */
#define RCPU_HDR_SIZE           32
#define RCPU_TX_META_SIZE       32
#define RCPU_TX_ENCAP_SIZE      (RCPU_HDR_SIZE + RCPU_TX_META_SIZE)
#define RCPU_RX_META_SIZE       64
#define RCPU_RX_ENCAP_SIZE      (RCPU_HDR_SIZE + RCPU_RX_META_SIZE)

static ibde_t *kernel_bde = NULL;

/* Descriptor info */
typedef struct bkn_desc_info_s {
    uint32_t *dcb_mem;
    dma_addr_t dcb_dma;
    struct sk_buff *skb;
    dma_addr_t skb_dma;
} bkn_desc_info_t;

/* DCB chain info */
typedef struct bkn_dcb_chain_s {
    struct list_head list;
    int dcb_cnt;
    int dcb_cur;
    uint32_t *dcb_mem;
    dma_addr_t dcb_dma;
} bkn_dcb_chain_t;

#define MAX_TX_DCBS 32
#define MAX_RX_DCBS 32

#define NUM_RX_CHAN 2
#define API_RX_CHAN 0

#define POLL_INTERVAL (HZ / 10)

/* Device control info */
typedef struct bkn_switch_info_s {
    struct list_head list;
    volatile void *base_addr;   /* Base address for PCI register access */
    struct pci_dev *pdev;       /* Required for DMA memory control */
    struct net_device *dev;     /* Base network device */
    struct timer_list timer;    /* Retry/resource timer */
    spinlock_t lock;            /* Main lock for device */
    int dev_no;                 /* Device number (from BDE) */
    int dcb_wsize;              /* DCB size (in 32-bit words) */
    uint32_t irq_mask;          /* Active IRQs for DMA control */
    void *dcb_mem;              /* Logical pointer to DCB memory */
    dma_addr_t dcb_dma;         /* Physical bus address for DCB memory */
    int dcb_mem_size;           /* Total size of allocated DCB memory */
    uint32_t dma_events;        /* DMA events pending for BCM API */
    struct {
        bkn_desc_info_t desc[MAX_TX_DCBS+1];
        int free;               /* Number of free Tx DCBs */
        int cur;                /* Index of current Tx DCB */
        int prev;               /* Index of previous Tx DCB (for chaining) */
        int dirty;              /* Index of next Tx DCB to complete */
        int api_active;         /* BCM Tx API is in progress */
        int suspends;           /* Calls to netif_stop_queue (debug only) */
        struct semaphore sem;   /* Lock required to start Tx DMA */
        struct list_head api_dcb_list; /* Tx DCB chains from BCM Tx API */
        unsigned long pkts;     /* Packet counter */
    } tx;
    struct {
        bkn_desc_info_t desc[MAX_RX_DCBS+1];
        int free;               /* Number of free Rx DCBs */
        int cur;                /* Index of current Rx DCB */
        int prev;               /* Index of previous Rx DCB (for chaining) */
        int dirty;              /* Index of next Rx DCB to complete */
        int running;            /* Rx DMA is active */
        int all_done;           /* No more completed Rx DCBs */
        int api_active;         /* BCM Rx API is active */
        struct list_head api_dcb_list; /* Rx DCB chains from BCM Rx API */
        bkn_dcb_chain_t *api_dcb_chain; /* Current Rx DCB chain */
        unsigned long pkts;     /* Packet counter */
    } rx[NUM_RX_CHAN];
} bkn_switch_info_t;

#define DEV_READ32(_d, _a, _p) \
    do { \
        *(_p) = (((volatile uint32_t *)(_d)->base_addr)[(_a)/4]); \
    } while(0)

#define DEV_WRITE32(_d, _a, _v) \
    do { \
        ((volatile uint32_t *)(_d)->base_addr)[(_a)/4] = (_v); \
    } while(0)

#define MEMORY_BARRIER mb()

/* Default random MAC address has Broadcom OUI with local admin bit set */
static u8 bkn_dev_mac[6] = { 0x02, 0x10, 0x18, 0x00, 0x00, 0x00 };

static u8 bkn_rcpu_dmac[6];
static u8 bkn_rcpu_smac[6];

/* Driver Proc Entry root */
static struct proc_dir_entry *bkn_proc_root = NULL;

typedef struct bkn_priv_s {
    struct list_head list;
    struct net_device_stats stats; 
    struct net_device *dev;
    bkn_switch_info_t *sinfo;
    int id;
    int type;
    int port;
    uint32_t vlan;
    uint32_t flags;
} bkn_priv_t;

typedef struct bkn_filter_s {
    struct list_head list;
    int dev_no;
    unsigned long hits;
    kcom_filter_t kf;
} bkn_filter_t;

static wait_queue_head_t evt_wq;
static int evt_wq_put;
static int evt_wq_get;

/* Switch devices */
LIST_HEAD(_sinfo_list);

/* Net devices */
LIST_HEAD(_ndev_list);

/* Packet filters */
LIST_HEAD(_filter_list);

/* Free API buffers */
LIST_HEAD(_buf_free_list);

/*
 * Thread management
 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))
/*
 * Old style using kernel_thread()
 */
typedef struct {
    const char * name;
    volatile int pid;
    volatile int run;
    struct completion completion;
    int state;
} bkn_thread_ctrl_t;

static int
bkn_thread_start(bkn_thread_ctrl_t *tc, const char *name,
                 int (*threadfn)(void *))
{
    if (name == NULL) {
        return -1;
    }
    tc->name = name;
    tc->pid = kernel_thread(threadfn, tc, 0);
    if (tc->pid < 0) {
        tc->pid = 0;
        return -1;
    }
    tc->run = 1;
    init_completion(&tc->completion);
    return 0;
}

static int
bkn_thread_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->pid == 0) {
        return 0;
    }
    tc->run = 0;
    kill_proc(tc->pid, SIGTERM, 1);
    wait_for_completion(&tc->completion);
    return 0;
}

static int
bkn_thread_should_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->run) {
        return 0;
    }
    tc->pid = 0;
    return 1;
}

static void
bkn_thread_boot(bkn_thread_ctrl_t *tc)
{
    siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));
}

static void
bkn_thread_exit(bkn_thread_ctrl_t *tc)
{
    complete_and_exit(&tc->completion, 0);
}

static void
bkn_sleep(int clicks)
{
    wait_queue_head_t wq;

    init_waitqueue_head(&wq);
    sleep_on_timeout(&wq, clicks);
}
#else
/*
 * New style using kthread API
 */
#include <linux/kthread.h>
typedef struct {
    const char * name;
    struct task_struct *task;
    int state;
} bkn_thread_ctrl_t;

static int
bkn_thread_start(bkn_thread_ctrl_t *tc, const char *name,
                 int (*threadfn)(void *))
{
    if (name == NULL) {
        return -1;
    }
    tc->name = name;
    tc->task = kthread_run(threadfn, tc, name);
    if (IS_ERR(tc->task)) {
        tc->task = NULL;
        return -1;
    }
    return 0;
}

static int
bkn_thread_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->task == NULL) {
        return 0;
    }
    send_sig(SIGTERM, tc->task, 0);
    return kthread_stop(tc->task);
}

static int
bkn_thread_should_stop(bkn_thread_ctrl_t *tc)
{
    return kthread_should_stop();
}

static void
bkn_thread_boot(bkn_thread_ctrl_t *tc)
{
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
}

static void
bkn_thread_exit(bkn_thread_ctrl_t *tc)
{
}

static void
bkn_sleep(int clicks)
{
    wait_queue_head_t wq;

    init_waitqueue_head(&wq);
    wait_event_timeout(wq, 0, clicks);
}
#endif

static bkn_thread_ctrl_t bkn_cmd_ctrl;
static bkn_thread_ctrl_t bkn_evt_ctrl;

/*
 * On XGS devices bit 15 fo the Transferred Bytes field in
 * the DCBs is used to indicate that kernel processing is
 * complete. Using this bit reduces the theoretically maximum
 * supported packet size from 64K to 32K, but this is still
 * adequate for 16K jumbo packets.
 */
#define SOC_DCB_KNET_DONE       0x8000
#define SOC_DCB_KNET_COUNT_MASK 0x7fff

/* Default channel configuration */
#define XGS_DMA_TX_CHAN         0
#define XGS_DMA_RX_CHAN         1

/* CMIC registers */
#define CMIC_DMA_CTRLr          0x00000100
#define CMIC_DMA_STATr          0x00000104
#define CMIC_DMA_DESC0r         0x00000110
#define CMIC_IRQ_STATr          0x00000144
#define CMIC_IRQ_MASKr          0x00000148
#define CMIC_DEV_REV_IDr        0x00000178

/* CMIC interrupts reserved for kernel handler */
#define TXRX_IRQ_MASK           0x7f80

/*
 * DMA_STAT: control bits
 *
 *  xxx_SET and xxx_CLR can be WRITTEN to CMIC_DMA_STAT
 *  xxx_TST can be masked against values read from CMIC_DMA_STAT.
 *  Argument required: 0 <= ch <= 3
 */

#define DS_DMA_ACTIVE(ch)               (0x00040000 << (ch))
#define DS_DMA_EN_SET(ch)               (0x80|(ch))
#define DS_DMA_EN_CLR(ch)               (0x00|(ch))
#define DS_DMA_EN_TST(ch)               (0x00000001 << (ch))

#define DS_CHAIN_DONE_SET(ch)           (0x80|(4+(ch)))
#define DS_CHAIN_DONE_CLR(ch)           (0x00|(4+(ch)))
#define DS_CHAIN_DONE_TST(ch)           (0x00000010 << (ch))

#define DS_DESC_DONE_SET(ch)            (0x80|(8+(ch)))
#define DS_DESC_DONE_CLR(ch)            (0x00|(8+(ch)))
#define DS_DESC_DONE_TST(ch)            (0x00000100 << (ch))

#define DC_ABORT_DMA(ch)                (0x04 << (8 * (ch)))


/*
 * Per-channel operations. 
 * These are the basis for the TX/RX functions
 */

static int
xgs_dma_clear_chan(bkn_switch_info_t *sinfo, int chan)
{
    uint32_t dma_stat;

    dma_stat = DS_DMA_EN_CLR(chan);
    DEV_WRITE32(sinfo, CMIC_DMA_STATr, dma_stat);

    dma_stat = DS_CHAIN_DONE_CLR(chan);
    DEV_WRITE32(sinfo, CMIC_DMA_STATr, dma_stat);

    dma_stat = DS_DESC_DONE_CLR(chan);
    DEV_WRITE32(sinfo, CMIC_DMA_STATr, dma_stat);

    MEMORY_BARRIER; 

    return 0; 
}

int 
xgs_dma_chan_init(bkn_switch_info_t *sinfo, int chan, int dir)
{
    uint32_t cdc; 

    DEV_READ32(sinfo, CMIC_DMA_CTRLr, &cdc); 

    cdc &= ~(0x9 << (8 * chan));
    if (dir) {
        cdc |= 0x1 << (8 * chan);
    } else {
        cdc |= 0x8 << (8 * chan);
    }

    DEV_WRITE32(sinfo, CMIC_DMA_CTRLr, cdc); 

    return 0; 
}

int 
xgs_dma_chan_start(bkn_switch_info_t *sinfo, int chan, dma_addr_t dcb)
{
    /* Write the DCB address to the DESC address for this channel */
    DEV_WRITE32(sinfo, CMIC_DMA_DESC0r + 4 * chan, dcb);
    
    MEMORY_BARRIER; 

    /* Kick it off */
    DEV_WRITE32(sinfo, CMIC_DMA_STATr, DS_DMA_EN_SET(chan)); 

    MEMORY_BARRIER; 

    return 0; 
}

int
xgs_dma_chan_abort(bkn_switch_info_t *sinfo, int chan, int polls)
{
    uint32_t ctrl, dma_stat; 
    int p; 
    
    /* Clear enable */
    DEV_WRITE32(sinfo, CMIC_DMA_STATr, DS_DMA_EN_CLR(chan)); 

    MEMORY_BARRIER; 
    
    /* Abort the channel */
    DEV_READ32(sinfo, CMIC_DMA_CTRLr, &ctrl); 
    DEV_WRITE32(sinfo, CMIC_DMA_CTRLr, ctrl | DC_ABORT_DMA(chan)); 

    MEMORY_BARRIER; 
    
    /* Poll for abort completion */
    for (p = 0; p < polls; p++) {
        DEV_READ32(sinfo, CMIC_DMA_STATr, &dma_stat);
        if (!(dma_stat & DS_DMA_ACTIVE(chan))) {
            /* Restore previous control value */            
            DEV_WRITE32(sinfo, CMIC_DMA_CTRLr, ctrl); 

            MEMORY_BARRIER; 
            
            /* Clear up channel */
            xgs_dma_clear_chan(sinfo, chan); 
            
            return polls; 
        }
    }
    DBG_VERB(("DMA channel %d abort failed\n", chan));

    return -1; 
}

static void
xgs_irq_mask_enable(bkn_switch_info_t *sinfo, uint32_t mask, int update_hw)
{
    sinfo->irq_mask |= mask;

    if (update_hw) {
        lkbde_irq_mask_set(sinfo->dev_no | LKBDE_ISR2_DEV, CMIC_IRQ_MASKr,
                           sinfo->irq_mask, TXRX_IRQ_MASK);
    }
}

static void
xgs_irq_mask_disable(bkn_switch_info_t *sinfo, uint32_t mask, int update_hw)
{
    sinfo->irq_mask &= ~mask;

    if (update_hw) {
        lkbde_irq_mask_set(sinfo->dev_no | LKBDE_ISR2_DEV, CMIC_IRQ_MASKr,
                           sinfo->irq_mask, TXRX_IRQ_MASK);
    }
}


static int
bkn_alloc_dcbs(bkn_switch_info_t *sinfo)
{
    int dcb_size;
    int tx_ring_size, rx_ring_size;

    dcb_size = sinfo->dcb_wsize * sizeof(uint32_t);
    tx_ring_size = dcb_size * (MAX_TX_DCBS + 1);
    rx_ring_size = dcb_size * (MAX_RX_DCBS + 1);
    sinfo->dcb_mem_size = tx_ring_size + rx_ring_size;

    sinfo->dcb_mem = pci_alloc_consistent(sinfo->pdev,
                                          sinfo->dcb_mem_size,
                                          &sinfo->dcb_dma);
    if (sinfo->dcb_mem == NULL) {
        gprintk("DCB memory allocation (%d bytes) failed.\n",
                sinfo->dcb_mem_size);
        return -ENOMEM;
    }

    return 0;
}

static void
bkn_free_dcbs(bkn_switch_info_t *sinfo)
{
    pci_free_consistent(sinfo->pdev, sinfo->dcb_mem_size,
                        sinfo->dcb_mem, sinfo->dcb_dma);
}

static void
bkn_clean_tx_dcbs(bkn_switch_info_t *sinfo)
{
    bkn_desc_info_t *desc;

    DBG_DCB(("Cleaning Tx DCBs (%d %d).\n",
             sinfo->tx.cur, sinfo->tx.dirty));
    while (sinfo->tx.free < MAX_TX_DCBS) {
        desc = &sinfo->tx.desc[sinfo->tx.dirty];
        if (desc->skb != NULL) {
            DBG_SKB(("Cleaning Tx SKB from DCB %d.\n",
                     sinfo->tx.dirty));
            pci_unmap_single(sinfo->pdev, desc->skb_dma, desc->skb->len,
                             PCI_DMA_TODEVICE);
            desc->skb_dma = 0;
            dev_kfree_skb(desc->skb);
            desc->skb = NULL;
        }
        if (++sinfo->tx.dirty >= MAX_TX_DCBS) {
            sinfo->tx.dirty = 0;
        }
        sinfo->tx.free++;
    }
    DBG_DCB(("Cleaned Tx DCBs (%d %d).\n",
             sinfo->tx.cur, sinfo->tx.dirty));
}

static void
bkn_clean_rx_dcbs(bkn_switch_info_t *sinfo, int chan)
{
    bkn_desc_info_t *desc;

    DBG_DCB(("Cleaning Rx%d DCBs (%d %d).\n",
             chan, sinfo->rx[chan].cur, sinfo->rx[chan].dirty));
    while (sinfo->rx[chan].free) {
        desc = &sinfo->rx[chan].desc[sinfo->rx[chan].dirty];
        if (desc->skb != NULL) {
            DBG_SKB(("Cleaning Rx%d SKB from DCB %d.\n",
                     chan, sinfo->rx[chan].dirty));
            pci_unmap_single(sinfo->pdev, desc->skb_dma, desc->skb->len,
                             PCI_DMA_FROMDEVICE);
            desc->skb_dma = 0;
            dev_kfree_skb(desc->skb);
            desc->skb = NULL;
        }
        if (++sinfo->rx[chan].dirty >= MAX_RX_DCBS) {
            sinfo->rx[chan].dirty = 0;
        }
        sinfo->rx[chan].free--;
    }
    sinfo->rx[chan].running = 0;
    sinfo->rx[chan].api_active = 0;
    DBG_DCB(("Cleaned Rx%d DCBs (%d %d).\n",
             chan, sinfo->rx[chan].cur, sinfo->rx[chan].dirty));
}

static void
bkn_clean_dcbs(bkn_switch_info_t *sinfo)
{
    int chan;

    bkn_clean_tx_dcbs(sinfo);

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        bkn_clean_rx_dcbs(sinfo, chan);
    }
}

static void
bkn_init_dcbs(bkn_switch_info_t *sinfo)
{
    int dcb_size;
    uint32_t *dcb_mem;
    dma_addr_t dcb_dma;
    bkn_desc_info_t *desc;
    int idx;
    int chan;

    memset(sinfo->dcb_mem, 0, sinfo->dcb_mem_size);

    dcb_size = sinfo->dcb_wsize * sizeof(uint32_t);
    dcb_mem = sinfo->dcb_mem;
    dcb_dma = sinfo->dcb_dma;

    for (idx = 0; idx < (MAX_TX_DCBS + 1); idx++) {
        desc = &sinfo->tx.desc[idx];
        desc->dcb_mem = dcb_mem;
        desc->dcb_dma = dcb_dma;
        dcb_mem += sinfo->dcb_wsize;
        dcb_dma += dcb_size;
    }
    sinfo->tx.free = MAX_TX_DCBS;
    sinfo->tx.prev = MAX_TX_DCBS;

    DBG_DCB(("Tx DCBs @ 0x%08x.\n",
             (uint32_t)sinfo->tx.desc[0].dcb_dma));

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        for (idx = 0; idx < (MAX_RX_DCBS + 1); idx++) {
            desc = &sinfo->rx[chan].desc[idx];
            desc->dcb_mem = dcb_mem;
            desc->dcb_dma = dcb_dma;
            dcb_mem += sinfo->dcb_wsize;
            dcb_dma += dcb_size;
        }
        sinfo->rx[chan].free = 0;
        sinfo->rx[chan].prev = MAX_RX_DCBS;

        DBG_DCB(("Rx%d DCBs @ 0x%08x.\n",
                 chan, (uint32_t)sinfo->rx[chan].desc[0].dcb_dma));
    }
}

static void
bkn_dump_dcb(char *prefix, uint32_t *dcb, int wsize)
{
    DBG_DCB(("%s: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x ... 0x%08x\n", prefix,
             dcb[0], dcb[1], dcb[2], dcb[3], dcb[4], dcb[5], dcb[wsize - 1]));
}

static void
bkn_dump_pkt(uint8_t *data, int size)
{
    int idx;
    char str[128];

    if ((debug & DBG_LVL_PDMP) == 0) {
        return;
    }

    size = 32;

    for (idx = 0; idx < size; idx++) {
        if ((idx & 0xf) == 0) {
            sprintf(str, "%04x: ", idx);
        }
        if ((idx & 0xf) == 8) {
            sprintf(&str[strlen(str)], "- ");
        }
        sprintf(&str[strlen(str)], "%02x ", data[idx]);
        if ((idx & 0xf) == 0xf) {
            sprintf(&str[strlen(str)], "\n");
            gprintk(str);
        }
    }
    if ((idx & 0xf) != 0) {
        sprintf(&str[strlen(str)], "\n");
        gprintk(str);
    }
}

static bkn_switch_info_t *
bkn_sinfo_from_unit(int unit)
{
    struct list_head *list;
    bkn_switch_info_t *sinfo;

    list_for_each(list, &_sinfo_list) {
        sinfo = (bkn_switch_info_t *)list;
        if (sinfo->dev_no == unit) {
            return sinfo;
        }
    }
    return NULL;
}

static void
bkn_api_rx(bkn_switch_info_t *sinfo)
{
    bkn_dcb_chain_t *dcb_chain;
    int chan = API_RX_CHAN;

    /* Rx API currently not supported in SKB mode */
    if (use_rx_skb) {
        return;
    }

    /* Assume that driver lock is held */
    if (!list_empty(&sinfo->rx[chan].api_dcb_list)) {
        dcb_chain = list_entry(sinfo->rx[chan].api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
        sinfo->rx[chan].api_dcb_chain = dcb_chain;
        sinfo->rx[chan].api_active = 1;
        DBG_DCB(("Start API Rx DMA, first DCB @ 0x%08x (%d DCBs).\n",
                 (uint32_t)dcb_chain->dcb_dma, dcb_chain->dcb_cnt));

        xgs_dma_clear_chan(sinfo, XGS_DMA_RX_CHAN + chan); 
        xgs_irq_mask_enable(sinfo, 0x600 << (2 * chan), 1);
        xgs_dma_chan_start(sinfo, XGS_DMA_RX_CHAN + chan,
                           dcb_chain->dcb_dma);

        list_del(&dcb_chain->list);
    }
}

static void
bkn_rx_refill(bkn_switch_info_t *sinfo, int chan)
{
    struct sk_buff *skb;
    bkn_desc_info_t *desc;
    uint32_t *dcb;

    if (use_rx_skb == 0) {
        /* Rx buffers are provided by BCM Rx API */
        return;
    }

    while (sinfo->rx[chan].free < MAX_RX_DCBS) {
        desc = &sinfo->rx[chan].desc[sinfo->rx[chan].cur];
        if (desc->skb == NULL) {
            skb = dev_alloc_skb(rx_buffer_size + RCPU_RX_ENCAP_SIZE);
            if (skb == NULL) {
                break;
            }
            skb_reserve(skb, RCPU_RX_ENCAP_SIZE);
            desc->skb = skb;
        } else {
            DBG_DCB(("Refill Rx%d SKB in DCB %d recycled.\n",
                      chan, sinfo->rx[chan].cur));
        }
        skb = desc->skb;
        desc->skb_dma = pci_map_single(sinfo->pdev, skb->data, rx_buffer_size,
                                       PCI_DMA_FROMDEVICE);
        DBG_DCB(("Refill Rx%d DCB %d (0x%08x).\n",
                 chan, sinfo->rx[chan].cur, (uint32_t)desc->skb_dma));
        dcb = desc->dcb_mem;
        memset(dcb, 0, sizeof(uint32_t) * sinfo->dcb_wsize);
        dcb[0] = desc->skb_dma;
        dcb[1] = rx_buffer_size;

        if (sinfo->rx[chan].prev < (MAX_RX_DCBS - 1)) {
            sinfo->rx[chan].desc[sinfo->rx[chan].prev].dcb_mem[1] |= (1 << 16);
        }
        sinfo->rx[chan].prev = sinfo->rx[chan].cur;
        if (++sinfo->rx[chan].cur >= MAX_RX_DCBS) {
            sinfo->rx[chan].cur = 0;
        }
        sinfo->rx[chan].free++;
    }
}

static void
bkn_rx_restart(bkn_switch_info_t *sinfo, int chan, int enable_intr)
{
    bkn_desc_info_t *desc;

    if (sinfo->rx[chan].running == 0 && sinfo->rx[chan].free) {
        desc = &sinfo->rx[chan].desc[sinfo->rx[chan].dirty];
        /* Do not restart chain if we have pending DCBs */
        if (desc->dcb_mem[sinfo->dcb_wsize-1] & (1 << 31)) {
            return;
        }
        DBG_DCB(("Restart Rx%d DMA, DCB @ 0x%08x (%d).\n",
                 chan, (uint32_t)desc->dcb_dma, sinfo->rx[chan].dirty));
        xgs_dma_clear_chan(sinfo, XGS_DMA_RX_CHAN + chan);
        xgs_irq_mask_enable(sinfo, 0x600 << (2 * chan), enable_intr);
        xgs_dma_chan_start(sinfo, XGS_DMA_RX_CHAN + chan, desc->dcb_dma);
        sinfo->rx[chan].running = 1;
    }
}

static int
bkn_dma_init(bkn_switch_info_t *sinfo)
{
    int chan;

    xgs_dma_chan_init(sinfo, XGS_DMA_TX_CHAN, 1);

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        xgs_dma_chan_init(sinfo, XGS_DMA_RX_CHAN + chan, 0);
        bkn_rx_refill(sinfo, chan);
        bkn_rx_restart(sinfo, chan, 1);
    }

    return 0;
}

static int
bkn_dma_abort_tx(bkn_switch_info_t *sinfo)
{
    bkn_dcb_chain_t *dcb_chain;

    DBG_VERB(("Aborting Tx DMA.\n"));

    xgs_irq_mask_disable(sinfo, 0x100, 1);

    xgs_dma_chan_abort(sinfo, XGS_DMA_TX_CHAN, 10000);

    while (!list_empty(&sinfo->tx.api_dcb_list)) {
	dcb_chain = list_entry(sinfo->tx.api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
	list_del(&dcb_chain->list);
        DBG_DCB(("Freeing Tx DCB chain.\n"));
        kfree(dcb_chain);
    }

    return 0;
}

static int
bkn_dma_abort_rx(bkn_switch_info_t *sinfo, int chan)
{
    bkn_dcb_chain_t *dcb_chain;

    DBG_VERB(("Aborting Rx%d DMA.\n", chan));

    xgs_irq_mask_disable(sinfo, 0x600 << (2 * chan), 1);

    xgs_dma_chan_abort(sinfo, XGS_DMA_RX_CHAN + chan, 10000);

    if (sinfo->rx[chan].api_dcb_chain) {
        DBG_DCB(("Freeing active Rx%d DCB chain.\n", chan));
        kfree(sinfo->rx[chan].api_dcb_chain);
        sinfo->rx[chan].api_dcb_chain = NULL;
    }
    while (!list_empty(&sinfo->rx[chan].api_dcb_list)) {
	dcb_chain = list_entry(sinfo->rx[chan].api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
	list_del(&dcb_chain->list);
        DBG_DCB(("Freeing Rx%d DCB chain.\n", chan));
        kfree(dcb_chain);
    }

    return 0;
}

static int
bkn_dma_abort(bkn_switch_info_t *sinfo)
{
    int chan;

    bkn_dma_abort_tx(sinfo);

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        bkn_dma_abort_rx(sinfo, chan);
    }

    return 0;
}

static void
bkn_timer(unsigned long context)
{
    bkn_switch_info_t *sinfo = (bkn_switch_info_t *)context;
    unsigned long flags;
    int chan;
    int restart;

    restart = 0;
    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        if (sinfo->rx[chan].free == 0) {
            spin_lock_irqsave(&sinfo->lock, flags);
            bkn_rx_refill(sinfo, chan);
            spin_unlock_irqrestore(&sinfo->lock, flags);
        }
        if (sinfo->rx[chan].free > 0) {
#if NAPI_SUPPORT
            /* Schedule NAPI poll to restart Rx */
            netif_rx_schedule(sinfo->dev);
#endif
        } else {
            restart = 1;
        }
    }
    if (restart) {
        /* Presumably still out of memory */
        sinfo->timer.expires = jiffies + POLL_INTERVAL;
        add_timer(&sinfo->timer);
    }
}

static void
bkn_destroy_sinfo(bkn_switch_info_t *sinfo)
{
    del_timer_sync(&sinfo->timer);
    list_del(&sinfo->list);
    bkn_free_dcbs(sinfo);
    kfree(sinfo);
}

static bkn_switch_info_t *
bkn_create_sinfo(int dev_no)
{
    bkn_switch_info_t *sinfo;
    uint32_t val;
    int chan;

    if ((sinfo = kmalloc(sizeof(*sinfo), GFP_KERNEL)) == NULL) {
        return NULL;
    }
    memset(sinfo, 0, sizeof(*sinfo));
    sinfo->base_addr = lkbde_get_dev_virt(dev_no);
    sinfo->pdev = lkbde_get_hw_dev(dev_no);
    sinfo->dev_no = dev_no;

    sema_init(&sinfo->tx.sem, 1);
    spin_lock_init(&sinfo->lock);

    init_timer(&sinfo->timer);
    sinfo->timer.expires = jiffies + POLL_INTERVAL;
    sinfo->timer.data = (unsigned long)sinfo;
    sinfo->timer.function = bkn_timer;

    INIT_LIST_HEAD(&sinfo->tx.api_dcb_list);

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        INIT_LIST_HEAD(&sinfo->rx[chan].api_dcb_list);
    }

    list_add_tail(&sinfo->list, &_sinfo_list);

    DEV_READ32(sinfo, CMIC_DEV_REV_IDr, &val);
    DBG_VERB(("Check dev/rev: 0x%08x\n", val));

    return sinfo;
}

static bkn_filter_t *
bkn_match_rx_pkt(uint32_t *desc, uint8_t *pkt, int chan)
{
    struct list_head *list;
    bkn_filter_t *filter;
    kcom_filter_t scratch, *kf;
    uint8_t *oob = (uint8_t *)desc;
    int size, wsize;
    int idx, match;

    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        kf = &filter->kf;
        memcpy(&scratch.data.b[0],
               &oob[kf->oob_data_offset], kf->oob_data_size);
        memcpy(&scratch.data.b[kf->oob_data_size],
               &pkt[kf->pkt_data_offset], kf->pkt_data_size);
        size = kf->oob_data_size + kf->pkt_data_size;
        wsize = BYTES2WORDS(size);
        DBG_VERB(("Filter: size = %d (%d), data = 0x%08x, mask = 0x%08x\n",
                  size, wsize, kf->data.w[0], kf->mask.w[0]));
        for (idx = 0; idx < wsize; idx++) {
            scratch.data.w[idx] &= kf->mask.w[idx];
        }
        DBG_VERB(("Packet: orig = 0x%08x, masked = 0x%08x\n",
                  ((uint32_t*)pkt)[0], scratch.data.w[0]));
        match = 1;
        if (match) {
            if (kf->priority < (num_rx_prio * NUM_RX_CHAN)) {
                if (kf->priority < (num_rx_prio * chan) ||
                    kf->priority >= (num_rx_prio * (chan + 1))) {
                    match = 0;
                }
            }
        }
        if (match) {
            for (idx = 0; idx < wsize; idx++) {
                if (scratch.data.w[idx] != kf->data.w[idx]) {
                    match = 0;
                }
            }
        }
        if (match) {
            return filter;
        }
    }

    return NULL;
}

static int
bkn_add_rcpu_encap(bkn_switch_info_t *sinfo, struct sk_buff *skb, uint32_t *dcb)
{
    int pktlen = skb->len;
    uint32_t *metadata;
    int idx;

    /* Add and clear RCPU encapsulation */
    skb_push(skb, RCPU_RX_ENCAP_SIZE);
    memset(skb->data, 0, RCPU_RX_ENCAP_SIZE);

    /* RCPU Header */
    memcpy(skb->data, &skb->data[RCPU_RX_ENCAP_SIZE], 12);
    if (rcpu_dmac != NULL) {
        memcpy(skb->data, bkn_rcpu_dmac, 6);
    }
    if (rcpu_smac != NULL) {
        memcpy(&skb->data[6], bkn_rcpu_smac, 6);
    }
    skb->data[12] = 0x81;
    skb->data[14] = rcpu_vlan >> 8;
    skb->data[15] = rcpu_vlan & 0xff;
    skb->data[16] = rcpu_ethertype >> 8;
    skb->data[17] = rcpu_ethertype & 0xff;
    skb->data[18] = rcpu_signature >> 8;
    skb->data[19] = rcpu_signature & 0xff;
    skb->data[20] = RCPU_OPCODE_RX;
    skb->data[21] = RCPU_F_MODHDR;
    skb->data[24] = pktlen >> 8;
    skb->data[25] = pktlen & 0xff;

    /* Meta data */
    metadata = (uint32_t *)&skb->data[RCPU_HDR_SIZE];
    for (idx = 0; idx < sinfo->dcb_wsize - 3; idx++) {
        metadata[idx] = htonl(dcb[idx+2]);
    }

    return 0;
}

static int
bkn_do_rx(bkn_switch_info_t *sinfo, int chan, int budget)
{
    struct list_head *list;
    bkn_priv_t *priv;
    bkn_dcb_chain_t *dcb_chain;
    bkn_desc_info_t *desc;
    struct sk_buff *skb;
    bkn_filter_t *filter;
    uint32_t *dcb;
    uint8_t *pkt;
    int ethertype;
    int dcbs_done;
    int pktlen;
    int drop_api;
    int found;
    int idx;

    if (use_rx_skb == 0) {
        /* Rx buffers are provided by BCM Rx API */
        dcb_chain = sinfo->rx[chan].api_dcb_chain;
        while (dcb_chain->dcb_cur < dcb_chain->dcb_cnt) {
            dcb = &dcb_chain->dcb_mem[dcb_chain->dcb_cur * sinfo->dcb_wsize];
            DBG_VERB(("DCB %2d: 0x%08x\n",
                      dcb_chain->dcb_cur, dcb[sinfo->dcb_wsize-1]));
            if ((dcb[sinfo->dcb_wsize-1] & (1 << 31)) == 0) {
                break;
            }
            sinfo->rx[chan].pkts++;
            pkt = (uint8_t *)kernel_bde->p2l(sinfo->dev_no, dcb[0]);
            bkn_dump_pkt(pkt, 32);
            if ((dcb[sinfo->dcb_wsize-1] & 0x70000) == 0x30000) {
                filter = bkn_match_rx_pkt(dcb, pkt, chan);
            } else {
                /* Fragment or error */
                filter = NULL;
            }
            drop_api = 1;
            if (filter) {
                DBG_FLTR(("Match filter ID %d\n", filter->kf.id));
                filter->hits++;
                switch (filter->kf.dest_type) {
                case KCOM_DEST_T_API:
                    DBG_FLTR(("Send to Rx API\n"));
                    drop_api = 0;
                    break;
                case KCOM_DEST_T_NETIF:
                    found = 0;
                    priv = NULL;
                    list_for_each(list, &_ndev_list) {
                        priv = (bkn_priv_t *)list;
                        if (priv->id == filter->kf.dest_id) {
                            found = 1;
                            break;
                        }
                    }
                    if (found && priv) {
                        pktlen = dcb[sinfo->dcb_wsize-1] & 0x7fff;
                        skb = dev_alloc_skb(pktlen + RCPU_RX_ENCAP_SIZE);
                        if (skb == NULL) {
                            break;
                        }
                        skb_reserve(skb, RCPU_RX_ENCAP_SIZE);

                        DBG_FLTR(("Send to netif %d (%s)\n",
                                  priv->id, priv->dev->name));
                        skb->dev = priv->dev;
                        skb_reserve(skb, 2);	/* 16 byte align the IP fields. */

                        /* Save for RCPU before stripping tag */
                        ethertype = (pkt[16] << 8) | pkt[17];

                        if (filter->kf.flags & KCOM_FILTER_F_STRIP_TAG) {
                            /* Strip the VLAN tag */
                            DBG_FLTR(("Strip VLAN tag\n"));
                            for (idx = 11; idx >= 0; idx--) {
                                pkt[idx+4] = pkt[idx];
                            }
                            pktlen -= 4;
                            pkt += 4;
                        }
                        skb_copy_to_linear_data(skb, pkt, pktlen);
                        skb_put(skb, pktlen - 4); /* Strip CRC */
                        priv->stats.rx_packets++;
                        priv->stats.rx_bytes += pktlen;

                        if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
                            bkn_add_rcpu_encap(sinfo, skb, dcb);
                        }

                        skb->protocol = eth_type_trans(skb, skb->dev);
                        if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
                            skb->protocol = PM_ETH_TYPE;
                            if (ethertype == ETH_P_FCOE) {
                                skb->protocol = PM_FC_TYPE;
                            }
                        }

                        /* Unlock while calling up network stack */
                        spin_unlock(&sinfo->lock);
                        if (use_napi) {
#if NAPI_SUPPORT
                            netif_receive_skb(skb);
#endif
                        } else {
                            netif_rx(skb);
                        }
                        spin_lock(&sinfo->lock);

                        if (filter->kf.mirror_type == KCOM_DEST_T_API) {
                            drop_api = 0;
                        }
                    } else {
                        DBG_FLTR(("Unknown netif %d\n",
                                  filter->kf.dest_id));
                    }
                    break;
                default:
                    /* Drop packet */
                    DBG_FLTR(("Unknown dest type %d\n",
                              filter->kf.dest_type));
                    break;
                }
            }
            if (drop_api) {
                /* If count is zero, the DCB will just be recycled */
                dcb[sinfo->dcb_wsize-1] &= ~SOC_DCB_KNET_COUNT_MASK;
            }
            dcb[sinfo->dcb_wsize-1] |= SOC_DCB_KNET_DONE;
            dcb_chain->dcb_cur++;
        }
        return 0;
    }

    dcbs_done = 0;
    sinfo->rx[chan].all_done = 0;

    while (dcbs_done < budget) {
        char str[32];
        sprintf(str, "Rx DCB (%d)", sinfo->rx[chan].dirty);
        desc = &sinfo->rx[chan].desc[sinfo->rx[chan].dirty];
        dcb = desc->dcb_mem;
        bkn_dump_dcb(str, dcb, sinfo->dcb_wsize);
        if ((dcb[sinfo->dcb_wsize-1] & (1 << 31)) == 0) {
            sinfo->rx[chan].all_done = 1;
            break;
        }
        sinfo->rx[chan].pkts++;
        pktlen = dcb[sinfo->dcb_wsize-1] & 0xffff;
        priv = netdev_priv(sinfo->dev);
        skb = desc->skb;
        DBG_DCB(("Rx%d SKB DMA done (%d).\n", chan, sinfo->rx[chan].dirty));
        pci_unmap_single(sinfo->pdev, desc->skb_dma, pktlen,
                         PCI_DMA_FROMDEVICE);
        if ((dcb[sinfo->dcb_wsize-1] & 0x70000) == 0x30000) {
            DBG_PKT(("Rx packet (%d bytes).\n", pktlen));
            filter = bkn_match_rx_pkt(dcb, skb->data, chan);
        } else {
            /* Fragment or error */
            priv->stats.rx_errors++;
            filter = NULL;
        }
        if (filter) {
            DBG_FLTR(("Match filter ID %d\n", filter->kf.id));
            filter->hits++;
            switch (filter->kf.dest_type) {
            case KCOM_DEST_T_API:
                DBG_FLTR(("Send to Rx API (dropped)\n"));
                break;
            case KCOM_DEST_T_NETIF:
                found = 0;
                priv = NULL;
                list_for_each(list, &_ndev_list) {
                    priv = (bkn_priv_t *)list;
                    if (priv->id == filter->kf.dest_id) {
                        found = 1;
                        break;
                    }
                }
                if (found && priv) {
                    DBG_FLTR(("Send to netif %d (%s)\n",
                              priv->id, priv->dev->name));

                    skb_put(skb, pktlen - 4); /* Strip CRC */

                    /* Save for RCPU before stripping tag */
                    ethertype = (skb->data[16] << 8) | skb->data[17];

                    if (filter->kf.flags & KCOM_FILTER_F_STRIP_TAG) {
                        /* Strip VLAN tag */
                        DBG_FLTR(("Strip VLAN tag\n"));
                        ((u32*)skb->data)[3] = ((u32*)skb->data)[2];
                        ((u32*)skb->data)[2] = ((u32*)skb->data)[1];
                        ((u32*)skb->data)[1] = ((u32*)skb->data)[0];
                        skb_pull(skb, 4);
                        pktlen -= 4;
                    }
                    bkn_dump_pkt(skb->data, skb->len);
                    priv->stats.rx_packets++;
                    priv->stats.rx_bytes += pktlen;
                    skb->dev = priv->dev;

                    if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
                        bkn_add_rcpu_encap(sinfo, skb, dcb);
                    }

                    skb->protocol = eth_type_trans(skb, skb->dev);
                    if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
                        skb->protocol = PM_ETH_TYPE;
                        if (ethertype == ETH_P_FCOE) {
                            skb->protocol = PM_FC_TYPE;
                        }
                    }

                    /* Unlock while calling up network stack */
                    spin_unlock(&sinfo->lock);
                    if (use_napi) {
#if NAPI_SUPPORT
                        netif_receive_skb(skb);
#endif
                    } else {
                        netif_rx(skb);
                    }
                    spin_lock(&sinfo->lock);

                    /* Ensure that we reallocate SKB for this DCB */
                    desc->skb = NULL;
                    desc->skb_dma = 0;
                }
            }
        } else {
            DBG_PKT(("Rx packet dropped.\n"));
            priv->stats.rx_dropped++;
        }
        dcb[sinfo->dcb_wsize-1] = 0;
        if (++sinfo->rx[chan].dirty >= MAX_RX_DCBS) {
            sinfo->rx[chan].dirty = 0;
        }
        sinfo->rx[chan].free--;
        dcbs_done++;
    }

    bkn_rx_refill(sinfo, chan);

#if NAPI_SUPPORT
    if (use_napi) {
        if (sinfo->rx[chan].all_done) {
            /* Unlock while calling up network stack */
            spin_unlock(&sinfo->lock);
            netif_rx_complete(sinfo->dev);
            spin_lock(&sinfo->lock);
            /* Re-enable Rx interrupts */
            xgs_irq_mask_enable(sinfo, 0x600 << (2 * chan), 1);
        }
    }
#endif

    if (sinfo->rx[chan].free == 0) {
        /* Presumably out of memory */
        sinfo->timer.expires = jiffies + POLL_INTERVAL;
        add_timer(&sinfo->timer);
    } else {
        bkn_rx_restart(sinfo, chan, !use_napi);
    }

    return dcbs_done;
}

static void
bkn_rx_desc_done(bkn_switch_info_t *sinfo, int chan)
{
    DBG_IRQ(("Rx%d desc done\n", chan));

    if (use_napi) {
#if NAPI_SUPPORT
        /* Schedule NAPI poll */
        DBG_NAPI(("Schedule Rx poll on %s.\n", sinfo->dev->name));
        /* Unlock while calling up network stack */
        spin_unlock(&sinfo->lock);
	if (netif_rx_schedule_prep(sinfo->dev)) {
            DBG_NAPI(("Schedule prep OK on %s.\n", sinfo->dev->name));
            __netif_rx_schedule(sinfo->dev);
            /* Disable Rx interrupts until poll job is complete */
            xgs_irq_mask_disable(sinfo, 0x600 << (2 * chan), 0);
        }
        spin_lock(&sinfo->lock);
#endif
    } else {
        bkn_do_rx(sinfo, chan, MAX_RX_DCBS);
    }
    sinfo->dma_events |= KCOM_DMA_INFO_F_RX_DONE;
    evt_wq_put++;
    wake_up_interruptible(&evt_wq);
}

static void
bkn_rx_chain_done(bkn_switch_info_t *sinfo, int chan)
{
    /* All work done by bkn_rx_desc_done */
    DBG_IRQ(("Rx%d chain done\n", chan));
    if (sinfo->rx[chan].api_dcb_chain) {
        kfree(sinfo->rx[chan].api_dcb_chain);
        sinfo->rx[chan].api_dcb_chain = NULL;
    }
    sinfo->rx[chan].api_active = 0;
    if (use_rx_skb == 0) {
        bkn_api_rx(sinfo);
    } else {
        bkn_rx_restart(sinfo, chan, !use_napi);
    }
}

static void
bkn_suspend_tx(bkn_switch_info_t *sinfo)
{
    struct list_head *list;
    bkn_priv_t *priv = netdev_priv(sinfo->dev);

    /* Unlock while calling up network stack */
    spin_unlock(&sinfo->lock);
    /* Stop main device */
    netif_stop_queue(priv->dev);
    sinfo->tx.suspends++;
    /* Stop associated virtual devices */
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (priv->sinfo->dev_no != sinfo->dev_no) {
            continue;
        }
        netif_stop_queue(priv->dev);
    }
    spin_lock(&sinfo->lock);
}

static void
bkn_resume_tx(bkn_switch_info_t *sinfo)
{
    struct list_head *list;
    bkn_priv_t *priv;

    /* Check main device */
    if (netif_queue_stopped(sinfo->dev) && sinfo->tx.free) {
        /* Unlock while calling up network stack */
        spin_unlock(&sinfo->lock);
        netif_wake_queue(sinfo->dev);
        spin_lock(&sinfo->lock);
    }
    /* Check associated virtual devices */
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (priv->sinfo->dev_no != sinfo->dev_no) {
            continue;
        }
        if (netif_queue_stopped(priv->dev) && sinfo->tx.free) {
            /* Unlock while calling up network stack */
            spin_unlock(&sinfo->lock);
            netif_wake_queue(priv->dev);
            spin_lock(&sinfo->lock);
        }
    }
}

static void
bkn_do_tx(bkn_switch_info_t *sinfo)
{
    bkn_desc_info_t *desc;
    int dcbs_done = 0;

    while (dcbs_done < MAX_TX_DCBS) {
        char str[32];
        if (sinfo->tx.free == MAX_TX_DCBS) {
            break;
        }
        sprintf(str, "Tx DCB (%d)", sinfo->tx.dirty);
        desc = &sinfo->tx.desc[sinfo->tx.dirty];
        bkn_dump_dcb(str, desc->dcb_mem, sinfo->dcb_wsize);
        if ((desc->dcb_mem[sinfo->dcb_wsize-1] & (1 << 31)) == 0) {
            break;
        }
        if (desc->skb) {
            DBG_DCB(("Tx SKB DMA done (%d).\n", sinfo->tx.dirty));
            pci_unmap_single(sinfo->pdev, desc->skb_dma, desc->skb->len,
                             PCI_DMA_TODEVICE);
            dev_kfree_skb_irq(desc->skb);
            desc->skb = NULL;
            desc->skb_dma = 0;
        }
        desc->dcb_mem[sinfo->dcb_wsize-1] = 0;
        if (++sinfo->tx.dirty >= MAX_TX_DCBS) {
            sinfo->tx.dirty = 0;
        }
        if (++sinfo->tx.free > MAX_TX_DCBS) {
            DBG_VERB(("Too many free Tx DCBs(%d).\n", sinfo->tx.free));
        }
        dcbs_done++;
    }

    bkn_resume_tx(sinfo);
}

static void
bkn_api_tx(bkn_switch_info_t *sinfo)
{
    bkn_dcb_chain_t *dcb_chain;

    /* Assume that driver lock is held */
    if (list_empty(&sinfo->tx.api_dcb_list)) {
        sinfo->tx.api_active = 0;
    } else {
        sinfo->tx.pkts++;
        sinfo->tx.api_active = 1;
        dcb_chain = list_entry(sinfo->tx.api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
        DBG_DCB(("Start API Tx DMA, first DCB @ 0x%08x (%d DCBs).\n",
                 (uint32_t)dcb_chain->dcb_dma, dcb_chain->dcb_cnt));

        xgs_dma_clear_chan(sinfo, XGS_DMA_TX_CHAN); 
        xgs_irq_mask_enable(sinfo, 0x100, 1);
        xgs_dma_chan_start(sinfo, XGS_DMA_TX_CHAN,
                           dcb_chain->dcb_dma);

        list_del(&dcb_chain->list);
        kfree(dcb_chain);
    }
}

static void
bkn_tx_chain_done(bkn_switch_info_t *sinfo)
{
    bkn_desc_info_t *desc;

    DBG_IRQ(("Tx chain done (%d/%d)\n", sinfo->tx.cur, sinfo->tx.dirty));

    if (sinfo->tx.api_active) {
        sinfo->dma_events |= KCOM_DMA_INFO_F_TX_DONE;
        evt_wq_put++;
        wake_up_interruptible(&evt_wq);
        /* Check if BCM API has more to send */
        bkn_api_tx(sinfo);
    }

    if (sinfo->tx.free == MAX_TX_DCBS) {
        /* If netif Tx is idle then allow BCM API to send */
        bkn_api_tx(sinfo);
    } else {
        desc = &sinfo->tx.desc[sinfo->tx.dirty];
        DBG_DCB(("Restart Tx DMA, DCB @ 0x%08x (%d).\n",
                 (uint32_t)desc->dcb_dma, sinfo->tx.dirty));
        xgs_dma_clear_chan(sinfo, XGS_DMA_TX_CHAN); 
        xgs_irq_mask_enable(sinfo, 0x100, 0);
        xgs_dma_chan_start(sinfo, XGS_DMA_TX_CHAN, desc->dcb_dma);
    }
}

static void
bkn_isr(void *isr_data)
{
    bkn_switch_info_t *sinfo = isr_data;
    uint32_t irq_stat;
    uint32_t dma_stat;
    int chan;

    spin_lock(&sinfo->lock);

    DEV_READ32(sinfo, CMIC_IRQ_STATr, &irq_stat);
    if ((irq_stat & sinfo->irq_mask) == 0) {
        /* Not ours */
        spin_unlock(&sinfo->lock);
        return;
    }

    lkbde_irq_mask_set(sinfo->dev_no | LKBDE_ISR2_DEV, CMIC_IRQ_MASKr,
                       0, TXRX_IRQ_MASK);

    DBG_IRQ(("Got interrupt on device %d (0x%08x)\n",
             sinfo->dev_no, irq_stat));

    DEV_READ32(sinfo, CMIC_DMA_STATr, &dma_stat);

    for (chan = 0; chan < NUM_RX_CHAN; chan++) {
        if (dma_stat & DS_DESC_DONE_TST(XGS_DMA_RX_CHAN + chan)) {
            DEV_WRITE32(sinfo, CMIC_DMA_STATr,
                        DS_DESC_DONE_CLR(XGS_DMA_RX_CHAN + chan));
            bkn_rx_desc_done(sinfo, chan);
        }

        if (dma_stat & DS_CHAIN_DONE_TST(XGS_DMA_RX_CHAN + chan)) {
            DEV_WRITE32(sinfo, CMIC_DMA_STATr,
                        DS_DMA_EN_CLR(XGS_DMA_RX_CHAN + chan));
            DEV_WRITE32(sinfo, CMIC_DMA_STATr,
                        DS_CHAIN_DONE_CLR(XGS_DMA_RX_CHAN + chan));
            sinfo->rx[chan].running = 0;
            bkn_rx_chain_done(sinfo, chan);
        }
    }

    bkn_do_tx(sinfo);

    if (dma_stat & DS_CHAIN_DONE_TST(XGS_DMA_TX_CHAN)) {
        sinfo->irq_mask &= ~0x100;
        bkn_tx_chain_done(sinfo);
    }

    lkbde_irq_mask_set(sinfo->dev_no | LKBDE_ISR2_DEV, CMIC_IRQ_MASKr,
                       sinfo->irq_mask, TXRX_IRQ_MASK);

    spin_unlock(&sinfo->lock);
}

static int 
bkn_open(struct net_device *dev)
{
    netif_start_queue(dev);
    return 0;
}

static int
bkn_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 68 || new_mtu > (rx_buffer_size + RCPU_RX_ENCAP_SIZE)) {
        return -EINVAL;
    }
    dev->mtu = new_mtu;
    return 0;
}

#if NAPI_SUPPORT
static int 
bkn_poll(struct net_device *dev, int *budget)
{
    bkn_priv_t *priv = netdev_priv(dev);
    bkn_switch_info_t *sinfo = priv->sinfo;
    int cur_budget = *budget;
    int rx_dcbs_done;
    unsigned long flags;

    spin_lock_irqsave(&sinfo->lock, flags);

    DBG_NAPI(("Rx poll on %s.\n", dev->name));

    if (cur_budget > dev->quota) {
        cur_budget = dev->quota;
    }
    rx_dcbs_done = bkn_do_rx(priv->sinfo, cur_budget);
    *budget -= rx_dcbs_done;
    dev->quota -= rx_dcbs_done;

    spin_unlock_irqrestore(&sinfo->lock, flags);

    return (priv->sinfo->rx[chan].all_done) ? 0 : 1;
}
#endif

static int
bkn_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    return 0;
}	

/* 
 * Network Device Statistics. 
 * Cleared at init time.  
 */
static struct net_device_stats *
bkn_get_stats(struct net_device *dev)
{
    bkn_priv_t *priv = netdev_priv(dev);

    return &priv->stats;
}

/* Fake multicast ability */
static void
bkn_set_multicast_list(struct net_device *dev)
{
}

static int 
bkn_tx(struct sk_buff *skb, struct net_device *dev)
{
    bkn_priv_t *priv = netdev_priv(dev);
    bkn_switch_info_t *sinfo = priv->sinfo;
    struct sk_buff *new_skb;
    unsigned char *pktdata;
    int pktlen;
    int taglen;
    int rcpulen;
    int metalen;
    int sop;
    int idx;
    uint32_t *metadata;
    unsigned long flags;

    DBG_VERB(("Netif Tx\n"));

    if (priv->id <= 0) {
        /* Do not transmit on base device */
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
        return 0;
    }

    if (down_trylock(&sinfo->tx.sem) != 0) {
        DBG_VERB(("Netif Tx while suspended - dropping SKB\n"));
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
        return 0;
    }

    spin_lock_irqsave(&sinfo->lock, flags);

    if (sinfo->tx.free) {
        bkn_desc_info_t *desc = &sinfo->tx.desc[sinfo->tx.cur];
        uint32_t *dcb;

        pktdata = skb->data;
        pktlen = skb->len + 4;
        rcpulen = 0;
        sop = 0;

        if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
            rcpulen = RCPU_HDR_SIZE;
            if (skb->len < (rcpulen + 14)) {
                DBG_VERB(("Tx drop: Invalid RCPU encapsulation\n"));
                priv->stats.tx_dropped++;
                dev_kfree_skb(skb);
                spin_unlock_irqrestore(&sinfo->lock, flags);
                up(&sinfo->tx.sem);
                return 0;
            }
            if (check_rcpu_signature &&
                ((skb->data[18] << 8) | skb->data[19]) != rcpu_signature) {
                DBG_VERB(("Tx drop: Invalid RCPU signature\n"));
                priv->stats.tx_dropped++;
                dev_kfree_skb(skb);
                spin_unlock_irqrestore(&sinfo->lock, flags);
                up(&sinfo->tx.sem);
                return 0;
            }
            if (skb->data[21] & RCPU_F_MODHDR) {
                rcpulen += RCPU_TX_META_SIZE;
                sop = skb->data[RCPU_HDR_SIZE];
                if (sop != 0xff && sop != 0xfb && sop != 0xfc) {
                    DBG_VERB(("Tx drop: Invalid RCPU meta data\n"));
                    priv->stats.tx_dropped++;
                    dev_kfree_skb(skb);
                    spin_unlock_irqrestore(&sinfo->lock, flags);
                    up(&sinfo->tx.sem);
                    return 0;
                }
            }
            /* Skip over RCPU encapsulation */
            pktdata = &skb->data[rcpulen];
            pktlen -= rcpulen;

            /* CPU packets require tag */
            if (sop == 0) {
                if (pktdata[12] != 0x81 && pktdata[13] != 0x00) {
                    if (skb_header_cloned(skb)) {
                        /* Current SKB cannot be modified */
                        DBG_SKB(("Realloc Tx SKB\n"));
                        new_skb = dev_alloc_skb(pktlen + 4);
                        if (skb == NULL) {
                            DBG_VERB(("Tx drop: No SKB memory\n"));
                            priv->stats.tx_dropped++;
                            dev_kfree_skb(skb);
                            spin_unlock_irqrestore(&sinfo->lock, flags);
                            up(&sinfo->tx.sem);
                            return 0;
                        }
                        memcpy(new_skb->data, pktdata, 12);
                        memcpy(&new_skb->data[16], &pktdata[12], pktlen - 12);
                        skb_put(new_skb, pktlen + 4);
                        dev_kfree_skb(skb);
                        skb = new_skb;
                        pktdata = skb->data;
                    } else {
                        /* Add tag to existing buffer */
                        DBG_SKB(("Expand into unused RCPU header\n"));
                        pktdata = &skb->data[rcpulen-4];
                        for (idx = 0; idx < 12; idx++) {
                            pktdata[idx] = pktdata[idx+4];
                        }
                    }
                    pktdata[12] = 0x81;
                    pktdata[13] = 0x00;
                    pktdata[14] = (priv->vlan >> 8) & 0xf;
                    pktdata[15] = priv->vlan & 0xff;
                    pktlen += 4;
                }
            }
        } else if (priv->port < 0 || (priv->flags & KCOM_NETIF_F_ADD_TAG)) {
            /* Need to add VLAN tag if packet is untagged */
            if (skb->data[12] != 0x81 && skb->data[13] != 0x00) {
                if (skb_header_cloned(skb) || skb_headroom(skb) < 4) {
                    /* Current SKB cannot be modified */
                    DBG_SKB(("Realloc Tx SKB\n"));
                    new_skb = dev_alloc_skb(pktlen + 4);
                    if (skb == NULL) {
                        DBG_VERB(("Tx drop: No SKB memory\n"));
                        priv->stats.tx_dropped++;
                        dev_kfree_skb(skb);
                        spin_unlock_irqrestore(&sinfo->lock, flags);
                        up(&sinfo->tx.sem);
                        return 0;
                    }
                    memcpy(new_skb->data, skb->data, 12);
                    memcpy(&new_skb->data[16], &skb->data[12], pktlen - 12);
                    skb_put(new_skb, pktlen + 4);
                    dev_kfree_skb(skb);
                    skb = new_skb;
                } else {
                    /* Add tag to existing buffer */
                    DBG_SKB(("Expand Tx SKB\n"));
                    skb_push(skb, 4);
                    for (idx = 0; idx < 12; idx++) {
                        skb->data[idx] = skb->data[idx+4];
                    }
                }
                pktdata = skb->data;
                pktdata[12] = 0x81;
                pktdata[13] = 0x00;
                pktdata[14] = (priv->vlan >> 8) & 0xf;
                pktdata[15] = priv->vlan & 0xff;
                pktlen += 4;
            }
        }

        /* Pad packet if needed */
        taglen = 0;
        if (pktdata[12] == 0x81 && pktdata[13] == 0x00) {
            taglen = 4;
        }
        if (pktlen < (64 + taglen)) {
            pktlen = (64 + taglen);
            if (SKB_PADTO(skb, pktlen) != 0) {
                priv->stats.tx_dropped++;
                DBG_SKB(("Tx drop: skb_padto failed\n"));
                spin_unlock_irqrestore(&sinfo->lock, flags);
                up(&sinfo->tx.sem);
                return 0;
            }
            DBG_SKB(("Packet padded to %d bytes\n", pktlen));
        }

        desc->skb = skb;
        desc->skb_dma = pci_map_single(sinfo->pdev, pktdata, pktlen,
                                       PCI_DMA_TODEVICE);

        dcb = desc->dcb_mem;
        memset(dcb, 0, sizeof(uint32_t) * sinfo->dcb_wsize);
        dcb[0] = desc->skb_dma;
        dcb[1] = pktlen;
        if (priv->flags & KCOM_NETIF_F_RCPU_ENCAP) {
            /* If module header SOP is non-zero, use RCPU meta data */
            if (sop != 0) {
                metalen = (sinfo->dcb_wsize - 3) * sizeof(uint32_t);
                if (metalen > RCPU_TX_META_SIZE) {
                    metalen = RCPU_TX_META_SIZE;
                }
                metadata = (uint32_t *)&skb->data[RCPU_HDR_SIZE];
                for (idx = 0; idx < BYTES2WORDS(metalen); idx++) {
                    dcb[idx+2] = ntohl(metadata[idx]);
                }
                dcb[1] |= (1 << 19);
            }
            bkn_dump_dcb("Tx RCPU", dcb, sinfo->dcb_wsize);
        } else if (priv->port >= 0) {
            /* Send to physical port */
            dcb[1] |= (1 << 19);
            dcb[2] = 0xff000000;
            dcb[3] = 0x00000100;
            dcb[4] = priv->port;
        }

        DBG_DCB(("Add Tx DCB @ 0x%08x (%d) [%d free] (%d bytes).\n",
                 (uint32_t)desc->dcb_dma, sinfo->tx.cur,
                 sinfo->tx.free, pktlen));
        bkn_dump_pkt(pktdata, skb->len - rcpulen);

        if (sinfo->tx.free == MAX_TX_DCBS) {
            DBG_DCB(("Start Tx DMA, DCB @ 0x%08x (%d).\n",
                     (uint32_t)desc->dcb_dma, sinfo->tx.cur));
            xgs_dma_clear_chan(sinfo, XGS_DMA_TX_CHAN); 
            xgs_irq_mask_enable(sinfo, 0x100, 1);
            xgs_dma_chan_start(sinfo, XGS_DMA_TX_CHAN, desc->dcb_dma);
        } else if (sinfo->tx.prev < (MAX_TX_DCBS - 1)) {
            sinfo->tx.desc[sinfo->tx.prev].dcb_mem[1] |= (1 << 16);
        }
        sinfo->tx.prev = sinfo->tx.cur;
        if (++sinfo->tx.cur >= MAX_TX_DCBS) {
            sinfo->tx.cur = 0;
        }
        sinfo->tx.free--;

        priv->stats.tx_packets++;
        priv->stats.tx_bytes += pktlen;
        sinfo->tx.pkts++;
    }
    else {
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
    }

    /* Check our Tx resources */
    if (sinfo->tx.free < 1) {
        bkn_suspend_tx(sinfo);
    }

    dev->trans_start = jiffies;

    spin_unlock_irqrestore(&sinfo->lock, flags);
    up(&sinfo->tx.sem);

    return 0;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
static const struct net_device_ops bkn_netdev_ops = {
	.ndo_open		= bkn_open,
	.ndo_stop		= bkn_stop,
	.ndo_start_xmit		= bkn_tx,
	.ndo_get_stats		= bkn_get_stats,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_multicast_list	= bkn_set_multicast_list,
	.ndo_set_mac_address	= NULL,
	.ndo_do_ioctl		= NULL,
	.ndo_tx_timeout		= NULL,
	.ndo_change_mtu		= bkn_change_mtu,
#ifdef CONFIG_NET_POLL_CONTROLLER
#if NAPI_SUPPORT
	.ndo_poll_controller	= bkn_poll,
#endif
#endif
};
#endif

static struct net_device *
bkn_init_ndev(u8 *mac, char *name)
{
    struct net_device *dev;

    /* Create Ethernet device */
    dev = alloc_etherdev(sizeof(bkn_priv_t));

    if (dev == NULL) {
        DBG_VERB(("Error allocating Ethernet device.\n"));
        return NULL;
    }
#ifdef SET_MODULE_OWNER
    SET_MODULE_OWNER(dev);
#endif

    /* Set the device MAC address */
    memcpy(dev->dev_addr, mac, 6);

    /* Device information -- not available right now */
    dev->irq = 0;
    dev->base_addr = 0;
    dev->mtu = rx_buffer_size + RCPU_RX_ENCAP_SIZE;

    /* Device vectors */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    dev->netdev_ops = &bkn_netdev_ops;
#else
    dev->open = bkn_open;
    dev->hard_start_xmit = bkn_tx;
    dev->stop = bkn_stop;
    dev->set_multicast_list = bkn_set_multicast_list;
    dev->do_ioctl = NULL;
    dev->get_stats = bkn_get_stats;
    dev->change_mtu = bkn_change_mtu;
#if NAPI_SUPPORT
    if (use_napi) {
        dev->poll = bkn_poll;
	dev->weight = 16;
    }
#endif
#endif
    if (name && *name) {
        strncpy(dev->name, name, IFNAMSIZ-1);
    }

    /* Register the kernel Ethernet device */
    if (register_netdev(dev)) {
        DBG_VERB(("Error registering Ethernet device.\n"));
        free_netdev(dev);
        return NULL;
    }
    DBG_VERB(("Created Ethernet device %s.\n", dev->name));

    return dev;
}

/*
 * Driver DMA Proc Entry
 *
 * This output can be rather large (> PAGE_SIZE) so we use the
 * seq_file interface to do the output. Special header records
 * are indicated by a negative DCB index.
 */
typedef struct {
    int dev_no;         /* Current unit */
    int rx_dma;         /* 0: Tx DMA, 1: Rx DMA*/
    int ch_no;          /* DMA channel no. (Rx only) */
    int idx;            /* DCB index */
} bkn_seq_dma_iter_t;

/* From current record, move forward 'pos' records */
static int
bkn_seq_dma_next_pos(bkn_seq_dma_iter_t *iter, loff_t pos)
{
    while (pos) {
        if (iter->rx_dma) {
            if (++iter->idx >= MAX_RX_DCBS) {
                iter->idx = -1;
                if (++iter->ch_no >= NUM_RX_CHAN) {
                    iter->rx_dma = 0;
                    iter->ch_no = 0;
                    iter->dev_no++;
                    if (bkn_sinfo_from_unit(iter->dev_no) == NULL) {
                        return -1;
                    }
                }
            }
        } else {
            if (++iter->idx >= MAX_TX_DCBS) {
                iter->idx = -1;
                iter->rx_dma = 1;
            }
        }
        pos--;
    }
    return 0;
}

/* Initialize private data and move to requested start record */
static void *
bkn_seq_dma_start(struct seq_file *s, loff_t *pos)
{
    bkn_seq_dma_iter_t *iter;

    iter = kmalloc(sizeof(bkn_seq_dma_iter_t), GFP_KERNEL);
    if (!iter) {
        return NULL;
    }
    memset(iter, 0, sizeof(*iter));
    iter->idx = -2;
    if (bkn_seq_dma_next_pos(iter, *pos) < 0) {
        kfree(iter);
        return NULL;
    }
    return iter;
}
 
/* Move to next record */
static void *
bkn_seq_dma_next(struct seq_file *s, void *v, loff_t *pos)
{
    bkn_seq_dma_iter_t *iter = (bkn_seq_dma_iter_t *)v;
    void *rv = iter;

    if (bkn_seq_dma_next_pos(iter, 1) < 0) {
        return NULL;
    }
    (*pos)++;
    return rv;
}

/* Release private data */
static void
bkn_seq_dma_stop(struct seq_file *s, void *v)
{
    if (v) {
	kfree(v);
    }
}

/* Print current record */
static int
bkn_seq_dma_show(struct seq_file *s, void *v)
{
    bkn_seq_dma_iter_t *iter = (bkn_seq_dma_iter_t *)v;
    bkn_switch_info_t *sinfo;
    uint32_t *dcb = NULL;
    int chan;

    sinfo = bkn_sinfo_from_unit(iter->dev_no);
    if (sinfo == NULL) {
        /* Should not happen */
        return 0;
    }

    if (iter->rx_dma == 0) {
        if (iter->idx == -2) {
            seq_printf(s, "Pending events: 0x%x\n", sinfo->dma_events);
        } else if (iter->idx == -1) {
            seq_printf(s,
                       "Tx DCB info (unit %d):\n"
                       "  api:   %d\n"
                       "  dirty: %d\n"
                       "  cur:   %d\n"
                       "  free:  %d\n"
                       "  pause: %s\n",
                       iter->dev_no,
                       sinfo->tx.api_active,
                       sinfo->tx.dirty,
                       sinfo->tx.cur,
                       sinfo->tx.free,
                       netif_queue_stopped(sinfo->dev) ? "yes" : "no");
        } else {
            dcb = sinfo->tx.desc[iter->idx].dcb_mem;
        }
    } else {
        if (iter->idx == -1) {
            chan = iter->ch_no;
            seq_printf(s,
                       "Rx%d DCB info (unit %d):\n"
                       "  api:   %d\n"
                       "  dirty: %d\n"
                       "  cur:   %d\n"
                       "  free:  %d\n"
                       "  run:   %d\n",
                       chan, iter->dev_no,
                       sinfo->rx[chan].api_active,
                       sinfo->rx[chan].dirty,
                       sinfo->rx[chan].cur,
                       sinfo->rx[chan].free,
                       sinfo->rx[chan].running);
        } else {
            dcb = sinfo->rx[iter->ch_no].desc[iter->idx].dcb_mem;
        }
    }
    if (dcb) {
        seq_printf(s, "  DCB %2d: 0x%08x 0x%08x ... 0x%08x\n", iter->idx,
                   dcb[0], dcb[1], dcb[sinfo->dcb_wsize - 1]);
    }
    return 0;
}

static struct seq_operations bkn_seq_dma_ops = {
    .start = bkn_seq_dma_start,
    .next  = bkn_seq_dma_next,
    .stop  = bkn_seq_dma_stop,
    .show  = bkn_seq_dma_show
};

static int
bkn_seq_dma_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &bkn_seq_dma_ops);
};

static struct file_operations bkn_seq_dma_file_ops = {
    .owner   = THIS_MODULE,
    .open    = bkn_seq_dma_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};


/*
 * Driver Debug Proc Entry
 */
static int 
bkn_proc_debug_read(char *page, char **start, off_t off,
                    int count, int *eof, void *data)
{
    char *ptr = page;

    /* only support whole reads :( */
    *eof = 1;

    gmodule_pprintf(&ptr, "Configuration:\n");
    gmodule_pprintf(&ptr, "  debug:          0x%x\n", debug);
    gmodule_pprintf(&ptr, "  mac_addr:       %02x:%02x:%02x:%02x:%02x:%02x\n",
                    bkn_dev_mac[0], bkn_dev_mac[1], bkn_dev_mac[2],
                    bkn_dev_mac[3], bkn_dev_mac[4], bkn_dev_mac[5]);
    gmodule_pprintf(&ptr, "  rx_buffer_size: %d (0x%x)\n",
                    rx_buffer_size, rx_buffer_size);
    gmodule_pprintf(&ptr, "  rcpu_mode:      %d\n", rcpu_mode);
    gmodule_pprintf(&ptr, "  rcpu_dmac:      %02x:%02x:%02x:%02x:%02x:%02x\n",
                    bkn_rcpu_dmac[0], bkn_rcpu_dmac[1], bkn_rcpu_dmac[2],
                    bkn_rcpu_dmac[3], bkn_rcpu_dmac[4], bkn_rcpu_dmac[5]);
    gmodule_pprintf(&ptr, "  rcpu_smac:      %02x:%02x:%02x:%02x:%02x:%02x\n",
                    bkn_rcpu_smac[0], bkn_rcpu_smac[1], bkn_rcpu_smac[2],
                    bkn_rcpu_smac[3], bkn_rcpu_smac[4], bkn_rcpu_smac[5]);
    gmodule_pprintf(&ptr, "  rcpu_ethertype: 0x%x\n", rcpu_ethertype);
    gmodule_pprintf(&ptr, "  rcpu_signature: 0x%x\n", rcpu_signature);
    gmodule_pprintf(&ptr, "  rcpu_vlan:      %d\n", rcpu_vlan);
    gmodule_pprintf(&ptr, "  use_rx_skb:     %d\n", use_rx_skb);
    gmodule_pprintf(&ptr, "Thread states:\n");
    gmodule_pprintf(&ptr, "  Command thread: %d\n", bkn_cmd_ctrl.state);
    gmodule_pprintf(&ptr, "  Event thread:   %d\n", bkn_evt_ctrl.state);

    return ptr - page; 
}

/*
 * Device Statistics Proc Entry
 */
static int 
bkn_proc_stats_read(char *page, char **start, off_t off,
                    int count, int *eof, void *data)
{
    char *ptr = page;
    int unit = 0;
    struct list_head *list;
    bkn_switch_info_t *sinfo;
    int chan;

    /* only support whole reads :( */
    *eof = 1;

    list_for_each(list, &_sinfo_list) {
        sinfo = (bkn_switch_info_t *)list;

        gmodule_pprintf(&ptr, "Device stats (unit %d):\n", unit);
        gmodule_pprintf(&ptr, "  Tx packets  %10lu\n", sinfo->tx.pkts);
        for (chan = 0; chan < NUM_RX_CHAN; chan++) {
            gmodule_pprintf(&ptr, "  Rx%d packets %10lu\n",
                            chan, sinfo->rx[chan].pkts);
        }
        gmodule_pprintf(&ptr, "  Tx suspends %10lu\n", sinfo->tx.suspends);

        unit++;
    }

    return ptr - page; 
}

static int
bkn_proc_init(void)
{
    struct proc_dir_entry *entry;

    entry = create_proc_entry("dma", 0, bkn_proc_root);
    if (entry) {
        entry->proc_fops = &bkn_seq_dma_file_ops;
    }
    create_proc_read_entry("debug", 0, bkn_proc_root, bkn_proc_debug_read, NULL);
    create_proc_read_entry("stats", 0, bkn_proc_root, bkn_proc_stats_read, NULL);

    return 0;  
}

static int
bkn_proc_cleanup(void)
{
    remove_proc_entry("stats", bkn_proc_root); 
    remove_proc_entry("dma", bkn_proc_root); 
    remove_proc_entry("debug", bkn_proc_root); 
    return 0;
}

/*
 * Generic module functions
 */

static int
_pprint(void)
{	
    pprintf("Broadcom BCM KNET Linux Network Driver\n");

    return 0;
}

static int
bkn_knet_dma_info(kcom_msg_dma_info_t *kmsg, int len)
{
    bkn_switch_info_t *sinfo;
    bkn_dcb_chain_t *dcb_chain;
    unsigned long flags;
    int chan = API_RX_CHAN;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    sinfo = bkn_sinfo_from_unit(kmsg->hdr.unit);
    if (sinfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    dcb_chain = kmalloc(sizeof(*dcb_chain), GFP_KERNEL);
    if (dcb_chain == NULL) {
        gprintk("Fatal error: No memory for dcb_chain\n");
        return 0;
    }
    memset(dcb_chain, 0, sizeof(*dcb_chain));
    dcb_chain->dcb_cnt = kmsg->dma_info.cnt;
    dcb_chain->dcb_dma = kmsg->dma_info.data.dcb_start;
    dcb_chain->dcb_mem = kernel_bde->p2l(sinfo->dev_no,
                                         dcb_chain->dcb_dma);

    if (kmsg->dma_info.type == KCOM_DMA_INFO_T_TX_DCB) {

        spin_lock_irqsave(&sinfo->lock, flags);

        /* Hold back packets from kernel */
        bkn_suspend_tx(sinfo);

        list_add_tail(&dcb_chain->list, &sinfo->tx.api_dcb_list);

        /* Acquire Tx resources */
        if (down_trylock(&sinfo->tx.sem) == 0) {
            if (sinfo->tx.free == MAX_TX_DCBS && sinfo->tx.api_active == 0) {
                bkn_api_tx(sinfo);
            }
            up(&sinfo->tx.sem);
        }

        spin_unlock_irqrestore(&sinfo->lock, flags);

    } else if (kmsg->dma_info.type == KCOM_DMA_INFO_T_RX_DCB) {

        spin_lock_irqsave(&sinfo->lock, flags);

        list_add_tail(&dcb_chain->list, &sinfo->rx[chan].api_dcb_list);

        if (sinfo->rx[chan].api_active == 0) {
            bkn_api_rx(sinfo);
        }

        spin_unlock_irqrestore(&sinfo->lock, flags);

    } else {
        DBG_DCB(("Unknown DCB_INFO type (%d).\n", kmsg->dma_info.type));
        kfree(dcb_chain);
        return 0;
    }

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_version(kcom_msg_version_t *kmsg, int len)
{
    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;
    kmsg->version = KCOM_VERSION;

    return sizeof(kcom_msg_version_t);
}

static int
bkn_knet_hw_reset(kcom_msg_hw_reset_t *kmsg, int len)
{
    bkn_switch_info_t *sinfo;
    unsigned long flags;
    int chan;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    sinfo = bkn_sinfo_from_unit(kmsg->hdr.unit);
    if (sinfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    spin_lock_irqsave(&sinfo->lock, flags);

    if (kmsg->channels == 0) {
        /* Clean all if no channels specified */
        bkn_dma_abort(sinfo);
        bkn_clean_dcbs(sinfo);
    } else {
        if (kmsg->channels & (1 << XGS_DMA_TX_CHAN)) {
            bkn_dma_abort_tx(sinfo);
            bkn_clean_tx_dcbs(sinfo);
        }
        for (chan = 0; chan < NUM_RX_CHAN; chan++) {
            if (kmsg->channels & (1 << (XGS_DMA_RX_CHAN + chan))) {
                bkn_dma_abort_rx(sinfo, chan);
                bkn_clean_rx_dcbs(sinfo, chan);
            }
        }
    }

    spin_unlock_irqrestore(&sinfo->lock, flags);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_hw_init(kcom_msg_hw_init_t *kmsg, int len)
{
    bkn_switch_info_t *sinfo;
    unsigned long flags;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    DBG_DCB(("DCB size %d, type %d\n", kmsg->dcb_size, kmsg->dcb_type));

    sinfo = bkn_sinfo_from_unit(kmsg->hdr.unit);
    if (sinfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    spin_lock_irqsave(&sinfo->lock, flags);

    /* First time called we need to allocate DCBs */
    sinfo->dcb_wsize = BYTES2WORDS(kmsg->dcb_size);
    if (sinfo->dcb_mem == NULL) {
        if (bkn_alloc_dcbs(sinfo) < 0) {
            kmsg->hdr.status = KCOM_E_RESOURCE;
            return -1;
        }
        bkn_init_dcbs(sinfo);
    }

    /* Ensure that we restart properly */
    bkn_dma_abort(sinfo);
    bkn_clean_dcbs(sinfo);

    bkn_dma_init(sinfo);

    spin_unlock_irqrestore(&sinfo->lock, flags);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_netif_create(kcom_msg_netif_create_t *kmsg, int len)
{
    bkn_switch_info_t *sinfo;
    struct net_device *dev;
    struct list_head *list;
    bkn_priv_t *priv, *lpriv;
    int found, id;
    uint8 *ma;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    switch (kmsg->netif.type) {
    case KCOM_NETIF_T_VLAN:
    case KCOM_NETIF_T_PORT:
    case KCOM_NETIF_T_META:
        break;
    default:
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    sinfo = bkn_sinfo_from_unit(kmsg->hdr.unit);
    if (sinfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    ma = kmsg->netif.macaddr;
    if ((ma[0] | ma[1] | ma[2] | ma[3] | ma[4] | ma[5]) == 0) {
        bkn_dev_mac[5]++;
        ma = bkn_dev_mac;
    }
    if ((dev = bkn_init_ndev(ma, kmsg->netif.name)) == NULL) {
        kmsg->hdr.status = KCOM_E_RESOURCE;
        return -1;
    }
    priv = netdev_priv(dev);
    priv->dev = dev;
    priv->sinfo = sinfo;
    priv->type = kmsg->netif.type;
    priv->vlan = kmsg->netif.vlan;
    if (priv->type == KCOM_NETIF_T_PORT) {
        priv->port = kmsg->netif.port;
    } else {
        priv->port = -1;
    }
    priv->flags = kmsg->netif.flags;

    /* Force RCPU encapsulation if rcpu_mode */
    if (rcpu_mode) {
        priv->flags |= KCOM_NETIF_F_RCPU_ENCAP;
        DBG_RCPU(("RCPU auto-enabled\n"));
    }

    /* Prevent (incorrect) compiler warning */
    lpriv = NULL;

    /*
     * We insert network interfaces sorted by ID.
     * In case an interface is destroyed, we reuse the ID
     * the next time an interface is created.
     */
    found = 0;
    id = 1;
    list_for_each(list, &_ndev_list) {
        lpriv = (bkn_priv_t *)list;
        if (id < lpriv->id) {
            found = 1;
            break;
        }
        id = lpriv->id + 1;
    }
    priv->id = id;
    if (found) {
        /* Replace previously removed interface */
        list_add_tail(&priv->list, &lpriv->list);
    } else {
        /* No holes - add to end of list */
        list_add_tail(&priv->list, &_ndev_list);
    }

    DBG_VERB(("Assigned ID %d to Ethernet device %s\n",
              priv->id, dev->name));

    kmsg->netif.id = priv->id;
    memcpy(kmsg->netif.macaddr, dev->dev_addr, 6);
    memcpy(kmsg->netif.name, dev->name, KCOM_NETIF_NAME_MAX - 1);

    return sizeof(*kmsg);
}

static int
bkn_knet_netif_destroy(kcom_msg_netif_destroy_t *kmsg, int len)
{
    struct net_device *dev;
    bkn_priv_t *priv;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (kmsg->hdr.id == priv->id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    list_del(&priv->list);
    dev = priv->dev;
    DBG_VERB(("Removing virtual Ethernet device %s (%d).\n",
              dev->name, priv->id));
    unregister_netdev(dev);
    free_netdev(dev);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_netif_list(kcom_msg_netif_list_t *kmsg, int len)
{
    bkn_priv_t *priv;
    struct list_head *list;
    int idx;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    idx = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        kmsg->id[idx] = priv->id;
        idx++;
    }
    kmsg->ifcnt = idx;

    return sizeof(*kmsg) - sizeof(kmsg->id) + (idx * sizeof(kmsg->id[0]));
}

static int
bkn_knet_netif_get(kcom_msg_netif_get_t *kmsg, int len)
{
    bkn_priv_t *priv;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (kmsg->hdr.id == priv->id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    memcpy(kmsg->netif.macaddr, priv->dev->dev_addr, 6);
    memcpy(kmsg->netif.name, priv->dev->name, KCOM_NETIF_NAME_MAX - 1);
    kmsg->netif.vlan = priv->vlan;
    kmsg->netif.type = priv->type;
    kmsg->netif.id = priv->id;
    kmsg->netif.flags = priv->flags;

    if (priv->port < 0) {
        kmsg->netif.port = 0;
    } else {
        kmsg->netif.port = priv->port;
    }

    return sizeof(*kmsg);
}

static int
bkn_knet_filter_create(kcom_msg_filter_create_t *kmsg, int len)
{
    struct list_head *list;
    bkn_filter_t *filter, *lfilter;
    int found, id;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    switch (kmsg->filter.type) {
    case KCOM_FILTER_T_RX_PKT:
        break;
    default:
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    /*
     * Find available ID
     */
    found = 1;
    id = 0;
    while (found && ++id < 256) {
        found = 0;
        list_for_each(list, &_filter_list) {
            lfilter = (bkn_filter_t *)list;
            if (id == lfilter->kf.id) {
                found = 1;
                break;
            }
        }
    }
    if (found) {
        /* Too many filters */
        kmsg->hdr.status = KCOM_E_RESOURCE;
        return -1;
    }

    filter = kmalloc(sizeof(*filter), GFP_KERNEL);
    if (filter == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    memset(filter, 0, sizeof(*filter));
    memcpy(&filter->kf, &kmsg->filter, sizeof(filter->kf));
    filter->kf.id = id;

    /* Add according to priority */
    found = 0;
    list_for_each(list, &_filter_list) {
        lfilter = (bkn_filter_t *)list;
        if (filter->kf.priority < lfilter->kf.priority) {
            list_add_tail(&filter->list, &lfilter->list);
            found = 1;
            break;
        }
    }
    if (!found) {
        list_add_tail(&filter->list, &_filter_list);
    }

    kmsg->filter.id = filter->kf.id;

    DBG_VERB(("Created filter ID %d (%s).\n",
              filter->kf.id, filter->kf.desc));

    return len;
}

static int
bkn_knet_filter_destroy(kcom_msg_filter_destroy_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        if (kmsg->hdr.id == filter->kf.id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    list_del(&filter->list);
    DBG_VERB(("Removing filter ID %d.\n", filter->kf.id));
    kfree(filter);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_filter_list(kcom_msg_filter_list_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int idx;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    idx = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        kmsg->id[idx] = filter->kf.id;
        idx++;
    }
    kmsg->fcnt = idx;

    return sizeof(*kmsg) - sizeof(kmsg->id) + (idx * sizeof(kmsg->id[0]));
}

static int
bkn_knet_filter_get(kcom_msg_filter_get_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        if (kmsg->hdr.id == filter->kf.id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    memcpy(&kmsg->filter, &filter->kf, sizeof(kmsg->filter));

    len = sizeof(*kmsg);

    return len;
}

static int
bkn_handle_cmd_req(kcom_msg_t *kmsg, int len)
{
    /* Silently drop events and unrecognized message types */
    if (kmsg->hdr.type != KCOM_MSG_TYPE_CMD) {
        if (kmsg->hdr.opcode == KCOM_M_STRING) {
            DBG_VERB(("Debug string: '%s'\n", kmsg->string.val));
            return 0;
        }
        DBG_VERB(("Unsupported message (type=%d, opcode=%d)\n",
                  kmsg->hdr.type, kmsg->hdr.opcode));
        return 0;
    }

    switch (kmsg->hdr.opcode) {
    case KCOM_M_DMA_INFO:
        DBG_VERB(("KCOM_M_DMA_INFO\n"));
        /* Packet buffer */
        len = bkn_knet_dma_info(&kmsg->dma_info, len);
        break;
    case KCOM_M_VERSION:
        DBG_VERB(("KCOM_M_VERSION\n"));
        /* Return procotol version */
        len = bkn_knet_version(&kmsg->version, len);
        break;
    case KCOM_M_HW_RESET:
        DBG_VERB(("KCOM_M_HW_RESET\n"));
        /* Shut down DMA and release buffers */
        len = bkn_knet_hw_reset(&kmsg->hw_reset, len);
        break;
    case KCOM_M_HW_INIT:
        DBG_VERB(("KCOM_M_HW_INIT\n"));
        /* Initialize DMA */
        len = bkn_knet_hw_init(&kmsg->hw_init, len);
        break;
    case KCOM_M_NETIF_CREATE:
        DBG_VERB(("KCOM_M_NETIF_CREATE\n"));
        /* Create network interface */
        len = bkn_knet_netif_create(&kmsg->netif_create, len);
        break;
    case KCOM_M_NETIF_DESTROY:
        DBG_VERB(("KCOM_M_NETIF_DESTROY\n"));
        /* Destroy network interface */
        len = bkn_knet_netif_destroy(&kmsg->netif_destroy, len);
        break;
    case KCOM_M_NETIF_LIST:
        DBG_VERB(("KCOM_M_NETIF_LIST\n"));
        /* Return list of IDs of installed network interfaces */
        len = bkn_knet_netif_list(&kmsg->netif_list, len);
        break;
    case KCOM_M_NETIF_GET:
        DBG_VERB(("KCOM_M_NETIF_GET\n"));
        /* Return network interface info */
        len = bkn_knet_netif_get(&kmsg->netif_get, len);
        break;
    case KCOM_M_FILTER_CREATE:
        DBG_VERB(("KCOM_M_FILTER_CREATE\n"));
        /* Create packet filter */
        len = bkn_knet_filter_create(&kmsg->filter_create, len);
        break;
    case KCOM_M_FILTER_DESTROY:
        DBG_VERB(("KCOM_M_FILTER_DESTROY\n"));
        /* Destroy packet filter */
        len = bkn_knet_filter_destroy(&kmsg->filter_destroy, len);
        break;
    case KCOM_M_FILTER_LIST:
        DBG_VERB(("KCOM_M_FILTER_LIST\n"));
        /* Return list of IDs of installed packet filters */
        len = bkn_knet_filter_list(&kmsg->filter_list, len);
        break;
    case KCOM_M_FILTER_GET:
        DBG_VERB(("KCOM_M_FILTER_GET\n"));
        /* Return packet filter info */
        len = bkn_knet_filter_get(&kmsg->filter_get, len);
        break;
    default:
        DBG_VERB(("Unsupported command (type=%d, opcode=%d)\n",
                  kmsg->hdr.type, kmsg->hdr.opcode));
        kmsg->hdr.opcode = 0;
        len = sizeof(kcom_msg_hdr_t);
        break;
    }
    return len;
}

static int
bkn_cmd_thread(void *context)
{
    bkn_thread_ctrl_t *tc = (bkn_thread_ctrl_t *)context;
    kcom_msg_t kmsg;
    int len, rlen;

    bkn_thread_boot(tc);

    DBG_VERB(("Command thread starting\n"));
    tc->state = 1;
    while (!bkn_thread_should_stop(tc)) {
        len = sizeof(kmsg);
        tc->state = 2;
        if (PROXY_RECV(KCOM_CHAN_KNET, &kmsg, &len) >= 0) {
            DBG_VERB(("Received %d bytes from KCOM_CHAN_CMD\n", len));
            tc->state = 3;
            rlen = bkn_handle_cmd_req(&kmsg, len);
            tc->state = 4;
            if (rlen > 0) {
                PROXY_SEND(KCOM_CHAN_KNET, &kmsg, rlen);
            }
        } else {
            /* Thread interrupted */
            bkn_sleep(1);
        }
    }
    DBG_VERB(("Command thread done\n"));

    bkn_thread_exit(tc);
    return 0;
}

static int
bkn_get_next_dma_event(kcom_msg_dma_info_t *kmsg)
{
    static int last_dev_no = 0;
    bkn_switch_info_t *sinfo;
    unsigned long flags;
    int dev_no;

    dev_no = last_dev_no;

    while (1) {
        dev_no++;
        sinfo = bkn_sinfo_from_unit(dev_no);
        if (sinfo == NULL) {
            dev_no = 0;
            sinfo = bkn_sinfo_from_unit(dev_no);
        }

        if (sinfo && sinfo->dma_events) {
            DBG_VERB(("Next DMA events (0x%08x)\n", sinfo->dma_events));
            kmsg->hdr.unit = sinfo->dev_no;

            spin_lock_irqsave(&sinfo->lock, flags);
            kmsg->dma_info.flags = sinfo->dma_events;
            sinfo->dma_events = 0;
            spin_unlock_irqrestore(&sinfo->lock, flags);

            last_dev_no = dev_no;
            break;
        }

        if (dev_no == last_dev_no) {
            wait_event_interruptible(evt_wq, evt_wq_get != evt_wq_put);
            DBG_VERB(("Event thread wakeup\n"));

            /* Thread interrupted */
            if (signal_pending(current)) {
               return 0;
            }

            evt_wq_get = evt_wq_put;
        }
    }
    return sizeof(*kmsg);
}

static int
bkn_evt_thread(void *context)
{
    bkn_thread_ctrl_t *tc = (bkn_thread_ctrl_t *)context;
    kcom_msg_dma_info_t kmsg;
    int len;

    bkn_thread_boot(tc);

    memset(&kmsg, 0, sizeof(kmsg));
    kmsg.hdr.type = KCOM_MSG_TYPE_EVT;
    kmsg.hdr.opcode = KCOM_M_DMA_INFO;

    DBG_VERB(("Event thread starting\n"));
    tc->state = 1;
    while (!bkn_thread_should_stop(tc)) {
        tc->state = 2;
        len = bkn_get_next_dma_event(&kmsg);
        tc->state = 3;
        if (len) {
            PROXY_SEND(KCOM_CHAN_KNET, &kmsg, len);
        } else {
            /* Thread interrupted */
            bkn_sleep(1);
        }
    }
    DBG_VERB(("Event thread done\n"));

    bkn_thread_exit(tc);
    return 0;
}

static int
_cleanup(void)
{
    struct list_head *list;
    struct net_device *dev;
    bkn_filter_t *filter;
    bkn_priv_t *priv;
    bkn_switch_info_t *sinfo;
    unsigned long flags;

    /* Shut down event thread */
    bkn_thread_stop(&bkn_evt_ctrl);

    /* Shut down command thread */
    bkn_thread_stop(&bkn_cmd_ctrl);

    /* Remove KCOM channel */
    PROXY_SERVICE_DESTROY(KCOM_CHAN_KNET); 

    bkn_proc_cleanup();
    remove_proc_entry("bcm/knet", NULL);
    remove_proc_entry("bcm", NULL);

    list_for_each(list, &_sinfo_list) {
        sinfo = (bkn_switch_info_t *)list;

        spin_lock_irqsave(&sinfo->lock, flags);

        xgs_irq_mask_disable(sinfo, TXRX_IRQ_MASK, 1);

        DBG_IRQ(("Unregister ISR.\n"));
        kernel_bde->interrupt_disconnect(sinfo->dev_no | LKBDE_ISR2_DEV);

        /* Abort DMA and release DCBs */
        bkn_dma_abort(sinfo);
        bkn_clean_dcbs(sinfo);

        spin_unlock_irqrestore(&sinfo->lock, flags);
    }

    /* Destroy all filters */
    while (!list_empty(&_filter_list)) {
	filter = list_entry(_filter_list.next, bkn_filter_t, list);	
	list_del(&filter->list);
        DBG_VERB(("Removing filter ID %d.\n", filter->kf.id));
        kfree(filter);
    }

    /* Destroy all virtual net devices */
    while (!list_empty(&_ndev_list)) {
	priv = list_entry(_ndev_list.next, bkn_priv_t, list);	
	list_del(&priv->list);
        dev = priv->dev;
        DBG_VERB(("Removing virtual Ethernet device %s.\n", dev->name));
        unregister_netdev(dev);
        free_netdev(dev);
    }

    /* Destroy all switch devices */
    while (!list_empty(&_sinfo_list)) {
	sinfo = list_entry(_sinfo_list.next, bkn_switch_info_t, list);	
        if (sinfo->dev) {
            DBG_VERB(("Removing Ethernet device %s.\n", sinfo->dev->name));
#if NAPI_SUPPORT
            netif_poll_disable(sinfo->dev);
#endif
            unregister_netdev(sinfo->dev);
            free_netdev(sinfo->dev);
        }
        DBG_VERB(("Removing switch device.\n"));
        bkn_destroy_sinfo(sinfo);
    }

    return 0;
}	

static int
_init(void)
{
    int idx;
    int num_dev;
    uint32_t dev_type;
    struct net_device *dev;
    bkn_switch_info_t *sinfo;
    bkn_priv_t *priv;
    char *bdev_name;

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &kernel_bde) < 0) || kernel_bde == NULL) {
        return -ENODEV;
    }

    /* Randomize Lower 3 bytes of the MAC address (TESTING ONLY) */
    get_random_bytes(&bkn_dev_mac[3], 3);

    /* Check for user-supplied MAC address (recommended) */
    if (mac_addr != NULL && strlen(mac_addr) == 17) {
        for (idx = 0; idx < 6; idx++) {
            bkn_dev_mac[idx] = simple_strtoul(&mac_addr[idx*3], NULL, 16);
        }
        /* Do not allow multicast address */
        bkn_dev_mac[0] &= ~0x01;
    }

    /* Base network device name */
    bdev_name = "bcm%d";
    if (base_dev_name) {
        if (strlen(base_dev_name) < IFNAMSIZ) {
            bdev_name = base_dev_name;
        } else {
            DBG_VERB(("Base device name too long\n"));
        }
    }

    /* Optional RCPU MAC addresses */
    if (rcpu_dmac != NULL && strlen(rcpu_dmac) == 17) {
        for (idx = 0; idx < 6; idx++) {
            bkn_rcpu_dmac[idx] = simple_strtoul(&rcpu_dmac[idx*3], NULL, 16);
        }
    }
    if (rcpu_smac != NULL && strlen(rcpu_smac) == 17) {
        for (idx = 0; idx < 6; idx++) {
            bkn_rcpu_smac[idx] = simple_strtoul(&rcpu_smac[idx*3], NULL, 16);
        }
    }

    num_dev = kernel_bde->num_devices(BDE_ALL_DEVICES);
    for (idx = 0; idx < num_dev; idx++) {
        dev_type = kernel_bde->get_dev_type(idx);
        DBG_VERB(("Found device type 0x%x\n", dev_type));
        if (dev_type & BDE_ETHER_DEV_TYPE) {
            DBG_VERB(("Not switch device - skipping\n"));
            continue;
        }
        if ((dev_type & BDE_PCI_DEV_TYPE) == 0) {
            DBG_VERB(("Not PCI device - skipping\n"));
            continue;
        }
        if ((sinfo = bkn_create_sinfo(idx)) == NULL) {
            _cleanup();
            return -ENOMEM;
        }

        /* Ensure 32-bit PCI DMA is mapped properly on 64-bit platforms */
        if (pci_set_dma_mask(sinfo->pdev, 0xffffffff)) {
            gprintk("No suitable DMA available for SKBs\n");
            _cleanup();
            return -EIO;
	}

        /* Register interrupt handler */
        kernel_bde->interrupt_connect(sinfo->dev_no | LKBDE_ISR2_DEV,
                                      bkn_isr, sinfo);

        /* Create base virtual net device */
        bkn_dev_mac[5]++;
        if ((dev = bkn_init_ndev(bkn_dev_mac, bdev_name)) == NULL) {
            _cleanup();
            return -ENOMEM;
        } else {
            sinfo->dev = dev;
            priv = netdev_priv(dev);
            priv->dev = dev;
            priv->sinfo = sinfo;
            priv->vlan = 1;
            priv->port = -1;
            priv->id = -1;
        }

#if NAPI_SUPPORT
	netif_poll_enable(dev);
#endif
    }

    /* Initialize proc files */
    proc_mkdir("bcm", NULL);
    bkn_proc_root = proc_mkdir("bcm/knet", NULL);

    bkn_proc_init();

    init_waitqueue_head(&evt_wq);

    if (use_proxy) {
        PROXY_SERVICE_CREATE(KCOM_CHAN_KNET, 1, 0); 

        DBG_VERB(("Starting command thread\n"));
        bkn_thread_start(&bkn_cmd_ctrl, "bkncmd", bkn_cmd_thread);

        DBG_VERB(("Starting event thread\n"));
        bkn_thread_start(&bkn_evt_ctrl, "bknevt", bkn_evt_thread);
    }

    return 0;
}

static int 
_ioctl(unsigned int cmd, unsigned long arg)
{
    bkn_ioctl_t io; 
    kcom_msg_t kmsg;

    if (copy_from_user(&io, (void*)arg, sizeof(io))) {
        return -EFAULT;
    }

    if (io.len > sizeof(kmsg)) {
        return -EINVAL;
    }

    io.rc = 0;
  
    switch(cmd)	{
    case 0:
        if (io.len > 0) {
            if (copy_from_user(&kmsg, io.buf, io.len)) {
                return -EFAULT;
            }
            io.len = bkn_handle_cmd_req(&kmsg, io.len);
        } else {
            memset(&kmsg, 0, sizeof(kcom_msg_dma_info_t));
            kmsg.hdr.type = KCOM_MSG_TYPE_EVT;
            kmsg.hdr.opcode = KCOM_M_DMA_INFO;
            io.len = bkn_get_next_dma_event((kcom_msg_dma_info_t *)&kmsg);
        }
        if (io.len > 0) {
            if (copy_to_user(io.buf, &kmsg, io.len)) {
                return -EFAULT;
            }
        }
        break;
    default:
        GDBG("Invalid IOCTL");
        io.rc = -1;
        break;
    }

    if (copy_to_user((void*)arg, &io, sizeof(io))) {
        return -EFAULT;
    }

    return 0;
}

static gmodule_t _gmodule = {
    name: MODULE_NAME, 
    major: MODULE_MAJOR, 
    init: _init,
    cleanup: _cleanup, 
    pprint: _pprint, 
    ioctl: _ioctl,
    open: NULL, 
    close: NULL, 
}; 

gmodule_t*
gmodule_get(void)
{
    EXPORT_NO_SYMBOLS;
    return &_gmodule;
}
