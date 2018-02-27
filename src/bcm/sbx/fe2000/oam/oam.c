/*
 * $Id: oam.c 1.41 Broadcom SDK $
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
 *
 * FE2000 OAM API
 */

#if defined(INCLUDE_L3)

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/oam/enet.h>
#include <bcm_int/sbx/fe2000/oam/bfd.h>
#include <bcm_int/sbx/fe2000/oam/pm.h>
#include <bcm_int/sbx/fe2000/oam/mpls.h>
#include <bcm_int/sbx/fe2000/oam/recovery.h>
#include <bcm_int/sbx/fe2000/oam/psc.h>

#define OAM_INTR_LOCK            oam_spl = sal_splhi()
#define OAM_INTR_UNLOCK          sal_spl(oam_spl)

typedef struct oam_control_s {
    /* Thread control */
    sal_sem_t         cb_notify;        /* Semaphore for callback thread */
    volatile int      cb_notify_given;  /* Semaphore already given */
    sal_thread_t      cb_tid;           /* callback thread id */
    volatile int      thread_running;   /* Input signal to thread */

} oam_control_t;


/* Single OAM control structure, common for all units */
static oam_control_t oam_control;
static int oam_spl;
oam_state_t* _state[SOC_MAX_NUM_DEVICES];

/****************************************************************************/

STATIC int _oam_cleanup(int unit);
#if defined(BCM_QE2000_SUPPORT) && defined(BCM_FE2000_SUPPORT)
STATIC int _oam_exceptions_set(int unit, int enable);
#endif /* BCM_QE2000_SUPPORT && BCM_FE2000_SUPPORT */

STATIC int _oam_remove_ep_from_trunk_list(int unit, bcm_trunk_t tid, dq_p_t ep_elem);
STATIC int _oam_delete_trunk_info(int unit, bcm_trunk_t tid);
STATIC int _oam_trunk_cb(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *trunk_info, void *user_data);
STATIC int _oam_endpoint_idx_find(int unit, bcm_oam_endpoint_info_t *ep_info, 
                                  uint32_t *idx);

STATIC int _oam_get_endpoint_by_timer(int unit, uint32_t timer, 
                                      uint32_t *ep_rec_idx);

STATIC void _init_ep_hash_key(int unit, oam_sw_hash_key_t key,
                              bcm_oam_endpoint_info_t *ep_info); 

STATIC int _oam_thread_start(int unit);
STATIC int _oam_thread_stop(int unit);
STATIC void oam_cb_handler_thread(void *param);
STATIC int _oam_process_expired_timers(int unit);
STATIC void _oam_fe2000_wd_timeout_cb(void *data);
STATIC int _oam_port_dest_queue_get(int unit, uint32 port, uint32 *pq);

STATIC INLINE void _oam_tq_lock(oam_timer_queue_t *q)
{ q->lock = sal_splhi(); }

STATIC INLINE void _oam_tq_unlock(oam_timer_queue_t *q)
{ sal_spl(q->lock); }


STATIC INLINE int _oam_tq_queue_count(oam_timer_queue_t *q)
{ return q->tail + 1; }

STATIC INLINE int _oam_tq_queue_empty(oam_timer_queue_t *q)
{ return q->tail < 0; }

STATIC INLINE int _oam_tq_queue_full(oam_timer_queue_t *q)
{ return _oam_tq_queue_count(q) >= q->max_depth; }

STATIC INLINE int _oam_tq_queue_init(oam_timer_queue_t *q, int max_depth)
{
    q->queue[0] =
        sal_alloc(sizeof(oam_timer_q_elt_t) * max_depth, "oam_tmr_queue0");
    q->queue[1] =
        sal_alloc(sizeof(oam_timer_q_elt_t) * max_depth, "oam_tmr_queue1");

    if (q->queue[0] == NULL || q->queue[1] == NULL) {
        return BCM_E_MEMORY;
    }

    q->intr_qid  = 0;
    q->tail      = -1;
    q->max_depth = max_depth;
    return BCM_E_NONE;
}

STATIC INLINE int _oam_tq_queue_destroy(oam_timer_queue_t *q)
{
    int i;
    for (i=0; i<2; i++) {
        if (q->queue[i]) {
            sal_free(q->queue[i]);
            q->queue[i] = NULL;
        }
    }

    return BCM_E_NONE;
}


/* Get the timer ids from the interrupt queue.  Locking. */
STATIC void _oam_tq_get_all(oam_timer_queue_t *q, int *count)
{
    _oam_tq_lock(q);
    *count           = _oam_tq_queue_count(q);
    q->tail          = -1;
    q->intr_qid      = !q->intr_qid; /* flip to the other queue */
    _oam_tq_unlock(q);
}

/* get the timer queue eleement at index 0 from the non-interrupt queue,
 * Must be called after a call to _oam_tq_get_all.  No locking required.
 */
STATIC INLINE void _oam_tq_queue_get_elt(oam_timer_queue_t *q,
                                         int idx,
                                         oam_timer_q_elt_t *elt)
{
    elt->timer = q->queue[ !q->intr_qid ][idx].timer;
    elt->rdi   = q->queue[ !q->intr_qid ][idx].rdi;
}

/* Enqueue at tail; no locking here (should be in intr context) */
STATIC INLINE void _oam_tq_enqueue(oam_timer_queue_t *q,
                                   uint32_t timer,
                                   uint32_t rdi)
{
    assert( !_oam_tq_queue_full(q) );
    q->tail++;
    q->queue[q->intr_qid][q->tail].timer = timer;
    q->queue[q->intr_qid][q->tail].rdi   = rdi;
}

int
oam_group_ep_list_count(int unit, int group)
{
    int count=0;
    DQ_LENGTH(&OAM_GROUP_EP_LIST(unit, group), count);
    return count;
}

int
oam_group_ep_list_add(int unit, int group, dq_p_t ep) 
{

    int rv = BCM_E_NONE;

    if (OAM_GROUP_INFO(unit, group) && ep) {
        DQ_INSERT_HEAD(&OAM_GROUP_EP_LIST(unit,group), ep); 
    } else {
        OAM_ERR((_SBX_D(unit, "Invalid Group\n")));
        rv = BCM_E_PARAM;
    }
    return rv;
}

int
oam_group_ep_list_remove(int unit, int group, dq_p_t ep)
{
    int rv = BCM_E_NONE;

    if(OAM_GROUP_INFO(unit, group) && ep) {
        DQ_REMOVE(ep);
    } else {
        OAM_ERR((_SBX_D(unit, "Invalid Group\n")));
        rv = BCM_E_PARAM;
    }
    return rv;
}

STATIC int
_oam_endpoint_type_supported(bcm_oam_endpoint_info_t *ep_info)
{
    switch (ep_info->type)
    {
    case bcmOAMEndpointTypeEthernet:
    case bcmOAMEndpointTypeBFDVccv:
    case bcmOAMEndpointTypeBFDMPLS:
    case bcmOAMEndpointTypeMPLSPerformance:  
    case bcmOAMEndpointTypePSC:
        return BCM_E_NONE;
    default:
        return BCM_E_FAIL;
    }
    return BCM_E_FAIL;
}

void
_oam_oamep_prepare(int unit, soc_sbx_g2p3_oamep_t *oamep)
{
    soc_sbx_g2p3_oamep_t tmp = *oamep;
    soc_sbx_g2p3_oamep_t_init(oamep);

    /* this function takes the passed-in oamep structure, and it
     * clears out all the fields.  then it will copy back only
     * those fields that are relavent to the endpoint type.  this
     * reason for this is that the soc layer does not handle over-
     * lapping fields in these UTG structures.  the "iff" clause
     * works for a single type, and many of these fields are common
     * to more than one endpoint type, but not common to all endpoint
     * types.  future revisions of UTG will hopefully resolve these
     * issues and this function hopefully can be removed.
     */

    if (tmp.function == _state[unit]->ep_type_ids.ccm_first) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->interval = tmp.interval;
        oamep->mdlevel = tmp.mdlevel;
        oamep->rdi = tmp.rdi;
        oamep->mip = tmp.mip;
        oamep->lm = tmp.lm;
        oamep->notlm = tmp.notlm;
        oamep->dir = tmp.dir;
        oamep->mepid = tmp.mepid;
        oamep->intpri = tmp.intpri;
        oamep->maidw0 = tmp.maidw0;
        oamep->maidw1 = tmp.maidw1;
        oamep->maidw2 = tmp.maidw2;
        oamep->maidw3 = tmp.maidw3;
        oamep->maidw4 = tmp.maidw4;
        oamep->maidw5 = tmp.maidw5;

    } else if (tmp.function == _state[unit]->ep_type_ids.ccm_second) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->smacaux = tmp.smacaux;
        oamep->smacadd = tmp.smacadd;
        oamep->smacoffset = tmp.smacoffset;
        oamep->sid = tmp.sid;
        oamep->ftidx = tmp.ftidx;
        oamep->maidw6 = tmp.maidw6;
        oamep->maidw7 = tmp.maidw7;
        oamep->maidw8 = tmp.maidw8;
        oamep->maidw9 = tmp.maidw9;
        oamep->maidw10 = tmp.maidw10;
        oamep->maidw11 = tmp.maidw11;

    } else if (tmp.function == _state[unit]->ep_type_ids.ccm_peer) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->maidcrc = tmp.maidcrc;
        oamep->policerid = tmp.policerid;
        oamep->peerrdi = tmp.peerrdi;
        oamep->ackrdi = tmp.ackrdi;
        oamep->ackrdi_rdi = tmp.ackrdi_rdi;

    } else if (tmp.function == _state[unit]->ep_type_ids.psc) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->ftidx = tmp.ftidx;
        oamep->tx = tmp.tx;
        oamep->path = tmp.path;
        oamep->fpath = tmp.fpath;
        oamep->reserved = tmp.reserved;
        oamep->revertive = tmp.revertive;
        oamep->pt = tmp.pt;
        oamep->request = tmp.request;
        oamep->version = tmp.version;
        oamep->pschdr = tmp.pschdr;
        oamep->slowrate = tmp.slowrate;
        oamep->filter = tmp.filter;
        oamep->burst = tmp.burst;
        oamep->conttohost = tmp.conttohost;    
        oamep->singletohost = tmp.singletohost;
        oamep->maxslowrate = tmp.maxslowrate;
        oamep->maxfilter = tmp.maxfilter;
        oamep->tlvlength = tmp.tlvlength;
        oamep->tlv0_3  = tmp.tlv0_3;
    } else if (tmp.function == _state[unit]->ep_type_ids.bfd) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->interval = tmp.interval;
        oamep->c = tmp.c;
        oamep->a = tmp.a;
        oamep->d = tmp.d;
        oamep->m = tmp.m;
        oamep->unibi = tmp.unibi;
        oamep->cv = tmp.cv;
        oamep->mode = tmp.mode;
        oamep->fbit = tmp.fbit;
        oamep->conttohost = tmp.conttohost;    
        oamep->singletohost = tmp.singletohost;
        oamep->ftidx = tmp.ftidx;
        oamep->ipda = tmp.ipda;
        oamep->yourdiscrim = tmp.yourdiscrim;
        oamep->mydiscrim = tmp.mydiscrim;
        oamep->udpsourceport = tmp.udpsourceport;
        oamep->peerrdi = tmp.peerrdi;
        oamep->ackrdi = tmp.ackrdi;
        oamep->ackrdi_rdi = tmp.ackrdi_rdi;
        oamep->multipoint = tmp.multipoint;
        oamep->peerstate  = tmp.peerstate;
        oamep->final = tmp.final;  /* unused */
        oamep->poll  = tmp.poll;
        oamep->localstate = tmp.localstate;
        oamep->diag = tmp.diag;
        oamep->policerid = tmp.policerid;
        oamep->peerdetectmulti = tmp.peerdetectmulti;
        oamep->localdetectmulti = tmp.localdetectmulti;

    } else if (tmp.function == _state[unit]->ep_type_ids.dm) {
        oamep->nextentry = tmp.nextentry;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->mdlevel = tmp.mdlevel;
        oamep->singletx = tmp.singletx;
        oamep->mode = tmp.mode;
        oamep->dir = tmp.dir;
        oamep->conttohost = tmp.conttohost;    
        oamep->singletohost = tmp.singletohost;
        oamep->ftidx = tmp.ftidx;
        oamep->delaysec = tmp.delaysec;
        oamep->remotesec = tmp.remotesec;
        oamep->delaynanosec = tmp.delaynanosec;
        oamep->remotenanosec = tmp.remotenanosec;

    } else if (tmp.function == _state[unit]->ep_type_ids.lm) {
        oamep->counteridx = tmp.counteridx;
        oamep->function = tmp.function;
        oamep->type = tmp.type;
        oamep->multiplieridx = tmp.multiplieridx;
        oamep->mdlevel = tmp.mdlevel;
        oamep->singletx = tmp.singletx;
        oamep->mode = tmp.mode;
        oamep->dir = tmp.dir;
        oamep->conttohost = tmp.conttohost;    
        oamep->singletohost = tmp.singletohost;
        oamep->rxfc = tmp.rxfc;
        oamep->txfcf = tmp.txfcf;
        oamep->txfcb = tmp.txfcb;
        oamep->rxfcl = tmp.rxfcl;
        oamep->nextentry_store = tmp.nextentry_store;
        oamep->ftidx_store = tmp.ftidx_store;
        oamep->framelossfar = tmp.framelossfar;
        oamep->framelossnear = tmp.framelossnear;

    } else if (tmp.function == _state[unit]->ep_type_ids.invalid) {
        oamep->type  = tmp.type;
        oamep->ftidx = tmp.ftidx;
    }
}


void _oam_hash_data_clear(oam_sw_hash_data_t *hash_data)
{
    sal_memset(hash_data, 0, sizeof(oam_sw_hash_data_t));
}

STATIC int
_oam_remove_ep_from_trunk_list(int unit, bcm_trunk_t tid, dq_p_t ep_list_node)
{
    int rv = BCM_E_NOT_FOUND;
    _oam_trunk_data_t *trunk_data=NULL;

    if(!ep_list_node) {
        return BCM_E_PARAM;
    }

    OAM_VERB((_SBX_D(unit, "Removing endpoint-id from trunk-id %d\n"),
                      tid));

    /* use the hash table to find the trunk entry. */
    trunk_data = &(_state[unit]->trunk_data[tid]);

    if (trunk_data==NULL) {
        OAM_ERR((_SBX_D(unit, "Failed to find trunk %d entry: %d %s\n"),
                 tid, rv, bcm_errmsg(rv)));
        return rv;
    }

    DQ_REMOVE(ep_list_node);
    return rv;
}

STATIC int
_oam_delete_trunk_info(int unit, bcm_trunk_t tid)
{
    int rv = BCM_E_NONE;
    _oam_trunk_data_t *trunk_data;
    
    if (OAM_TID_VALID(tid) == FALSE) {
        return BCM_E_INTERNAL;
    }

    trunk_data = &(_state[unit]->trunk_data[tid]);

    /* check that there arent any endpoints remaining.
     * currently, if there are still endpoints, just return without deleting.
     * should an error be returned? */
    if (DQ_EMPTY(&trunk_data->endpoint_list)) {
        sal_memset (trunk_data, 0, sizeof(_oam_trunk_data_t));
        DQ_INIT(&trunk_data->endpoint_list);
    }

    return rv;
}


int
bcm_fe2000_oam_detach(int unit)
{
    int rv = BCM_E_NONE;
    rv = _oam_cleanup(unit);
    return rv;
}

int
bcm_fe2000_oam_init(int unit)
{
    int rv = BCM_E_NONE;
    int base, size, dmac_type;
    int queue_depth = 0;
    soc_sbx_g2p3_oam_rx_t soc_sbx_g2p3_oam_rx;
    soc_sbx_g2p3_policer_params_t timerParams;
    soc_sbx_g2p3_policer_bank_params_t policerBankParams;
    soc_sbx_g2p3_p2fti_t p2fti;
    bcm_port_t port;
    pbmp_t pbmp;
    int port_mdlvl, mdlvl, index;
    soc_sbx_g2p3_oamportmdlevel2etc_t pmd;

#ifdef BCM_WARM_BOOT_SUPPORT 
    soc_wb_cache_t            *wbc;
    int                        wb_size; 
    fe2k_oam_wb_mem_layout_t  *layout = NULL;
#endif /* BCM_WARM_BOOT_SUPPORT */


    if (BCM_OAM_IS_INIT(unit)) {
#ifdef BCM_WARM_BOOT_SUPPORT 
        if (SOC_WARM_BOOT(unit)) {
            return _oam_wb_post_init_recover(unit);
        }
#endif /* BCM_WARM_BOOT_SUPPORT */

        rv = bcm_fe2000_oam_detach(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    soc_sbx_g2p3_oam_rx_t_init(&soc_sbx_g2p3_oam_rx);

    rv =  soc_sbx_g2p3_oamtimer_policer_params_get(unit, &timerParams);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get oam timer params: %d %s\n"),
                 rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_policer_bank_params_get(unit, timerParams.bank, 
                                              &policerBankParams);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get oam timer policer bank params: "
                        "%d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }

    _state[unit] = sal_alloc(sizeof(oam_state_t), "oam_state");
    if (_state[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_state[unit], 0, sizeof(oam_state_t));

    _state[unit]->timer_base = policerBankParams.base;

    rv = soc_sbx_g2p3_oam_num_endpoints_get(unit, 
                                            &_state[unit]->max_endpoints);
    if (BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    /* provides record index -> hash data mapping */
    size = sizeof(oam_sw_hash_data_t) * _state[unit]->max_endpoints;
    _state[unit]->hash_data_store = sal_alloc(size, "oam_hash_data");

    if (_state[unit]->hash_data_store == NULL) {
        _oam_cleanup(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_state[unit]->hash_data_store, 0, size);

    /* provides group index -> group_info_t* mapping */
    size = sizeof(oam_group_desc_t) * _state[unit]->max_endpoints;
    _state[unit]->group_info = sal_alloc(size, "oam_group_info");

    if (_state[unit]->group_info == NULL) {
        _oam_cleanup(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_state[unit]->group_info, 0, size);

    /* provides trunk index -> endpoint mapping */
    size = sizeof(_oam_trunk_data_t) * SBX_MAX_TRUNKS;
    _state[unit]->trunk_data = sal_alloc(size, "oam_trunk_data");

    if (_state[unit]->trunk_data == NULL) {
        _oam_cleanup(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_state[unit]->trunk_data, 0, size);
   
    for(index=0; index < SBX_MAX_TRUNKS; index++) {
        DQ_INIT(&_state[unit]->trunk_data[index].endpoint_list);
    }
    
    /* the first ccm entry is reserved by the g2 ucode */
    rv = shr_idxres_list_create(&_state[unit]->ep_pool,
                                1, _state[unit]->max_endpoints - 1,
                                0, _state[unit]->max_endpoints - 1,
                                "oam ep pool");
    if (BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    /* SBX stores groups in the endpoints, therefore it's possible to 
     * have num_endpoints groups
     */
    rv = shr_idxres_list_create(&_state[unit]->group_pool,
                                1, _state[unit]->max_endpoints - 1,
                                0, _state[unit]->max_endpoints - 1,
                                "oam group pool");
    if (BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }


    /* Find the base ids for the timer and sequence number generator groups
     * defined by sbG2FeInit.  Track the base ids, and compute the range
     * for use in the idxres_list manager per group.
     */
    base = timerParams.base;
    size = timerParams.size;
    rv = shr_idxres_list_create(&_state[unit]->timer_pool,
                                base + 1, base + size - 1,
                                base, base + size - 1,
                                "oam timer pool");
    if (BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    /* g2p3 ucode requires 2 consecutive counter ids for loss measurement.
     * The size is cut in half where 
     *   RxCounterIdx = id * 2
     *   TxCounterIdx = (id * 2) + 1
     * Reserve the first pair of counters (0 & 1) as invalid counters.
     */
    size = OAM_NUM_COHERENT_COUNTERS >> 1;
    rv = shr_idxres_list_create(&_state[unit]->coco_pool,
                                1, size - 1,
                                0, size - 1,
                                "oam coherent counter pool");
    if (BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    rv = shr_htb_create(&_state[unit]->mamep_htbl,
                        _state[unit]->max_endpoints,
                        sizeof(oam_sw_hash_key_t),
                        "mamep_hash");

    if( BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    rv = shr_htb_create(&_state[unit]->timer_htbl,
                        _state[unit]->max_endpoints,
                        sizeof(uint32_t),
                        "timer_hash");

    if( BCM_FAILURE(rv)) {
        _oam_cleanup(unit);
        return rv;
    }

    soc_sbx_g2p3_max_pids_get(unit, &_state[unit]->max_pids);

    /* get endpoint function types */
    soc_sbx_g2p3_oam_function_bfd_get(unit, &_state[unit]->ep_type_ids.bfd);
    soc_sbx_g2p3_oam_function_lm_get(unit, &_state[unit]->ep_type_ids.lm);
    soc_sbx_g2p3_oam_function_dm_get(unit, &_state[unit]->ep_type_ids.dm);
    soc_sbx_g2p3_oam_function_ccm_first_get(unit, &_state[unit]->ep_type_ids.ccm_first);
    soc_sbx_g2p3_oam_function_ccm_second_get(unit, &_state[unit]->ep_type_ids.ccm_second);
    soc_sbx_g2p3_oam_function_ccm_peer_get(unit, &_state[unit]->ep_type_ids.ccm_peer);
    soc_sbx_g2p3_oam_function_psc_get(unit, &_state[unit]->ep_type_ids.psc);
    soc_sbx_g2p3_oam_type_invalid_get(unit, &_state[unit]->ep_type_ids.invalid);

    soc_sbx_g2p3_oam_ep_subtype_ccm_get(unit, &_state[unit]->ep_subtypes.cfm);
    soc_sbx_g2p3_oam_ep_subtype_lm_get(unit, &_state[unit]->ep_subtypes.lm);
    soc_sbx_g2p3_oam_ep_subtype_dm_get(unit, &_state[unit]->ep_subtypes.dm);

    /* OAM transport layer types */
    soc_sbx_g2p3_oam_type_enet_get(unit, &_state[unit]->ep_transport.enet);
    soc_sbx_g2p3_oam_type_mpls_pwe_get(unit, &_state[unit]->ep_transport.pwe);
    soc_sbx_g2p3_oam_type_mpls_lsp_get(unit, &_state[unit]->ep_transport.lsp);

    /* set the BFD diag code for RDI */
    soc_sbx_g2p3_oam_bfd_rdi_diag_code_set(unit, 1);

    /* enable OAM rule in PPE */
    if (SOC_SBX_OAM_RX_ENABLE(unit)) {
        OAM_DEBUG((_SBX_D(unit, "Enabling OAM Rx PPE Rule\n")));
        soc_sbx_g2p3_oam_rx.valid = 1;
        soc_sbx_g2p3_oam_rx_set(unit, OAM_RX_RULE, &soc_sbx_g2p3_oam_rx);

        /* on Fe2k-XT systems, the source-buffer field has been shifted
           to the right one bit in the PED Descriptor.  */
        if (SOC_IS_SBX_FE2KXT(unit)) {
            soc_sbx_g2p3_oam_source_buffer_set(unit, ((SB_FE2000XT_QM_OAM_SRC_BUFFER) >> 1 ));
            soc_sbx_g2p3_oam_source_buffer_set(unit, ((SB_FE2000XT_QM_OAM_SRC_BUFFER) >> 1 ));
        }
    }

    rv = _bcm_tcal_init(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to init timer calendar manager\n")));
        _oam_cleanup(unit);
        return rv;
    }

    rv = _bcm_ltm_init(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to init loss threshold manager\n")));
        _oam_cleanup(unit);
        return rv;
    }

    /* enable OAM bubble timer */
    if (SOC_SBX_OAM_TX_ENABLE(unit)) {
        uint32 list_mgr_entries;
        uint32 bubble_timeout;
        uint32 dest_queue;
        uint32 port;

        OAM_DEBUG((_SBX_D(unit, "Enabling OAM Bubble Timer\n")));

        /* this value is the bubble timer's operational timeout in microseconds. */
        soc_sbx_g2p3_oam_bubble_timeout_get (unit, &bubble_timeout);
        
        /* max number of entries in the timer calendar.  this is the size of
           the ring buffer in the list manager. */
        if (SOC_IS_SBX_FE2KXT(unit)) {
            soc_sbx_g2p3_oam_list_mgr_size_get (unit, &list_mgr_entries);
        } else {
            soc_sbx_g2p3_oam_list_mgr_size_ca_get (unit, &list_mgr_entries);
        }

        /* this pushdown value starts off with the default value of 0x7fff,
           which disables the bubble timer.  we over-write that value with
           the operational value (currently 10 usec). */
        soc_sbx_g2p3_oam_bubble_timer_offset_set (unit, bubble_timeout);

        /* Configure the PCI's ingress destination queue as the bubble's
         * destination queue
         */
        port = CMIC_PORT(unit);
        _oam_port_dest_queue_get(unit, port, &dest_queue);
        soc_sbx_g2p3_oam_dest_queue_set(unit, dest_queue);

        /* the second argument (startBubble) of 1 kicks off the timer.  once started
           the timer uses the timerout value (above). */
        if (SOC_IS_SBX_FE2KXT(unit)) {
            sbFe2000XtStartBubbleTimer(unit, 1, list_mgr_entries);
        } else {
            sbFe2000StartBubbleTimer(unit, 1, list_mgr_entries);
        }
    }

#if defined(BCM_QE2000_SUPPORT) && defined(BCM_FE2000_SUPPORT)
    /* Set the exceptions for OAM */
    rv = _oam_exceptions_set(unit, 1);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to configure exceptions\n")));
        _oam_cleanup(unit);
        return rv;
    }
#endif /* defined(BCM_QE2000_SUPPORT) && defined(BCM_FE2000_SUPPORT) */

    _state[unit]->mutex = sal_mutex_create("OAM_MUTEX");
    if (_state[unit]->mutex == NULL) {
        OAM_ERR((_SBX_D(unit, "failed to create mutex\n")));
        _oam_cleanup(unit);
        return BCM_E_RESOURCE;
    }

    queue_depth = SOC_SBX_CFG_FE2000(unit)->oam_q_depth;

    /* initialize the queue for communicating timer expirations from the
     * interrupt context to the expired timer callback thread
     */
    rv = _oam_tq_queue_init( OAM_TIMER_QUEUE(unit), queue_depth );
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed ot allocate timer queue\n")));
        _oam_cleanup(unit);
        return rv;
    }

    /* start the non-interrput thread to handle the expired timer callbacks */
    rv = _oam_thread_start(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to start oam cb thread rv=%d\n"), rv));
        _oam_cleanup(unit);
        return rv;
    }

    /* initialize the PortMDLevel2Etc table with fwd=1 */
    soc_sbx_g2p3_oamportmdlevel2etc_t_init(&pmd);
    SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
    SOC_PBMP_ITER(pbmp, port) {
        for (mdlvl = 0; mdlvl < 8; mdlvl++) {
            /* first set the down (0) MEP entries. */
            port_mdlvl = (0 << 9 ) + (port << 3) + mdlvl;
            rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdlvl, &pmd);
            if (rv != BCM_E_NONE) {
                OAM_ERR((_SBX_D(unit, "Failed port/mdlvl=(%d/%d) entry set\n"),port, mdlvl));
            }
            /* now set the up (1) MEP entries. */
            port_mdlvl = (1 << 9) + (port << 3) + mdlvl;
            rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdlvl, &pmd);
            if (rv != BCM_E_NONE) {
                OAM_ERR((_SBX_D(unit, "Failed port/mdlvl=(%d/%d) entry set\n"),port, mdlvl));
            }
        }
    } 

    /* Initialize the p2fti table to the drop FTE */
    size = soc_sbx_g2p3_p2fti_table_size_get(unit);
    soc_sbx_g2p3_p2fti_t_init(&p2fti);
    p2fti.ftidx = SBX_DROP_FTE(unit);

    for (dmac_type=0; dmac_type < 2 && BCM_SUCCESS(rv); dmac_type++) {
        for (port=0; port < (size/2) && BCM_SUCCESS(rv); port++) {
            rv = soc_sbx_g2p3_p2fti_set(unit, port, dmac_type, &p2fti);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to set p2fti[0x%x][%d]: %s\n"),
                         port, dmac_type, bcm_errmsg(rv)));
            }
        }
    }

    /* register the trunk port membership change callback */
    bcm_fe2000_trunk_change_register(unit, _oam_trunk_cb, NULL);

    /* Lastly, register for the watchdog timeout interrupt */
    soc_sbx_fe2000_register_isr(unit, SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG,
                                _oam_fe2000_wd_timeout_cb);

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_SCACHE_HANDLE_SET(_state[unit]->wb_hdl, unit, BCM_MODULE_OAM, 0);
    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, (int*)&size);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Only level 2 warm boot is supported by OAM\n")));
        _oam_cleanup(unit);
        return rv;
    }

    wb_size = (sizeof(fe2k_oam_wb_mem_layout_t) +
               SOC_WB_SCACHE_CONTROL_SIZE);

    /* Allocate a new chunk of the scache during a cold boot */
    if (!SOC_WARM_BOOT(unit) && (size > 0)) {
        rv = soc_scache_alloc(unit, _state[unit]->wb_hdl, wb_size);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to allocate scache for warm boot: %s\n"),
                     bcm_errmsg(rv)));
            _oam_cleanup(unit);
            return rv;
        }
    }

    /* Get the pointer for the Level 2 cache */
    wbc = NULL;
    if (size > 0) {
        rv = soc_scache_ptr_get(unit, _state[unit]->wb_hdl,
                                (uint8**)&wbc, (uint32*)&size);
        if (BCM_FAILURE(rv)) {
            _oam_cleanup(unit);
            return rv; 
        }
    }

    if (wbc) {
        soc_scache_handle_lock(unit, _state[unit]->wb_hdl);

        if (!SOC_WARM_BOOT(unit)) {
            wbc->version = OAM_WB_CURRENT_VERSION;
        }

        OAM_VERB((_SBX_D(unit, "Obtained scache pointer=0x%08x, %d bytes, "
                         "version=%d.%d\n"),
                  (int)wbc->cache, size,
                  SOC_SCACHE_VERSION_MAJOR(wbc->version),
                  SOC_SCACHE_VERSION_MINOR(wbc->version)));
        
        if (wbc->version > OAM_WB_CURRENT_VERSION) {
            OAM_ERR((_SBX_D(unit, "Upgrade scenario not supported.  "
                            "Current version=%d.%d  found %d.%d\n"),
                     SOC_SCACHE_VERSION_MAJOR(OAM_WB_CURRENT_VERSION),
                     SOC_SCACHE_VERSION_MINOR(OAM_WB_CURRENT_VERSION),
                     SOC_SCACHE_VERSION_MAJOR(wbc->version),
                     SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;

        } else if (wbc->version < OAM_WB_CURRENT_VERSION) {
            OAM_ERR((_SBX_D(unit, "Downgrade scenario not supported.  "
                            "Current version=%d.%d  found %d.%d\n"),
                     SOC_SCACHE_VERSION_MAJOR(OAM_WB_CURRENT_VERSION),
                     SOC_SCACHE_VERSION_MINOR(OAM_WB_CURRENT_VERSION),
                     SOC_SCACHE_VERSION_MAJOR(wbc->version),
                     SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;
        }

        if (BCM_SUCCESS(rv) && SOC_WARM_BOOT(unit)) {

            layout = (fe2k_oam_wb_mem_layout_t*)wbc->cache;
            rv = _oam_wb_recover(unit, layout);
        }

        soc_scache_handle_unlock(unit, _state[unit]->wb_hdl);
        
        if (BCM_FAILURE(rv)) {
            _oam_cleanup(unit);
            return rv;
        }

    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}

STATIC 
int _bcm_fe2000_validate_endpoint(int unit, 
                                  bcm_oam_endpoint_info_t *endpoint_info)
{
    int status = BCM_E_PARAM;

    if(!endpoint_info) {
        OAM_ERR((_SBX_D(unit, "Null endpoint info\n")));

    } else if (BCM_FAILURE(_oam_endpoint_type_supported(endpoint_info))) {
        OAM_ERR((_SBX_D(unit, "Endpoint type %d not supported\n"), 
                 endpoint_info->type));

    } else if ((endpoint_info->group > _state[unit]->max_endpoints) ||
               (endpoint_info->group == 0) ||
               (OAM_GROUP_INFO(unit, endpoint_info->group) == NULL)) {
        OAM_ERR((_SBX_D(unit, "Invalid oam group: %d\n"), endpoint_info->group));

    } else if ((endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) &&
               ((endpoint_info->id == 0) || 
                (endpoint_info->id > _state[unit]->max_endpoints)))  {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint id.  Must supply endpoint ID \n")));

    } else if ((endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE) &&
               ((endpoint_info->local_id == 0) ||
                (endpoint_info->local_id > _state[unit]->max_endpoints))) {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint id.  Remote EPs must supply "
                        "the associated local EP ID\n")));

    /* Accept NONE or INVALID too, in case of LSP BFD */
    } else if (!(OAM_IS_LSP_BFD(endpoint_info->type)) && 
               (endpoint_info->gport == BCM_GPORT_TYPE_NONE || endpoint_info->gport == BCM_GPORT_INVALID)) {
        OAM_ERR((_SBX_D(unit, "Invalid Gport id 0x%x\n"),endpoint_info->gport));

    } else {
        status = BCM_E_NONE;
    }

    return status;
}


/*
 *   Function
 *      bcm_fe2000_oam_endpoint_get
 *   Purpose
 *      Retrieve an oam endpoint with the given endpoint id
 *   Parameters
 *       unit           = BCM device number
 *       endpoint       = endpoint ID to retrieve
 *       endpoint_info  = storage location for found endpoint
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_endpoint_get(int unit, bcm_oam_endpoint_t endpoint, 
                            bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv;
    uint32_t ep_rec_index = 0;
    soc_sbx_g2p3_oamep_t oamep;
    uint32_t clear_persistent_faults;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
        break;
    }

    if (endpoint == 0 || endpoint > _state[unit]->max_endpoints) {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint.\n")));
        return BCM_E_PARAM;
    }

    /* save the flags for use later */
    clear_persistent_faults = endpoint_info->clear_persistent_faults;

    bcm_oam_endpoint_info_t_init(endpoint_info);
    endpoint_info->flags |= BCM_OAM_ENDPOINT_WITH_ID;
    endpoint_info->id = endpoint;

    OAM_VERB((_SBX_D(unit, "Entered oam_endpoint_get with id=0x%x.\n"), endpoint));

    OAM_LOCK(unit);

    if (oamBfd == _state[unit]->hash_data_store[endpoint].oam_type) {
        rv = _oam_bfd_endpoint_get(unit, endpoint_info, endpoint);

        OAM_VERB((_SBX_D(unit, "Persistent-Flags = 0x%8x, Flags = 0x%8x.\n"),
                  clear_persistent_faults, endpoint_info->flags));

        /* now check the RDI bit.  Only valid for Peer (remote) endpoint entries. */
        if ((clear_persistent_faults & BCM_OAM_ENDPOINT_FAULT_REMOTE) &&
            (endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {

            OAM_VERB((_SBX_D(unit, "Clear persistent faults set, checking Ack RDI... \n")));

            /* get the ucode endpoint entry. */
            rv = soc_sbx_g2p3_oamep_get(unit, endpoint, &oamep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"), 
                         endpoint, rv, bcm_errmsg(rv)));
                rv = BCM_E_INTERNAL;
            } else {

               /* clear the ack bit, acknowledging the host application received
                * the change of state in the received RDI bit from the peer.
                */
               oamep.ackrdi = 0;
               oamep.ackrdi_rdi = oamep.peerrdi;

               rv = soc_sbx_g2p3_oamep_set(unit, endpoint, &oamep);
               if (BCM_FAILURE(rv)) {
                   OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%x: %d %s\n"), 
                            endpoint, rv, bcm_errmsg(rv)));
                   rv = BCM_E_INTERNAL;
               } else {
                   endpoint_info->clear_persistent_faults =
                       ~BCM_OAM_ENDPOINT_FAULT_REMOTE & endpoint_info->clear_persistent_faults;
                   OAM_VERB((_SBX_D(unit, "Cleared Ack-Flag in oamEp %d. \n"), endpoint));
               }
            }
        }

    } else if (oamlsp1731 == _state[unit]->hash_data_store[endpoint].oam_type) {
        rv = _oam_mpls_endpoint_get(unit, endpoint_info, endpoint);

    } else if (oamPsc == _state[unit]->hash_data_store[endpoint].oam_type) {
        rv = _oam_psc_endpoint_get(unit, endpoint_info, endpoint);
    } else {
        rv = _oam_endpoint_idx_find(unit, endpoint_info, &ep_rec_index);

        /* verify found record index is allocated */
        rv = shr_idxres_list_elem_state(_state[unit]->ep_pool, ep_rec_index);
        if (rv != BCM_E_EXISTS) { 
            OAM_ERR((_SBX_D(unit, "Record not found\n")));
            rv =  BCM_E_NOT_FOUND;;
        }
        else {
            rv = BCM_E_NONE;
            OAM_VERB((_SBX_D(unit, "Found endpoint entry 0x%8x,  %d. \n"),
                      ep_rec_index, ep_rec_index));
        }

        OAM_VERB((_SBX_D(unit, "Persistent-Flags = 0x%8x, Flags = 0x%8x.\n"),
                  clear_persistent_faults, endpoint_info->flags));

        /* now check the RDI bit.  Only valid for Peer (remote) endpoint entries. */
        if ((clear_persistent_faults & BCM_OAM_ENDPOINT_FAULT_REMOTE) &&
            (endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {

            OAM_VERB((_SBX_D(unit, "Clear persistent faults set, checking Ack RDI... \n")));

            /* get the ucode endpoint entry. */
            rv = soc_sbx_g2p3_oamep_get(unit, endpoint, &oamep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"), 
                         endpoint, rv, bcm_errmsg(rv)));
                rv = BCM_E_INTERNAL;
            } else {

               /* clear the ack bit, acknowledging the host application received
                * the change of state in the received RDI bit from the peer.
                */
               oamep.ackrdi = 0;
               oamep.ackrdi_rdi = oamep.peerrdi;

               rv = soc_sbx_g2p3_oamep_set(unit, endpoint, &oamep);
               if (BCM_FAILURE(rv)) {
                   OAM_ERR((_SBX_D(unit, "Failed to set oamep 0x%x: %d %s\n"), 
                            endpoint, rv, bcm_errmsg(rv)));
                   rv = BCM_E_INTERNAL;
               } else {
                   endpoint_info->clear_persistent_faults =
                       ~BCM_OAM_ENDPOINT_FAULT_REMOTE & endpoint_info->clear_persistent_faults;
                   OAM_VERB((_SBX_D(unit, "Cleared Ack-Flag in oamEp %d. \n"), endpoint));
               }
            }
        }
    }

    OAM_UNLOCK(unit);

    endpoint_info->flags &= ~BCM_OAM_ENDPOINT_WITH_ID;

    return rv;
}


 

/*
 *   Function
 *      bcm_fe2000_oam_endpoint_create
 *   Purpose
 *      Create an oam endpoint and commit to hardware
 *   Parameters
 *       unit           = BCM device number
 *       endpoint_info  = description of endpoint to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 *
 *  BFD over LSP over trunk notes:
 *    1/ There is no gport associated with a LSP. We get to know if there is 
 *       a trunk underneath the LSP when we fetch the tunnel egress info
 *       None of the code under BCM_GPORT_IS_TRUNK is applicable in this case.
 *
 *    2/ Application can optionally specify a port within the trunk group 
 *       for BFD traffic using the trunk_index variable of the endpoint_info struct. 
 *       If incorrectly specified this defaults to index 0
 *
 *    3/ The selected port determines the QID to be used for BFD, this is 
 *       setup in _oam_egr_path_update. The endpoint->gport is internally 
 *       setup to reflect the MODPORT of the port over which the bfd traffic 
 *       has to be sent
 *
 *    4/ We also enqueue the lsp endpoint for callback processing, and rely 
 *       on the oam callback for trunks. 
 */
int
bcm_fe2000_oam_endpoint_create(int unit, 
                               bcm_oam_endpoint_info_t *endpoint_info)
{
    int rv, rv2;
    uint32_t ep_rec_index = 0, ep_rec_index2=0;
    oam_sw_hash_key_t hash_key;
    oam_sw_hash_key_t tmp_hash_key;
    oam_sw_hash_data_t *hash_data = NULL;
    tcal_id_t tcal_id = _BCM_TCAL_INVALID_ID;
    egr_path_desc_t egrPath;
    _oam_trunk_data_t *trunk_data = NULL;
    bcm_trunk_t tid = 0;
    bcm_trunk_add_info_t trunk_info;
    _fe2k_vpn_sap_t *vpn_sap;
    bcm_oam_endpoint_info_t ep_info;
    int new_hash = 0;

    if (!BCM_OAM_IS_INIT(unit)) {
        OAM_ERR((_SBX_D(unit, "Failed to initialize OAM Module\n")));
        return BCM_E_INIT;
    }

    OAM_LOCK (unit);

    egrPath.allocated = FALSE;

    rv = _bcm_fe2000_init_upmep(unit);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to initialize Up Mep\n")));
        goto exit;
    }

    /* Generic endpoint check */
    rv = _bcm_fe2000_validate_endpoint(unit, endpoint_info);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to validate endpoint\n")));
        goto exit;
    }

    if (OAM_IS_ETHERNET(endpoint_info->type)) {
       rv =  _bcm_fe2000_validate_oam_eth_endpoint(unit, endpoint_info);

    } else if (OAM_IS_LSP_BFD(endpoint_info->type)) {
        rv =  _bcm_fe2000_validate_oam_lsp_bfd_endpoint(unit, endpoint_info);
    } else if (OAM_IS_BFD(endpoint_info->type)) {
        rv =  _bcm_fe2000_validate_oam_bfd_endpoint(unit, endpoint_info);
        if (BCM_SUCCESS(rv)) {
            /* obtain encapsulation information from the Gport */
            rv = bcm_fe2000_mpls_gport_get(unit,
                                           endpoint_info->gport,
                                           &vpn_sap);
            if(BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to set mpls gport[0x%x] information %d\n"),
                         endpoint_info->gport, rv));
            }
        }

    } else if (OAM_IS_MPLS_Y1731(endpoint_info->type)) {
        /* validate end point */
        rv = _bcm_fe2000_validate_oam_mpls_endpoint(unit, endpoint_info);
    } else if (OAM_IS_PSC(endpoint_info->type)) {
        rv =  _bcm_fe2000_validate_oam_lsp_psc_endpoint(unit, endpoint_info);
    }

    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to validate specific endpoint type\n")));
        goto exit;
    }

    sal_memset(&egrPath, 0, sizeof(egrPath));
    sal_memset(&trunk_info, 0, sizeof(trunk_info));
    _init_ep_hash_key(unit, hash_key, endpoint_info);

    /* BFD over LSP over trunk notes:
     *    None of the code under BCM_GPORT_IS_TRUNK is applicable in this case.
     *    See notes under bcm_fe2000_oam_endpoint_create
     */
    if (BCM_GPORT_IS_TRUNK(endpoint_info->gport)) {
        /* only endpoints of Enet type can be created on trunks. */
        if(!OAM_IS_ETHERNET(endpoint_info->type)) {
            OAM_ERR((_SBX_D(unit, "Can not create endpoint of type %d, on trunks.\n"), endpoint_info->type));
            rv = BCM_E_PARAM;
            goto exit;
        }

        /* get tid from gport */
        tid = BCM_GPORT_TRUNK_GET(endpoint_info->gport);

        if (OAM_TID_VALID(tid) == FALSE) {
            rv = BCM_E_PARAM;
            goto exit;
        }

        trunk_data = &(_state[unit]->trunk_data[tid]);

        /* the application can configure multiple endpoints 
         * (at different md-levels) on the same trunk, so dont be surprised
         * if an entry already exists.
         */
        rv = bcm_trunk_get (unit, tid, &(trunk_data->add_info));
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get trunk info: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            goto exit;
        }

        sal_memcpy(&trunk_info, &(trunk_data->add_info), sizeof(trunk_info));
    }

    rv = shr_htb_find(_state[unit]->mamep_htbl, hash_key,
                      (shr_htb_data_t *)&hash_data,
                      0 /* don't remove */);

    if (BCM_SUCCESS(rv) && 
        !(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) 
    {
        OAM_ERR((_SBX_D(unit, "Attempted to add same entry without replace "
                        "flag set\n")));
        rv = BCM_E_EXISTS;
        goto exit;

    } else if (BCM_FAILURE(rv) && 
              (endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
        /* If we are replacing an endpoint, we need to get a hash hit,
         * otherwise there is no matching entry to replace.
         *   
         * If we are replacing an endpoint with_id, then we ignore the
         * hash hit, and replace the hash entry itself.
         */
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) {
            bcm_oam_endpoint_info_t_init(&ep_info);
            rv = bcm_fe2000_oam_endpoint_get(unit, endpoint_info->id, &ep_info);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get endpoint info for ep 0x%x: %d %s\n"),
                endpoint_info->id, rv, bcm_errmsg(rv)));
                goto exit;

            } else {
                _init_ep_hash_key(unit, tmp_hash_key, &ep_info);
                rv = shr_htb_find(_state[unit]->mamep_htbl, tmp_hash_key,
                                 (shr_htb_data_t *)&hash_data,
                                  1 /* remove old hash */);
                new_hash = 1;
            }   
        } else {
            OAM_ERR((_SBX_D(unit, "Replace flag set, but entry not found\n")));
            rv = BCM_E_NOT_FOUND;
            goto exit;
        }
    }

    /* Allocate an endpoint record, configure the record for local or peer,
     * allocate the necessary policer resource, and write the record out to
     * the ilib for ucode access.
     */
    if (endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) {
            ep_rec_index = endpoint_info->id;
        } else {
            /* At this point, hash_data is valid */
            ep_rec_index = hash_data->rec_idx;
        }

        /* verify found record index is allocated */
        rv = shr_idxres_list_elem_state(_state[unit]->ep_pool, ep_rec_index);

        /* caller must have added a record at this given MAID & MEP prior to
         * calling with the REPLACE flag
         */
        if (rv != BCM_E_EXISTS) {
            OAM_ERR((_SBX_D(unit, "Didn't find endpoint %d\n"), endpoint_info->id));
            goto exit;
        }

    } else {
        /* new entry, allocate a record from the database pool.
         * check if a particular endpoint-id has been requested. */
        if (endpoint_info->flags & BCM_OAM_ENDPOINT_WITH_ID) {
            /* set the requested ID */
            ep_rec_index = endpoint_info->id;

            if ((OAM_IS_BFD(endpoint_info->type)) || \
                (OAM_IS_MPLS_Y1731(endpoint_info->type)) || \
                (OAM_IS_LSP_BFD(endpoint_info->type))) {
                /* check if the requested ID has already been alocated. */
                rv = shr_idxres_list_elem_state(_state[unit]->ep_pool, ep_rec_index);
                if (rv != BCM_E_NOT_FOUND) {
                    OAM_ERR((_SBX_D(unit, "Endpoint %d already in use:"
                                    "%d %s\n"), ep_rec_index, rv, bcm_errmsg(rv)));
                    goto exit;
                }                
                
            } else {
                if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
                    ep_rec_index2 = ep_rec_index + 1;
                    rv2 = shr_idxres_list_elem_state(_state[unit]->ep_pool, ep_rec_index2);
                } else {
                    ep_rec_index2 = ep_rec_index;
                    rv2 = BCM_E_NOT_FOUND;
                }

                /* check if the requested ID has already been alocated. */
                rv = shr_idxres_list_elem_state(_state[unit]->ep_pool, ep_rec_index);
                if ((rv != BCM_E_NOT_FOUND) || (rv2 != BCM_E_NOT_FOUND)) {
                    OAM_ERR((_SBX_D(unit, "Endpoint %d already in use:"
                                    "%d %s\n"), ep_rec_index, rv, bcm_errmsg(rv)));
                    goto exit;
                } else {
                    /* reserve the requested endpoint-id */
                    rv = shr_idxres_list_reserve(_state[unit]->ep_pool, ep_rec_index, ep_rec_index2);

                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Endpoint %d couldnt be reserved:"
                                        "%d %s\n"), ep_rec_index, rv, bcm_errmsg(rv)));
                        goto exit;
                    }
                }
            }
        } else {
            rv = shr_idxres_list_alloc(_state[unit]->ep_pool, &ep_rec_index);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocate an endpoint record: %d %s\n"),
                         rv, bcm_errmsg(rv)));
                goto exit;
            }
        }
    }

    hash_data = &_state[unit]->hash_data_store[ep_rec_index];

    /* this rv could be BCM_E_EXISTS or other expected "error" codes
     * depending on the path(s) taken above.  rv is checked again below,
     * and it is expected that rv be set to BCM_E_NONE.  so clear any
     * error codes now.
     */
    rv = BCM_E_NONE;

    /* local endpoints need an egress path, allocate, or find on replace */
    if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {

        if (endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE) { 
            rv = _oam_egr_path_get(unit, &egrPath, ep_rec_index);

        } else if ((endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING) == 0) {
            /* allocate a new egress path, for down meps only */
            rv = _oam_egr_path_alloc(unit, 
                                     endpoint_info->src_mac_address, &egrPath);
        }

        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to allocate/retrieve egress path for "
                            "endpoint %d: %d %s\n"), endpoint_info->id, rv, 
                     bcm_errmsg(rv)));
            goto exit;
        }

        /* BFD egress path */
        if (OAM_IS_BFD(endpoint_info->type)) {
            rv = _oam_bfd_egr_path_update(unit, &egrPath, endpoint_info, vpn_sap);

        } else if ((OAM_IS_LSP_BFD(endpoint_info->type)) ||
                   (OAM_IS_MPLS_Y1731(endpoint_info->type)) ||
                   (OAM_IS_PSC(endpoint_info->type))) {
            rv = _oam_mpls_egr_path_update(unit, &egrPath, endpoint_info);
        }

        if(BCM_SUCCESS(rv)) {
            rv = _oam_egr_path_update(unit, &egrPath, endpoint_info, &trunk_info);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to update egress path for EP %d\n"),
                         endpoint_info->id));
            }
        }

    } else {  /* REMOTE endpoint */
        egrPath.ftIdx = 0;
    }

    if(BCM_SUCCESS(rv)) {   
        if (OAM_IS_ETHERNET(endpoint_info->type)) {
            rv = _oam_enet_endpoint_set(unit, endpoint_info, ep_rec_index,
                                        &egrPath, &tcal_id, &trunk_info);

        } else if (OAM_IS_BFD(endpoint_info->type)) {
            rv = _oam_bfd_endpoint_set(unit, endpoint_info, ep_rec_index,
                                       &egrPath, &tcal_id, 
                                       vpn_sap->vc_mpls_port.match_label);
        } else if (OAM_IS_LSP_BFD(endpoint_info->type)) {
            rv = _oam_bfd_endpoint_set(unit, endpoint_info, ep_rec_index,
                                       &egrPath, &tcal_id, 
                                       endpoint_info->mpls_label);

        } else if (OAM_IS_MPLS_Y1731(endpoint_info->type)) {
            rv = _oam_mpls_endpoint_set(unit, endpoint_info, ep_rec_index,
                                       &egrPath, &tcal_id);            
        } else if (OAM_IS_PSC(endpoint_info->type)) {
            rv = _oam_psc_endpoint_set(unit, endpoint_info, ep_rec_index,
                                       &egrPath, &tcal_id, 
                                       endpoint_info->mpls_label);

        } else {
            OAM_ERR((_SBX_D(unit, "Unsupported endpoint type:%d\n"), endpoint_info->type));
            rv = BCM_E_PARAM;
        }
    }
    

    if (BCM_FAILURE(rv)) {

        /* free record on failure only if this is a new entry, that is, not
         * a REPLACE
         */
        if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
            if (ep_rec_index) {
                shr_idxres_list_free(_state[unit]->ep_pool, ep_rec_index);
            }
            if(_BCM_TCAL_ID_VALID(tcal_id)) {
                _bcm_tcal_free(unit, &tcal_id);
            }
        }

    } else {

        hash_data->vid_label = endpoint_info->vlan;
        
        /* add the endpoint entry to the trunk's list of endpoints
         * if the gport is a trunk or if the LSP is over a trunk */
        if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
            if (egrPath.is_trunk) {
                tid = egrPath.trunk_id;
                trunk_data = &(_state[unit]->trunk_data[tid]);
            }
            if (BCM_GPORT_IS_TRUNK(endpoint_info->gport) || egrPath.is_trunk) {
                /* insert this entry at start of list. */
                DQ_INSERT_HEAD(&trunk_data->endpoint_list, &hash_data->trunk_list_node);
                OAM_VERB((_SBX_D(unit, "Adding hash_data 0x%8x, ep_idx %d to trunk"
                                " list\n"),
                          hash_data, ep_rec_index));
            }
        }
        
        hash_data->rec_idx     = ep_rec_index;
        hash_data->ep_name     = endpoint_info->name;
        hash_data->flags       = endpoint_info->flags;
        hash_data->type        = endpoint_info->type;
        hash_data->group       = endpoint_info->group;
        hash_data->gport       = endpoint_info->gport;
        hash_data->tcal_id     = tcal_id;
        hash_data->ing_map     = endpoint_info->ing_map;
        hash_data->egr_map     = endpoint_info->egr_map;
        hash_data->intf_id     = endpoint_info->intf_id;
        /* By caching the trunk id, we save on invoking set of API calls 
         * to get trunk id during endpoint_delete
         */
        hash_data->lsp_trunk_id = (egrPath.is_trunk ?  egrPath.trunk_id : SBX_MAX_TRUNKS);
        hash_data->trunk_index = endpoint_info->trunk_index;
        
        if (OAM_IS_ETHERNET(endpoint_info->type)) {
            hash_data->oam_type    = oamEp;
            hash_data->maid_cookie = endpoint_info->local_id; /* local EP's ID */
            hash_data->mdlevel     = endpoint_info->level;
            hash_data->interval    = endpoint_info->ccm_period;
            hash_data->direction = !!(endpoint_info->flags & BCM_OAM_ENDPOINT_UP_FACING);

        } else if (OAM_IS_BFD(endpoint_info->type)) {
            hash_data->oam_type  = oamBfd;
            hash_data->vid_label = vpn_sap->vc_mpls_port.match_label;
            hash_data->bfd_flags = endpoint_info->bfd_flags;

            if(!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
                hash_data->bfd_interval = endpoint_info->min_tx_interval;
                hash_data->local_rec_idx = ep_rec_index;
            } else {
                hash_data->bfd_interval = endpoint_info->min_rx_interval;
                hash_data->local_rec_idx = endpoint_info->local_id;
            }

        } else if (OAM_IS_LSP_BFD(endpoint_info->type)) {
            hash_data->oam_type  = oamBfd;
            hash_data->vid_label = endpoint_info->mpls_label;
            hash_data->bfd_flags = endpoint_info->bfd_flags;

            if(!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
                hash_data->bfd_interval = endpoint_info->min_tx_interval;
                hash_data->local_rec_idx = ep_rec_index;
            } else {
                hash_data->bfd_interval = endpoint_info->min_rx_interval;
                hash_data->local_rec_idx = endpoint_info->local_id;
            }

        } else if (OAM_IS_MPLS_Y1731(endpoint_info->type)) {
            hash_data->oam_type  = oamlsp1731;
            /* only Interface ID are used to infer Tunnel information */
            /*hash_data->vid_label = endpoint_info->mpls_label;*/

            hash_data->local_rec_idx = (endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE) ? \
                                           endpoint_info->local_id:ep_rec_index;
        } else if (OAM_IS_PSC(endpoint_info->type)) {
            hash_data->oam_type  = oamPsc;
            if(!(endpoint_info->flags & BCM_OAM_ENDPOINT_REMOTE)) {
                hash_data->local_rec_idx = ep_rec_index;
                hash_data->psc_slow_rate = 0;
                hash_data->psc_burst     = 0;
            } else {
                hash_data->local_rec_idx = endpoint_info->local_id;
                hash_data->vid_label = endpoint_info->mpls_label;
            }
        }

        sal_memcpy( hash_data->mac_address, endpoint_info->src_mac_address,
                    sizeof(bcm_mac_t));

        /* Add to hash table if the entry is newly added/ replaced */
        if (!(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE) || (new_hash)) {
            rv = shr_htb_insert(_state[unit]->mamep_htbl, hash_key, hash_data);
        }

        endpoint_info->id = ep_rec_index;

        /* back out the endpoint add only if its not a replace */
        if (BCM_FAILURE(rv) && 
            !(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {

            OAM_ERR((_SBX_D(unit, "Duplicate MAID/MEP\n")));
            /* don't re-fetch the data, use the record id currently allocated.
             * The record found by hash may be a duplicate, thus getting the
             * duplicate data will delete the valid entry, not _this_
             * invalid duplicate
             */
            bcm_oam_endpoint_destroy(unit, endpoint_info->id);
        }
    }

    if (BCM_SUCCESS(rv) && 
        !(endpoint_info->flags & BCM_OAM_ENDPOINT_REPLACE)) {
        rv = oam_group_ep_list_add(unit, 
                                   endpoint_info->group,
                                   &hash_data->list_node);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to add endpoint id 0x%x to group "
                            "tracking state: %d %s\n"), 
                     endpoint_info->id, rv, bcm_errmsg(rv)));
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rv)) {
        rv = oam_ep_store(unit, endpoint_info);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */
exit:
    if(BCM_FAILURE(rv)) {
        if(egrPath.allocated) {
            _oam_egr_path_free(unit, &egrPath);
        }
    }

    OAM_UNLOCK(unit);
    
    return rv;
}


/*
 *   Function
 *      bcm_fe2000_oam_endpoint_destroy
 *   Purpose
 *      Destroy an oam endpoint and all allocated resources  with the given 
 *      endpoint id
 *   Parameters
 *       unit           = BCM device number
 *       endpoint       = endpoint ID to destroy
 *   Returns
 *       BCM_E_*
 *  Notes:
 *  BFD over LSP over trunk notes:
 *    1/ There is no gport associated with a LSP. We get to know if there is 
 *       a trunk underneath the LSP when we fetch the tunnel egress info
 *       None of the code under BCM_GPORT_IS_TRUNK is applicable in this case.
 *
 *    2/ Application can optionally specify a port within the trunk group 
 *       for BFD traffic using the trunk_index variable of the endpoint_info
 *       If incorrectly specified this defaults to index 0
 *
 *    3/ The selected port determines the QID to be used for BFD, this is 
 *       setup in _oam_egr_path_update. The endpoint->gport is internally 
 *       setup to reflect the MODPORT of the port over which the bfd traffic 
 *       has to be sent
 *
 *    4/ We also make sure that the endpoint is removed from the trunk_list
 *
 */
int
bcm_fe2000_oam_endpoint_destroy(int unit, bcm_oam_endpoint_t endpoint)
{
    int rv;
    bcm_oam_endpoint_info_t ep_info;
    egr_path_desc_t egrPath;
    oam_sw_hash_key_t hash_key;
    oam_sw_hash_data_t *hash_data = NULL;
    bcm_trunk_t tid = 0;
    _oam_trunk_data_t *trunk_data = NULL;
    bcm_trunk_add_info_t trunk_info;
    int is_trunk = 0;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_NONE;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    /* oam_endpoint_get will clear the ackrdi bit in the ucode oamep table entry,
     * if ep_info.clear_persistent_faults & BCM_OAM_ENDPOINT_FAULT_REMOTE is true.
     * so, ep_info can not be used uninitialized with the oam_endpoint_get function.
     * always clear out ep_info before sending it into oam_endpoint_get.
     */
    bcm_oam_endpoint_info_t_init(&ep_info);
    rv = bcm_fe2000_oam_endpoint_get(unit, endpoint, &ep_info);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get endpoint info for ep 0x%x: %d %s\n"),
                 endpoint, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* For BFD, verify remote is destroyed before local endpoint */
    if (OAM_IS_BFD(ep_info.type) ||
        OAM_IS_LSP_BFD(ep_info.type)) {
        if (OAM_GROUP_EP_LIST_EMPTY(unit, ep_info.group)) {
            OAM_ERR((_SBX_D(unit, "Group has no Endpoints associated. \n")));
            return BCM_E_INTERNAL;

        } else {
            dq_p_t ep_elem;
            uint8 local_ep_count = 0, peer_ep_count = 0;

            DQ_TRAVERSE(&OAM_GROUP_EP_LIST(unit, ep_info.group), ep_elem) {
                oam_sw_hash_data_t *hash_data = NULL;
                _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(ep_elem, hash_data);

                if (hash_data->flags & BCM_OAM_ENDPOINT_REMOTE) {   
                    peer_ep_count++;
                } else {
                    local_ep_count++;
                }
            } DQ_TRAVERSE_END(&trunk_data->endpoint_list, ep_elem);

            if (ep_info.flags & BCM_OAM_ENDPOINT_REMOTE) {
                if(!local_ep_count) {
                    OAM_ERR((_SBX_D(unit, "Local Endpoing must Exist."
                                    " Peer must be destroyed before local endpoint\n")));
                    return BCM_E_INTERNAL;

                } else if (!peer_ep_count) {
                    OAM_ERR((_SBX_D(unit, "Missing Remote Endpoint!!!\n")));
                    return BCM_E_PARAM;      
    
                }      
            } else {
                /* local endpoint */
                if (!local_ep_count) {
                    OAM_ERR((_SBX_D(unit, "Missing Local Endpoint!!!\n")));
                    return BCM_E_PARAM;

                } else if (peer_ep_count) {
                    OAM_ERR((_SBX_D(unit, "Peer BFD endpoint must be destroyed" 
                                    " before local endpoint destroy!!!\n")));
                    return BCM_E_PARAM;
                }
            }
        }
    }

    if (!(ep_info.flags & BCM_OAM_ENDPOINT_REMOTE) &&
        !(ep_info.flags & BCM_OAM_ENDPOINT_UP_FACING)) {
        rv = _oam_egr_path_get(unit, &egrPath, ep_info.id);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get egress path for epi=0x%x\n"), ep_info.id));
            OAM_ERR((_SBX_D(unit, "ep_info.flags=0x%x\n"), ep_info.flags));
            return rv;
        }
    }

    _init_ep_hash_key(unit, hash_key, &ep_info);

    shr_htb_find(_state[unit]->mamep_htbl, hash_key,
                 (shr_htb_data_t *)&hash_data,
                 0 /* do not remove */);

    /* BFD over LSP over trunk notes:
     *    None of the code under BCM_GPORT_IS_TRUNK is applicable in this case.
     *    See notes under bcm_fe2000_oam_endpoint_create
     */
    if (BCM_GPORT_IS_TRUNK(ep_info.gport)) {
        /* only endpoints of Enet type can be created on trunks. */
        if(!OAM_IS_ETHERNET(ep_info.type)) {
            OAM_ERR((_SBX_D(unit, "Can not create endpoint of type %d, "
                            "on trunks.\n"), 
                     ep_info.type));
            return BCM_E_PARAM;
        }

        /* get tid from gport */
        tid = BCM_GPORT_TRUNK_GET(ep_info.gport);

        if (OAM_TID_VALID(tid) == FALSE) {
            OAM_ERR((_SBX_D(unit, "Invalid trunk ID: %d\n"), tid));
            return BCM_E_PARAM;
        }
        is_trunk = 1;
    } else if ((hash_data) && OAM_TID_VALID(hash_data->lsp_trunk_id)) {
        /* This is an oam ep, over an lsp over a trunk */
        tid = hash_data->lsp_trunk_id;
        is_trunk = 1;
    }

    if (is_trunk) {
        trunk_data = &(_state[unit]->trunk_data[tid]);

        /* need to save a copy of trunk_info (trunk_add_info)
         * because we may remove the trunk_data structure.
         */
        sal_memcpy(&trunk_info, &(trunk_data->add_info), sizeof(trunk_info));

        /* if the endpoint is created on a trunk, then remove the
         * endpoint from the trunk's list.  if trunk's list is empty, 
         * then remove the trunk entry. */
        OAM_VERB((_SBX_D(unit, "Remove endpoint from trunk list\n")));
        rv = _oam_remove_ep_from_trunk_list(unit, tid, &hash_data->trunk_list_node);
        if (DQ_EMPTY(&trunk_data->endpoint_list)) {
            _oam_delete_trunk_info(unit, tid);
        }
    }

    if (OAM_IS_ETHERNET(ep_info.type)) {
        rv = _oam_enet_endpoint_delete(unit, &ep_info, &trunk_info);

    } else if (OAM_IS_BFD(ep_info.type) || OAM_IS_LSP_BFD(ep_info.type)) {
        rv = _oam_bfd_endpoint_delete(unit, &ep_info);

    } else if (OAM_IS_MPLS_Y1731(ep_info.type)) {
        rv = _oam_mpls_endpoint_delete(unit, &ep_info);

    } else if (OAM_IS_PSC(ep_info.type)) {
        rv = _oam_psc_endpoint_delete(unit, &ep_info);

    } else {
        rv = BCM_E_BADID;
    }

    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to destroy endpoint: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
        /* non-fatal, keep trying */
    }

    if (!(ep_info.flags & BCM_OAM_ENDPOINT_REMOTE) &&
        !(ep_info.flags & BCM_OAM_ENDPOINT_UP_FACING)) {
        _oam_egr_path_free(unit, &egrPath);
    }

    shr_idxres_list_free(_state[unit]->ep_pool, ep_info.id);

    shr_htb_find(_state[unit]->mamep_htbl, hash_key,
                 (shr_htb_data_t *)&hash_data,
                 1 /* remove */);

    if (hash_data) {
        oam_group_ep_list_remove(unit, ep_info.group, &hash_data->list_node);
        _bcm_tcal_free(unit, &hash_data->tcal_id);
        _oam_hash_data_clear(hash_data);
    }

    OAM_VERB((_SBX_D(unit, "Endpoint 0x%x destroyed\n"), endpoint));

    return rv;
}

/*
 *   Function
 *      bcm_fe2000_oam_endpoint_destroy_all
 *   Purpose
 *      Destroy all endpoints associated with the given group
 *   Parameters
 *       unit         = BCM device number
 *       group        = endpoints belonging to this group id will be destroyed
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_endpoint_destroy_all(int unit, bcm_oam_group_t group)
{
    int rv;
    dq_p_t ep_elem;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_NONE;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    rv = shr_idxres_list_elem_state(_state[unit]->group_pool, group);

    if (rv != BCM_E_EXISTS) {
        OAM_ERR((_SBX_D(unit, "oam group id %d does not exist!\n"), group));
        return BCM_E_NOT_FOUND;
    }

    /* iterate through all the endpoints in the group and destroy */
    rv = BCM_E_NONE;
    OAM_LOCK(unit);
    if(!OAM_GROUP_EP_LIST_EMPTY(unit, group)) {
        DQ_TRAVERSE(&OAM_GROUP_EP_LIST(unit,group), ep_elem) {
            int tmp_rv;
            oam_sw_hash_data_t *hash_data = NULL;
            _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(ep_elem, hash_data);
            
            OAM_VERB((_SBX_D(unit, "Destroying endpoint 0x%x\n"), hash_data->rec_idx));
            
            tmp_rv = bcm_fe2000_oam_endpoint_destroy(unit, hash_data->rec_idx);
            if (BCM_FAILURE(tmp_rv)) {
                OAM_ERR((_SBX_D(unit, "Error destrying endpoint 0x%x: %d %s\n"), 
                         hash_data->rec_idx, rv, bcm_errmsg(rv)));
                
                /* save the first error, and keep trying */
                if (BCM_SUCCESS(rv)) { 
                    rv  = tmp_rv; 
                }            
            }
        } DQ_TRAVERSE_END(&OAM_GROUP_EP_LIST(unit,group), ep_elem);
    }

    OAM_UNLOCK(unit);

    return rv;
}


/*
 *   Function
 *      bcm_fe2000_oam_endpoint_traverse
 *   Purpose
 *      Traverse the set of active endpoints in a given group
 *   Parameters
 *       unit        = BCM device number
 *       group       = group id to traverse
 *       cb          = callback invoked for each active endpoint
 *       user_data   = user data passed to each invocation of the callback
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_endpoint_traverse(int unit,  bcm_oam_group_t group, 
                                 bcm_oam_endpoint_traverse_cb cb,  
                                 void *user_data)
{
    int rv = BCM_E_NONE;
    bcm_oam_endpoint_info_t epInfo;
    dq_p_t ep_elem;


    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_NONE;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    rv = shr_idxres_list_elem_state(_state[unit]->group_pool, group);

    if (rv != BCM_E_EXISTS) {
        OAM_ERR((_SBX_D(unit, "oam group id %d does not exist!\n"), group));
        return BCM_E_NOT_FOUND;
    }

    rv = BCM_E_NONE;
    OAM_LOCK(unit);
    if(!OAM_GROUP_EP_LIST_EMPTY(unit, group)) {

        DQ_TRAVERSE(&OAM_GROUP_EP_LIST(unit,group), ep_elem) {
            oam_sw_hash_data_t *hash_data = NULL;
            int tmp_rv;

            _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(ep_elem, hash_data);

            bcm_oam_endpoint_info_t_init(&epInfo);
            
            tmp_rv = bcm_fe2000_oam_endpoint_get(unit, hash_data->rec_idx, &epInfo);
            if (BCM_FAILURE(tmp_rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to get epInfo on active endpoint 0x%x:"
                                " %d %s\n"), hash_data->rec_idx, tmp_rv, bcm_errmsg(tmp_rv)));
                
                /* non-fatal, keep trying, but keep first error */
                if (BCM_SUCCESS(rv)) {
                    rv = tmp_rv;
                }
            } else {
                (cb) (unit, &epInfo, user_data);    
            }
            
        } DQ_TRAVERSE_END(&OAM_GROUP_EP_LIST(unit,group), ep_elem);
    }

    OAM_UNLOCK(unit);
    return rv;
}

/*
 *   Function
 *      bcm_fe2000_oam_event_register
 *   Purpose
 *      Register a callback for various oam events
 *   Parameters
 *       unit           = BCM device number
 *       event_types    = event to cause an invocation of the callback
 *       cb             = user callback
 *       user_data      = user supplied data, passed to user callback
 *                        when invoked
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_event_register(int unit, bcm_oam_event_types_t event_types, 
                              bcm_oam_event_cb cb, void *user_data)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    
    OAM_INTR_LOCK;
    _state[unit]->wd_cb_data.epCb = cb;
    _state[unit]->wd_cb_data.userData = user_data;
    OAM_INTR_UNLOCK;

    return BCM_E_NONE;
}


/*
 *   Function
 *      bcm_fe2000_oam_event_unregister
 *   Purpose
 *      Unregister a callback from an event
 *   Parameters
 *       unit        = BCM device number
 *       event_types = event to unregister
 *       cb          = callback to unregister
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_event_unregister(int unit, bcm_oam_event_types_t event_types, 
                                bcm_oam_event_cb cb)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    
    OAM_INTR_LOCK;
    _state[unit]->wd_cb_data.epCb = NULL;
    _state[unit]->wd_cb_data.userData = NULL;
    OAM_INTR_UNLOCK;

    return BCM_E_NONE;
}


/*
 *   Function
 *      bcm_fe2000_oam_group_create
 *   Purpose
 *      Create an oam group, stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *       group_info  = description of group to create
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_group_create(int unit, bcm_oam_group_info_t *group_info)
{    
    int rv;
    int allocate_group_info;
    int ep_count = 0;
    int maid_change, rdi_change;
    soc_sbx_g2p3_oamep_t oamep;
    dq_p_t ep_elem;


    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    /* On replace with id, verify the group exists
     * On with id, reserve the group id & allocate group_info
     * else, allocate a new group id, and allocate group_info
     */
    OAM_LOCK(unit);
    if ((group_info->flags & BCM_OAM_GROUP_WITH_ID) &&
        (group_info->flags & BCM_OAM_GROUP_REPLACE)) 
    {
       
        allocate_group_info = 0;
        rv = shr_idxres_list_elem_state(_state[unit]->group_pool, 
                                        group_info->id);
        if (rv != BCM_E_EXISTS) {
            OAM_ERR((_SBX_D(unit, "Attempted to replace non-existent oam group id %d\n"),
                     group_info->id));
            rv = BCM_E_PARAM;
            goto Exit;
        }
        rv = BCM_E_NONE;

        ep_count = oam_group_ep_list_count(unit, group_info->id);
        maid_change = sal_memcmp(group_info->name, OAM_GROUP_INFO(unit, group_info->id)->name,
                                 sizeof(group_info->name));
        rdi_change = ((group_info->flags & BCM_OAM_GROUP_REMOTE_DEFECT_TX) !=
                      (OAM_GROUP_INFO(unit, group_info->id)->flags & BCM_OAM_GROUP_REMOTE_DEFECT_TX));

        /* if the MAID changed and there are active EP's in this group,
         * then fail the request.
         */
        if ((maid_change != 0) && (ep_count != 0)) {
            OAM_VERB((_SBX_D(unit, "Replacing group's name %d while group contains %d active endpoints.\n"),
                      group_info->id, ep_count));
            rv = BCM_E_PARAM;
            goto Exit;
        }

        /* if the RDI bit changed and there are active EP's in this group,
         * then iterate through all active endpoints and update the RDI flag in hw.
         */
        if ((rdi_change != 0) && (ep_count != 0)) {
            if(!OAM_GROUP_EP_LIST_EMPTY(unit, group_info->id)) {
                DQ_TRAVERSE(&OAM_GROUP_EP_LIST(unit,group_info->id), ep_elem) {
                    oam_sw_hash_data_t *hash_data = NULL;
                    _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(ep_elem, hash_data);

                    soc_sbx_g2p3_oamep_t_init(&oamep);
                    rv = soc_sbx_g2p3_oamep_get(unit, hash_data->rec_idx, &oamep);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                                 hash_data->rec_idx, rv, bcm_errmsg(rv)));
                        rv = BCM_E_PARAM;
                        goto Exit;
                    }

                    /* only update the local entries.  peer (and second local) entries
                     * do not have the rdi field in them.
                     */
                    if ((oamep.function == _state[unit]->ep_type_ids.ccm_first) &&
                        (oamep.type == _state[unit]->ep_transport.enet)) {

                        /* if the oamEp type doesnt support RDI, setting this
                         * bit will not affect the member fields supported by
                         * this endpoint type because _oam_oamep_prepare will
                         * over-write the rdi flag.
                         */
                        oamep.rdi = !!(group_info->flags & BCM_OAM_GROUP_REMOTE_DEFECT_TX);

                        /* the soc layer doesn't handle overlays very well.
                         * Must clear all fields not related to this type to ensure
                         * bits are set as expected
                         */
                        _oam_oamep_prepare(unit, &oamep);

                        rv = soc_sbx_g2p3_oamep_set(unit, hash_data->rec_idx, &oamep);
                        if (BCM_FAILURE(rv)) {
                            OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                                     hash_data->rec_idx, rv, bcm_errmsg(rv)));
                            rv = BCM_E_PARAM;
                            goto Exit;
                        }
                    }
                } DQ_TRAVERSE_END(&OAM_GROUP_EP_LIST(unit,group_info->id), ep_elem);
            }
        }
    } else if (group_info->flags & BCM_OAM_GROUP_WITH_ID) {

        allocate_group_info = 1;
        rv = shr_idxres_list_reserve(_state[unit]->group_pool,
                                     group_info->id, group_info->id);
        if (BCM_FAILURE(rv)) {
            if (rv == BCM_E_RESOURCE) {
                rv = BCM_E_EXISTS;
            }
            
            OAM_ERR((_SBX_D(unit, "Failed to reserve oam group id %d: %d %s\n"),
                     group_info->id, rv, bcm_errmsg(rv)));
            goto Exit;
        }

    } else if (group_info->flags & BCM_OAM_GROUP_REPLACE) {

        OAM_ERR((_SBX_D(unit, "Invalid flag - must replace with id. Flags "
                        "passed=0x%x\n"), group_info->flags));
        rv = BCM_E_PARAM;
        goto Exit;
    } else {
        allocate_group_info = 1;
        rv = shr_idxres_list_alloc(_state[unit]->group_pool,
                                   (shr_idxres_element_t *)&group_info->id);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to allocate oam group id: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            goto Exit;
        }
    }

    if (group_info->id > _state[unit]->max_endpoints) {
        OAM_ERR((_SBX_D(unit, "Invalid group_id:%d\n"), group_info->id));
        return BCM_E_PARAM;
    }

    if (allocate_group_info) {
        OAM_GROUP_INFO(unit, group_info->id) = 
            sal_alloc(sizeof(bcm_oam_group_info_t), "oam group info");

        if (OAM_GROUP_INFO(unit, group_info->id) == NULL) {
            OAM_ERR((_SBX_D(unit, "Failed to allocate group_info state\n")));
            rv = BCM_E_MEMORY;
            goto Exit;
        }

        /* New group - no associated endpoints */
        DQ_INIT(&OAM_GROUP_EP_LIST(unit, group_info->id));
    }

Exit:
    OAM_UNLOCK(unit);
    if (BCM_SUCCESS(rv)) {
        sal_memcpy(OAM_GROUP_INFO(unit, group_info->id), group_info, 
                   sizeof(bcm_oam_group_info_t));
        
        OAM_VERB((_SBX_D(unit, "Created oam_group id=%d\n"), group_info->id));
    } else {
        OAM_ERR((_SBX_D(unit, "Failed to create oam group: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
    }
    
    return rv;
}

/*
 *   Function
 *      bcm_fe2000_oam_group_destroy
 *   Purpose
 *      Destroy an oam group, stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *       group       = group id to destroy
 *   Returns
 *       BCM_E_*
 *  Notes:
 *      Any associated endpoint will also be destroyed
 */
int
bcm_fe2000_oam_group_destroy(int unit, bcm_oam_group_t group)
{
    int rv;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    OAM_LOCK(unit);

    rv = shr_idxres_list_elem_state(_state[unit]->group_pool, group);

    if (rv != BCM_E_EXISTS) {
        OAM_UNLOCK(unit);
        OAM_ERR((_SBX_D(unit, "Attempted to destroy non-existent oam group id %d\n"),
                 group));
        return BCM_E_PARAM;
    }


    /* clear out any endpoints, ignore error */
    bcm_fe2000_oam_endpoint_destroy_all(unit, group);

    rv = shr_idxres_list_free(_state[unit]->group_pool, group);
    if (BCM_FAILURE(rv)) {
        OAM_WARN((_SBX_D(unit, "Failed to free oam group id: %d %s\n"),
                  rv, bcm_errmsg(rv)));
        /* non-fatal, keep trying to destroy this group */
    }

    if (OAM_GROUP_INFO(unit, group)) {
        sal_free(OAM_GROUP_INFO(unit, group));
        OAM_GROUP_INFO(unit, group) = NULL;
    }
    OAM_UNLOCK(unit);

    OAM_VERB((_SBX_D(unit, "Destroyed oam_group id=%d\n"), group));

    return rv;
}

/*
 *   Function
 *      bcm_fe2000_oam_group_destroy_all
 *   Purpose
 *      Destroy all empty and active oam groups stored in soft state on SBX.
 *   Parameters
 *       unit        = BCM device number
 *   Returns
 *       BCM_E_*
 *  Notes:
 *      If a non-empty group is found, the group is not destroyed, but 
 *      continues to destroy the remaining groups.
 */
int
bcm_fe2000_oam_group_destroy_all(int unit)
{
    int group;
    int rv, rv_tmp;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    OAM_LOCK(unit);
    rv = BCM_E_NONE;
    for (group = 0; group < _state[unit]->max_endpoints; group++) {
        if (OAM_GROUP_INFO(unit, group)) {    

            rv_tmp = bcm_fe2000_oam_group_destroy(unit, group);
            if (BCM_FAILURE(rv_tmp)) {
                OAM_WARN((_SBX_D(unit, "Failed to destroy group %d: %d %s\n"), group,
                          rv_tmp, bcm_errmsg(rv)));
                /* Non-fatal error, keep trying, but keep last error */
                rv = rv_tmp;
            }
        }
    }
    OAM_UNLOCK(unit);

    return rv;
}


/*
 *   Function
 *      bcm_fe2000_oam_group_get
 *   Purpose
 *      Retrieve the group information for the given group id
 *   Parameters
 *       unit        = BCM device number
 *       group       = group to retrieve
 *       group_info  = storage location for found information
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_group_get(int unit, bcm_oam_group_t group, 
                         bcm_oam_group_info_t *group_info)
{
    int rv;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    rv = shr_idxres_list_elem_state(_state[unit]->group_pool, group);

    if (rv != BCM_E_EXISTS) {
        OAM_ERR((_SBX_D(unit, "oam group id %d does not exist!\n"), group));
        rv = BCM_E_NOT_FOUND;
    } else {
        sal_memcpy(group_info, OAM_GROUP_INFO(unit, group), 
                   sizeof(bcm_oam_group_info_t));
        rv = BCM_E_NONE;
    }

    return rv;
}

/*
 *   Function
 *      bcm_fe2000_oam_group_traverse
 *   Purpose
 *      Traverse the set of active groups
 *   Parameters
 *       unit        = BCM device number
 *       cb          = callback invoked for each active group
 *       user_data   = user data passed to each invocation of the callback
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int
bcm_fe2000_oam_group_traverse(int unit, bcm_oam_group_traverse_cb cb, 
                              void *user_data)
{
    int group;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (!BCM_OAM_IS_INIT(unit)) {
            return BCM_E_INIT;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    OAM_LOCK(unit);
    for (group=0; group < _state[unit]->max_endpoints; group++) {
        if (OAM_GROUP_INFO(unit, group)) {
            (cb)(unit, OAM_GROUP_INFO(unit, group), user_data);
        }
    }
    OAM_UNLOCK(unit);

    return BCM_E_NONE;
}

STATIC int
_oam_trunk_cb(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *trunk_info, void *user_data)
{
    int rv = BCM_E_NONE, status = BCM_E_NONE;
    _oam_trunk_data_t *trunk_data=NULL;
    bcm_oam_endpoint_info_t ep_info;
    egr_path_desc_t egrPath;
    uint16 save_ep_id;
    tcal_id_t save_tcal_id;
    uint16 save_maid_cookie;
    dq_p_t ep_elem;

    if ( trunk_info == NULL){
        return BCM_E_PARAM;
    }
    if (OAM_TID_VALID(tid) == FALSE) {
        return BCM_E_PARAM;
    }

    OAM_VERB((_SBX_D(unit, "Updating OAM endpoints after a trunk change\n")));

    /* dont need user_data */
    (void) user_data;

    sal_memset(&egrPath, 0, sizeof(egrPath));

    /* can not allow another API to add/delete or modify endpoints,
     * as that might break the endpoint linked list. */
    OAM_LOCK (unit);

    trunk_data = &(_state[unit]->trunk_data[tid]);

    /* if we dont have information about this trunk, return. */
    if (BCM_SUCCESS(rv) && trunk_data) {

        /* now, traverse the list of endpoints on this LAG.
         * for each endpoint in the list, remove it, and then
         * re-create it using the new port/mod membership info. */

        if(!DQ_EMPTY(&trunk_data->endpoint_list)) {

            /* set the curr pointer to the first item in the list. */
            DQ_TRAVERSE(&trunk_data->endpoint_list, ep_elem) {

                oam_sw_hash_data_t *hash_data = NULL;

                hash_data = DQ_ELEMENT(oam_sw_hash_data_t*, ep_elem, hash_data,
                                       trunk_list_node);
                if (OAM_IS_ETHERNET(hash_data->type)) {
                    /* fill in an endpoint_info structure. */
                    rv = _oam_copy_endpoint(unit, hash_data, &ep_info);
                    if (BCM_SUCCESS(rv)) {
    
                        /* the hash-data pointed to by curr used to be
                         * cleared by endpoint_delete, so it was save off.
                         * turns out there were other reasons why 
                         * endpoint_delete should not have cleared the
                         * hash-data, so it doesnt anymore.  the "save_"
                         * variables are left in the code in case future
                         * changes stomp these values. */
                        save_ep_id = hash_data->rec_idx;
                        save_tcal_id = hash_data->tcal_id;
                        save_maid_cookie = hash_data->maid_cookie;
    
                        /* get the egrPath info before calling endpoint_delete. */
                        if (!(ep_info.flags & BCM_OAM_ENDPOINT_REMOTE)) {
                            rv = _oam_egr_path_get(unit, &egrPath, save_ep_id);
                            if (BCM_FAILURE(rv)) {
                                OAM_ERR((_SBX_D(unit, "Failed to get egrPath for epid %d: %d %s\n"),
                                                 save_ep_id, rv, bcm_errmsg(rv)));
                                status = rv;
                            }
                            ep_info.pkt_pri = egrPath.eteL2.defpricfi >> 1;
                            ep_info.int_pri = egrPath.eteEncap.remark << 5;
                        }
    
                        OAM_VERB((_SBX_D(unit, "Removing endpoint %d\n"), save_ep_id));
    
                        /* delete the existing endpoint's table entries,
                         * using the OLD trunk-info structure! */
                        rv = _oam_enet_endpoint_delete(unit, &ep_info, &(trunk_data->add_info));
                        if (BCM_FAILURE(rv)) {
                            OAM_ERR((_SBX_D(unit, "Failed to delete endpoint %d: %d %s\n"),
                                             save_ep_id, rv, bcm_errmsg(rv)));
                            status = rv;
                        }
    
                        OAM_VERB((_SBX_D(unit, "Re-creating endpoint %d\n"), save_ep_id));
    
                        if (ep_info.flags & BCM_OAM_ENDPOINT_REMOTE) {
                            /* peer entries need to have their local-entry's ep_id
                             * stuffed into the ep_info.local_id field before calling endpoint_set.
                             * the hash entry's maid-cookie is the local entry's id. */
                            ep_info.local_id = save_maid_cookie;
                        } else {
                            /* update egress path with new designate port,
                             * (NEW trunk-info) - local entries only! */
                            rv = _oam_egr_path_update(unit, &egrPath, &ep_info, trunk_info);
                            if (BCM_FAILURE(rv)) {
                                OAM_ERR((_SBX_D(unit, "Failed to update egrPath for epid %d: %d %s\n"),
                                                 save_ep_id, rv, bcm_errmsg(rv)));
                                status = rv;
                            }
                        }
    
                        /* re-create the endpoint's table entries,
                         * (NEW trunk-info). */
                        rv = _oam_enet_endpoint_set(unit, &ep_info, save_ep_id, &egrPath,
                                                    &save_tcal_id, trunk_info);
                        if (BCM_FAILURE(rv)) {
                            OAM_ERR((_SBX_D(unit, "Failed to re-create endpoint %d: %d %s\n"),
                                             save_ep_id, rv, bcm_errmsg(rv)));
                            status = rv;
                        }
                    } else {
                        OAM_ERR((_SBX_D(unit, "Failed to copy endpoint %d: %d %s\n"),
                                         hash_data->rec_idx, rv, bcm_errmsg(rv)));
                        status = rv;
                    }
                } else {
                    /* We are looking at a BFD/Y1731 endpoint 
                     * these endpoints do not exist over the trunk directly.
                     * We have to handle the case when the port identified 
                     * by trunk_index is changed or if the trunk itself 
                     * is deleted. Just recreate the endpoint.
                     */
                     bcm_oam_endpoint_info_t_init(&ep_info);
                     status = bcm_fe2000_oam_endpoint_get(unit, hash_data->rec_idx, &ep_info);
                     if (BCM_FAILURE(status)) {
                         OAM_ERR((_SBX_D(unit, "Failed to get endpoint info %d: %d %s\n"),
                                                hash_data->rec_idx, rv, bcm_errmsg(rv)));
                     } else {
                         ep_info.flags |= (BCM_OAM_ENDPOINT_WITH_ID | BCM_OAM_ENDPOINT_REPLACE);
                         status = bcm_fe2000_oam_endpoint_create(unit, &ep_info);
                     }
                }   

                /* not sure how to handle errors here.
                 * if either the destroy or create fail,
                 * then OAM will be in a bad state, and
                 * not sure how to recover. */
                if (BCM_FAILURE(status))
                    break; 

            } DQ_TRAVERSE_END(&trunk_data->endpoint_list, ep_elem);
        }

        /* update the trunk_data structure with the new add_info. */
        sal_memcpy(&(trunk_data->add_info), trunk_info, sizeof(bcm_trunk_add_info_t));

    } else {
        OAM_ERR((_SBX_D(unit, "Failed to lookup trunk info for tid %d: %d %s\n"),
                         tid, rv, bcm_errmsg(rv)));
        status = BCM_E_PARAM;
    }

    OAM_UNLOCK (unit);

    return status;
}


STATIC int
_oam_thread_start(int unit)
{
    /* create semaphore to signal thread to start processing
     * expired timer events
     */
    if (!oam_control.cb_notify) {
        if ((oam_control.cb_notify =
             sal_sem_create("OAM cb ntfy", sal_sem_BINARY, 0)) == NULL) {
            return BCM_E_MEMORY;
        }

        oam_control.cb_notify_given = FALSE;
    }

    oam_control.thread_running = TRUE;
    oam_control.cb_tid = sal_thread_create("bcmOAM",
                                           SAL_THREAD_STKSZ,
                                           SOC_SBX_CFG_FE2000(unit)->oam_thread_pri,
                                           oam_cb_handler_thread,
                                           NULL);
    if (oam_control.cb_tid == SAL_THREAD_ERROR) {
        OAM_ERR((_SBX_D(unit, "Failed to create oam thread\n")));
        oam_control.thread_running = FALSE;
        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}

STATIC int
_oam_thread_stop(int unit)
{
    int rv = BCM_E_NONE;
    int i, stop_thread = 0;
    soc_timeout_t  wait_stop;

    if (oam_control.thread_running == FALSE) {
        return BCM_E_NONE;
    }

    for (i=0; (i < SOC_MAX_NUM_DEVICES) && (stop_thread == 0); i++) {
        if (BCM_OAM_IS_INIT(i)) {
            stop_thread = 1;
        }
    }

    if (stop_thread) {

        oam_control.thread_running = FALSE;
        OAM_THREAD_NOTIFY(unit);

        soc_timeout_init(&wait_stop, 5 * SECOND_USEC, 0);
        while (oam_control.cb_tid != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&wait_stop)) {
                OAM_ERR((_SBX_D(unit, "thread did not exit\n")));
                oam_control.cb_tid = SAL_THREAD_ERROR;
                rv = SOC_E_INTERNAL;
                break;
            }

            sal_usleep(10000);
        }

        sal_sem_destroy(oam_control.cb_notify);
        oam_control.cb_notify = NULL;
    }

    return rv;
}


STATIC void
oam_cb_handler_thread(void *param)
{
    int unit;
    COMPILER_REFERENCE(param);

    OAM_DEBUG(("expired timer callback thread starting\n"));

    while (oam_control.thread_running) {

        for (unit = 0; unit < SOC_MAX_NUM_DEVICES; unit++) {
            if (!BCM_OAM_IS_INIT(unit)) {
                continue;
            }

            if (!oam_control.thread_running) goto thread_done;

            while (_oam_process_expired_timers(unit)) {
                if (!oam_control.thread_running) goto thread_done;
            }
        }

        sal_sem_take(oam_control.cb_notify, sal_sem_FOREVER);
        oam_control.cb_notify_given = FALSE;
    }

thread_done:

    oam_control.cb_tid = SAL_THREAD_ERROR;
    OAM_DEBUG(("expired timer thread exitting\n"));

    sal_thread_exit(0);
}


STATIC int
_oam_process_expired_timers(int unit)
{
    oam_timer_queue_t *queue = OAM_TIMER_QUEUE(unit);
    bcm_oam_endpoint_info_t epInfo;
    bcm_oam_event_type_t eventType;
    int i, count, flags = 0;

    OAM_VVERB((_SBX_D(unit, "Processing expired timers on unit %d\n"), unit));

    _oam_tq_get_all(queue, &count);
    /* the queue is always serviced from 0-n after a get_all */

    for (i=0; i<count; i++) {
        uint32_t epRecIdx;
        oam_timer_q_elt_t elt;
        int rv;

        _oam_tq_queue_get_elt(queue, i, &elt);
        rv = _oam_get_endpoint_by_timer(unit, elt.timer, &epRecIdx);

        if (BCM_FAILURE(rv) ) {
            OAM_ERR((_SBX_D(unit, "failed to find endpoint with timer id=%d\n"), 
                     elt.timer));
            continue;
        }

        bcm_oam_endpoint_info_t_init(&epInfo);
        epInfo.id = epRecIdx;
        epInfo.flags = BCM_OAM_ENDPOINT_WITH_ID;
        rv = bcm_fe2000_oam_endpoint_get(unit, epRecIdx, &epInfo);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "endpiont get failed: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
            continue;
        }

        if (OAM_IS_BFD(epInfo.type) || OAM_IS_LSP_BFD(epInfo.type)) {
            eventType = bcmOAMEventBFDTimeout;
        } else {
            eventType = bcmOAMEventGroupCCMTimeout;
        }

        if (_state[unit]->wd_cb_data.epCb) {
            (_state[unit]->wd_cb_data.epCb)(unit, flags, eventType, 
                                        epInfo.group, epInfo.id,
                                        _state[unit]->wd_cb_data.userData);
        }

    }

    /* more to do while callbacks were servicing? */
    return !_oam_tq_queue_empty(queue);
}


int
_oam_fe2000_oam_timer_expiration_reset(int unit, int timerid)
{
    soc_sbx_g2p3_state_t                *fe = (soc_sbx_g2p3_state_t *) 
        SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_table_manager_t        *tm = fe->tableinfo;
    soc_sbx_g2p3_policer_desc_t         *pd = &tm->policers[SOC_SBX_G2P3_OAMTIMER_ID];
    soc_sbx_g2p3_policer_bank_desc_t    *pb = &tm->policerbanks[pd->bank];
    sbZfFe2000PmOamTimerRawFormat_t     zTimerRaw;
    uint32_t                            real_timerid;
    sbStatus_t                          sts;

    if (timerid >= pd->size) {
        return BCM_E_PARAM;
    }

    /* get the real id */
    real_timerid = (uint32) (timerid + pb->base + pd->base - tm->policerbase);

    /* read the timer */
    sts = sbFe2000PmOamTimerRawGet(fe->pm, real_timerid, &zTimerRaw);
    if (sts != SB_OK) {
        OAM_ERR((_SBX_D(unit, "Internal error sbFe2000PmOamTimerRawGet failed. "
                        "rv(%d)\n"), sts));
        return soc_sbx_translate_status(sts);
    }

    /* reset the time */
    zTimerRaw.uTime = 0;
    sts = sbFe2000PmOamTimerRawSet(fe->pm, real_timerid, &zTimerRaw);
    if (sts != SB_OK) {
        OAM_ERR((_SBX_D(unit, "Error resetting timer(id:%d). rv (%d) \n"),
                 timerid, sts));
        return soc_sbx_translate_status(sts);
    }

    return BCM_E_NONE;
}

int
_oam_fe2000_oam_timer_expiration_state(int unit, int timerid, 
                                       uint32 *deadline, uint32 *time)
{
    soc_sbx_g2p3_state_t                *fe = (soc_sbx_g2p3_state_t *) 
        SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_table_manager_t        *tm = fe->tableinfo;
    soc_sbx_g2p3_policer_desc_t         *pd = &tm->policers[SOC_SBX_G2P3_OAMTIMER_ID];
    soc_sbx_g2p3_policer_bank_desc_t    *pb = &tm->policerbanks[pd->bank];
    sbZfFe2000PmOamTimerRawFormat_t     zTimerRaw;
    sbZfFe2000PmOamTimerConfig_t        zTimerConfig;
    uint32_t                            real_timerid;
    sbStatus_t                          sts;

    if ((timerid >= pd->size) || (deadline == NULL) || (time == NULL)) {
        return BCM_E_PARAM;
    }

    real_timerid = (uint32) (timerid + pb->base + pd->base - tm->policerbase);

    /* this looks the profile and returns the deadline */
    sts = sbFe2000PmOamTimerRead(fe->pm, real_timerid, &zTimerConfig);
    if (sts != SB_OK) {
        OAM_ERR((_SBX_D(unit, "Internal error, sbFe2000PmOamTimerRead failed. "
                        "rv(%d) id %x\n"), sts, real_timerid));
        return soc_sbx_translate_status(sts);
    }

    /* this looks at the raw timer state to determine that current time */
    sts = sbFe2000PmOamTimerRawGet(fe->pm, real_timerid, &zTimerRaw);
    if (sts != SB_OK) {
        OAM_ERR((_SBX_D(unit, "Internal error sbFe2000PmOamTimerRawGet failed. "
                        "rv(%d)\n"), sts));
        return soc_sbx_translate_status(sts);
    }

    *deadline = zTimerConfig.uDeadline;
    *time = zTimerRaw.uTime;

    return BCM_E_NONE;
}

#define TIMER_EXPIRED_FIFO_OVERFLOW_TIMEOUT 10 
void
_oam_fe2000_wd_timer_expired_fifo_overflow_handle(int unit)
{
    uint32_t fifo_status, timerid, timeout, temp, v, rdi, error;
    soc_sbx_g2p3_policer_params_t   timerParams;
    int                             rv, notify=0;
    oam_timer_queue_t               *queue = OAM_TIMER_QUEUE(unit);
    uint32                          deadline, time;

    OAM_VERB((_SBX_D(unit, " PM_ERROR.TIMER_EXPIRED_FIFO_OVERFLOW condition\n")));

    rv = soc_sbx_g2p3_oamtimer_policer_params_get(unit, &timerParams);
    if (rv != SOC_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Internal error reading oamtimer policer params \n")));
        return;
    }

    fifo_status = SAND_HAL_FE2000_READ(unit, PM_ERROR);
    fifo_status = SAND_HAL_FE2000_GET_FIELD(unit, PM_ERROR,
                                            TIMER_EXPIRED_FIFO_OVERFLOW, fifo_status);
    if (fifo_status) {
        /* mask off the overflow interrupt until the processing is complete */
        v = SAND_HAL_FE2000_READ(unit, PM_ERROR_MASK);
        v |= SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, 
                                        TIMER_EXPIRED_FIFO_OVERFLOW_DISINT);
        SAND_HAL_FE2000_WRITE(unit, PM_ERROR_MASK, v);
    } else {
        OAM_ERR((_SBX_D(unit, "Internal error while handling "
                        "PM_ERROR.TIMER_EXPIRED_FIFO_OVERFLOW condition\n")));
        return;
    }

    timeout = TIMER_EXPIRED_FIFO_OVERFLOW_TIMEOUT;
    error = 0;
    /* loop through timeout times. If overflow condition is still set some 
       internal error happened or expirations are happening faster than SW 
       can handle...bailout */
    while (fifo_status && (timeout > 0) && (error == 0)) {
        /* clear the overflow condition (W1TC) */
        v = SAND_HAL_FE2000_READ(unit, PM_ERROR);
        v |= SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR, 
                                        TIMER_EXPIRED_FIFO_OVERFLOW);
        SAND_HAL_FE2000_WRITE(unit, PM_ERROR, v);

        OAM_VERB((_SBX_D(unit, " Iterating through timers to see what expired\n")));
        /* now process all the timers */
        for (timerid = 1; timerid < timerParams.size; timerid++) {
            /* first check if timer is being used */
            rv = shr_idxres_list_elem_state(_state[unit]->timer_pool, timerid);
            if (rv != BCM_E_EXISTS) {
                continue; /* the timer is not being used */
            }

            rv = _oam_fe2000_oam_timer_expiration_state(unit, timerid, 
                                                        &deadline, &time);
            if (rv != BCM_E_NONE) {
                OAM_ERR((_SBX_D(unit, "Internal error handling PM_ERROR.TIMER_EXPIRE"
                                "D_FIFO_OVERFLOW. rv(%d) - %s  id %x\n"), rv, bcm_errmsg(rv),timerid));
                error = 1;
                break;
            }

            if (time >= deadline) {
                /* timer expired...process it */
                if (_oam_tq_queue_full(queue)) {
                    OAM_WARN((_SBX_D(unit, "Timer queue Full on unit %d, "
                                     "dropping event.\n"),unit));
                    notify = 1;
                } else {
                    if (_state[unit]->wd_cb_data.epCb)
                    {
                        rdi = 0; 
                        OAM_VVERB((_SBX_D(unit, "timer %d expired \n"),timerid));
                        _oam_tq_enqueue(queue, timerid, rdi);
                        notify = 1;
                    } else {
                        OAM_VVERB((_SBX_D(unit, "timer %d expired but no callback "
                                          "registered \n"),timerid));
                    }
                }

#ifdef BCM_FE2000_OAM_TIMER_RESET_ON_EXPIRE
                rv = _oam_fe2000_oam_timer_expiration_reset(unit, timerid);
                if (rv != BCM_E_NONE) {
                    OAM_ERR((_SBX_D(unit, "Error resetting timer(id:%d) handling "
                                    "PM_ERROR.TIMER_EXPIRED_FIFO_OVERFLOW \n"),timerid));
                    error = 1;
                    break;
                }
#endif
            }
        }
        timeout--;
        fifo_status = SAND_HAL_FE2000_READ(unit, PM_ERROR);
        fifo_status = SAND_HAL_FE2000_GET_FIELD(unit, PM_ERROR,
                                                TIMER_EXPIRED_FIFO_OVERFLOW, fifo_status);
    }

    if (timeout <= 0) {
        OAM_ERR((_SBX_D(unit, "Timeout occurred while handling "
                        "PM_ERROR.TIMER_EXPIRED_FIFO_OVERFLOW condition\n")));
    }

    /* now pop off the whole fifo (8 entries). These are already handled
       when each timer's expiration status was looked at individually */
    for (temp=0; ((error == 0) && (temp < 8)); temp++) {
        v = SAND_HAL_FE2000_READ(unit, PM_WATCHDOG_TIMER_EXPIRED_FIFO);
        v = SAND_HAL_FE2000_MOD_FIELD(unit, PM_WATCHDOG_TIMER_EXPIRED_FIFO, 
                                      POP, v, 1);
        SAND_HAL_FE2000_WRITE(unit, PM_WATCHDOG_TIMER_EXPIRED_FIFO, v);
    }

    if (notify) {
        OAM_THREAD_NOTIFY(unit);
    }

    /* unmask the overflow interrupt */
    v = SAND_HAL_FE2000_READ(unit, PM_ERROR_MASK);
    v &= (~SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, 
                                      TIMER_EXPIRED_FIFO_OVERFLOW_DISINT));
    SAND_HAL_FE2000_WRITE(unit, PM_ERROR_MASK, v);
}

/*
 *  Handling the PM_ERROR is as follows:
 *      1. First check if TIMER_EXPIRED_FIFO_OVERFLOW is set. If so, check
 *         the status of each timer individually. And continue as long as the
 *         overflow condition is set or timeout happens. At the end, empty
 *         the fifo (only upto 8 entries)...and discard them. They will have
 *         been handled already. After overflow is handled, process fifo if 
 *         its non-empty.
 *      2. If Overflow condition is not set, but PM_WATCHDOG_TIMER_EXPIRED_FIFO
 *         is set keep popping the fifo until fifo is empty.
 */
STATIC void
_oam_fe2000_wd_timeout_cb(void *unit_)
{
    int unit = PTR_TO_INT(unit_);
    sbhandle sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    uint32_t v, timerid, rdi;
    uint32_t fifo_status;
    oam_timer_queue_t *queue = OAM_TIMER_QUEUE(unit);
    int notify = 0;

    OAM_VVERB((_SBX_D(unit, "Processing WD TIMEOUT callback \n")));

    fifo_status = SAND_HAL_FE2000_READ(sbh, PM_ERROR);
    fifo_status = SAND_HAL_FE2000_GET_FIELD(sbh, PM_ERROR,
                                            TIMER_EXPIRED_FIFO_OVERFLOW, fifo_status);
    if (fifo_status) {
        _oam_fe2000_wd_timer_expired_fifo_overflow_handle(unit);
    }

    /* Pop off each expired timer's ID off of the fifo and push the info
     * timer queue for the callback handler to consume.  Notify the thread to
     * wake and process the events.
     */
    fifo_status = SAND_HAL_FE2000_READ(sbh, PM_ERROR);
    fifo_status = SAND_HAL_FE2000_GET_FIELD(sbh, PM_ERROR,
                                            TIMER_EXPIRED_FIFO_NONEMPTY, fifo_status);
    while(fifo_status) {
        v = SAND_HAL_FE2000_READ(sbh, PM_WATCHDOG_TIMER_EXPIRED_FIFO);
        timerid = SAND_HAL_FE2000_GET_FIELD(sbh, 
                                            PM_WATCHDOG_TIMER_EXPIRED_FIFO, TIMER_ID, v);
        rdi = SAND_HAL_FE2000_GET_FIELD(sbh, PM_WATCHDOG_TIMER_EXPIRED_FIFO,
                                        RDI, v);

        timerid -= _state[unit]->timer_base;

        if (_oam_tq_queue_full(queue)) {
            OAM_WARN((_SBX_D(unit, "Timer queue Full on unit %d, dropping event.\n"),
                      unit));
            notify = 1;
        } else {
            if (_state[unit]->wd_cb_data.epCb)
            {
                OAM_VVERB((_SBX_D(unit, "timer %d expired \n"),timerid));
                _oam_tq_enqueue(queue, timerid, rdi);
                notify = 1;
            } else {
                OAM_VVERB((_SBX_D(unit, "timer %d expired but no callback "
                                  "registered\n"),timerid));
            }
        }
        v = SAND_HAL_FE2000_MOD_FIELD(sbh, PM_WATCHDOG_TIMER_EXPIRED_FIFO, 
                                      POP, v, 1);
        SAND_HAL_FE2000_WRITE(sbh, PM_WATCHDOG_TIMER_EXPIRED_FIFO, v);

        fifo_status = SAND_HAL_FE2000_READ(sbh, PM_ERROR);
        fifo_status = SAND_HAL_FE2000_GET_FIELD(sbh, PM_ERROR,
                                                TIMER_EXPIRED_FIFO_NONEMPTY,
                                                fifo_status);
        OAM_VVERB((_SBX_D(unit, "Timer Expired:  id=%d (0x%08x) fifo_status=%d\n"),
                  timerid, timerid, fifo_status));
    }

    if (notify) {
        OAM_THREAD_NOTIFY(unit);
    }
}

#if defined(BCM_QE2000_SUPPORT) && defined(BCM_FE2000_SUPPORT)
STATIC int
_oam_exceptions_set(int unit, int enable)
{
    int i, rv;
    soc_sbx_g2p3_xt_t zXtable;
    uint32 exc_mismatch_id;
    uint32 exc_learn_id;
    uint32 exc_no_endpoint_id;
    uint32 exc_type_id;
    uint32 exc_rdi_id;
    uint32 exc_threshold_id;
    uint32 exc_copy_id;
    uint32 exc_param_id;
    uint32 exc_ep_invalid_id;
    uint32 oam_exception[9];

    rv = soc_sbx_g2p3_exc_oam_mismatch_get(unit, &exc_mismatch_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g2p3_exc_oam_learn_get(unit, &exc_learn_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g2p3_exc_oam_no_endpoint_get(unit, &exc_no_endpoint_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g2p3_exc_oam_type_get(unit, &exc_type_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    rv = soc_sbx_g2p3_exc_oam_rdi_get(unit, &exc_rdi_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g2p3_exc_oam_threshold_exceeded_get(unit, &exc_threshold_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g2p3_exc_oam_copy_to_host_get(unit, &exc_copy_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g2p3_exc_oam_param_change_get(unit, &exc_param_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g2p3_exc_oam_ep_invalid_idx_get(unit, &exc_ep_invalid_id);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    oam_exception[0] = exc_mismatch_id;
    oam_exception[1] = exc_learn_id;
    oam_exception[2] = exc_no_endpoint_id;
    oam_exception[3] = exc_type_id;
    oam_exception[4] = exc_rdi_id;
    oam_exception[5] = exc_threshold_id;
    oam_exception[6] = exc_copy_id;
    oam_exception[7] = exc_param_id;
    oam_exception[8] = exc_ep_invalid_id;

    for (i=0; i < sizeof(oam_exception)/sizeof(oam_exception[0]); i++) {
        rv = soc_sbx_g2p3_xt_get(unit, oam_exception[i], &zXtable);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* enable them all for now, for testing, but "no_endpoint"
           should eventually be just a drop. */
        zXtable.forward = enable;

        rv = soc_sbx_g2p3_xt_set(unit, oam_exception[i], &zXtable);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
    return rv;
}
#endif /* BCM_QE2000_SUPPORT && BCM_FE2000_SUPPORT */

/*
 *   Function
 *      _oam_port_dest_queue_get
 *   Purpose
 *      Get the ingress sws destination queue for the given port
 *   Parameters
 *       unit        = BCM device number
 *       port        = port to retreiv dq
 *       pq          = storage location for the sws queue number
 *   Returns
 *       BCM_E_*
 */
STATIC int 
_oam_port_dest_queue_get(int unit, uint32 port, uint32 *pq)
{
    sbFe2000Queues_t *queues;    
    uint32_t qid, cIdx;
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        queues   = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;
    } else {
        queues   = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
    }

    qid  = queues->port2iqid[port];
    cIdx = queues->qid2con[qid];
    
    *pq  = queues->toqid[cIdx];
    return 0;
}


STATIC int
_oam_cleanup(int unit)
{
    /* stop the thread before destroying any data structures,
     * if no more oam instances are running 
     */
    _oam_thread_stop(unit);
    soc_sbx_fe2000_unregister_isr(unit, SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG);

    if (_state[unit]->ep_pool) {
        shr_idxres_list_destroy(_state[unit]->ep_pool);
        _state[unit]->ep_pool = 0;
    }

    if (_state[unit]->group_pool) {
        shr_idxres_list_destroy(_state[unit]->group_pool);
        _state[unit]->group_pool = 0;
    }

    if (_state[unit]->timer_pool) {
        shr_idxres_list_destroy(_state[unit]->timer_pool);
        _state[unit]->timer_pool = 0;
    }

    _bcm_tcal_cleanup(unit);
    _bcm_ltm_cleanup(unit);

    if (_state[unit]->mamep_htbl) {
        shr_htb_destroy(&_state[unit]->mamep_htbl, NULL);
    }
    if (_state[unit]->timer_htbl) {
        shr_htb_destroy(&_state[unit]->timer_htbl, NULL);
    }

    if (_state[unit]->hash_data_store) {
        sal_free(_state[unit]->hash_data_store);
    }

    if (_state[unit]->group_info) {
        uint32_t group;

        for (group=0; group < _state[unit]->max_endpoints; group++) {
            if (OAM_GROUP_INFO(unit, group)) {
                sal_free(OAM_GROUP_INFO(unit, group));
                OAM_GROUP_INFO(unit, group) = NULL;
            }
        }

        sal_free(_state[unit]->group_info);
    }


#if defined(BCM_QE2000_SUPPORT) && defined(BCM_FE2000_SUPPORT)
    /* clear the exception table for OAM */
    _oam_exceptions_set(unit, 0);
#endif /* BCM_QE2000_SUPPORT && BCM_FE2000_SUPPORT */

    if(_state[unit]->mutex) {
        sal_mutex_destroy(_state[unit]->mutex);
    }
    _oam_tq_queue_destroy( OAM_TIMER_QUEUE(unit) );

    sal_free(_state[unit]);
    _state[unit] = NULL;

    bcm_fe2000_trunk_change_unregister(unit, _oam_trunk_cb);

    return BCM_E_NONE;
}




void _oam_egr_path_init(int unit, egr_path_desc_t *egrPath)
{
    sal_memset(egrPath, 0, sizeof(egr_path_desc_t));
}

/*
 *  Get the egress path for the given oam endpoint index
 */
int
_oam_egr_path_get(int unit, egr_path_desc_t *egrPath, uint32_t epIdx)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_oamep_t oamep;
    soc_sbx_g2p3_oamep_t oamep2;
    uint32_t dir = OAM_DIR_DOWN;

    egrPath->allocated = 0;

    OAM_DEBUG((_SBX_D(unit, "Getting egress path for oamepi=0x%x\n"), epIdx));
    /* use the given epIdx to find the egress path */
    rv = soc_sbx_g2p3_oamep_get(unit, epIdx, &oamep);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x on replace: %d %s\n"), 
                 epIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* different endpoint types require different processing to get teh ftidx */
    if (oamep.function == _state[unit]->ep_type_ids.ccm_first) {
        /* this is the first local entry, the ftidx is in the second local entry. */
        rv = soc_sbx_g2p3_oamep_get(unit, oamep.nextentry, &oamep2);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get (second) oamep 0x%x on replace: %d %s\n"), 
                     oamep.nextentry, rv, bcm_errmsg(rv)));
            return rv;
        }    
        egrPath->ftIdx = oamep2.ftidx;
        dir = oamep.dir;

    } else if (oamep.function == _state[unit]->ep_type_ids.bfd) {
        egrPath->ftIdx = oamep.ftidx;
        dir = OAM_DIR_DOWN;  /* only down mode supported for BFD */

    } else if (oamep.function == _state[unit]->ep_type_ids.dm) {
        egrPath->ftIdx = oamep.ftidx;
        dir = oamep.dir;

    } else if (oamep.function == _state[unit]->ep_type_ids.lm) {
        /* LM entries have a frame-count in the common ftidx location...
         * retrieve ftidx from the ftidx_store field.
         */
        egrPath->ftIdx = oamep.ftidx_store;
        dir = oamep.dir;

    } else if (oamep.function == _state[unit]->ep_type_ids.invalid) {
        /* MPLS endpoint */
        egrPath->ftIdx = oamep.ftidx;        
    } else {
      /* CRA encap -> psc */
        /* types: Encap, Second-CCM and Peer-CCM are not valid for egr_path_get (they do
           not contain an ftidx). */
        OAM_ERR((_SBX_D(unit, "_oam_egr_path_get called with an invalid epidx 0x%x \n"), 
                 epIdx));
        return BCM_E_PARAM;
    }

    /* FTE */
    rv = soc_sbx_g2p3_ft_get(unit, egrPath->ftIdx, &egrPath->fte);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get fte 0x%x on replace: %d %s\n"),
                 egrPath->ftIdx, rv, bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(FT, egrPath->allocated);


    /* down MEP CCM Endpoints do not have the OI -> ETE egress path
     * configured because they are an mcast forward and get flooded
     * on the vlan.  up MEP PM are unicast and require the ETE encap.
     */
    if ((dir == OAM_DIR_DOWN) ||
        (oamep.function != _state[unit]->ep_type_ids.ccm_first)) {
        ALLOC_SET(ESMAC, egrPath->allocated);

        /* OHI */
        egrPath->ohIdx = egrPath->fte.oi;
        rv = soc_sbx_g2p3_oi2e_get(unit, egrPath->ohIdx - SBX_RAW_OHI_BASE,
                                   &egrPath->oh);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oi 0x%x on replace: %d %s\n"),
                     egrPath->ohIdx, rv, bcm_errmsg(rv)));
            return rv;
        }
        ALLOC_SET(OH, egrPath->allocated);

        /* ETE Encap */
        egrPath->eteEncapIdx = egrPath->oh.eteptr;
        rv = soc_sbx_g2p3_eteencap_get(unit, egrPath->eteEncapIdx, &egrPath->eteEncap);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get eteencap 0x%x on replace: %d %s\n"),
                     egrPath->eteEncapIdx, rv, bcm_errmsg(rv)));
            return rv;
        }
        ALLOC_SET(ETEENCAP, egrPath->allocated);

        /* ETE L2 */
        egrPath->eteL2Idx = egrPath->eteEncap.l2ete;
        rv = soc_sbx_g2p3_etel2_get(unit, egrPath->eteL2Idx, &egrPath->eteL2);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get etel2 0x%x on replace: %d %s\n"),
                     egrPath->eteL2Idx, rv, bcm_errmsg(rv)));
            return rv;
        }
        ALLOC_SET(ETEL2, egrPath->allocated);
    }

/* all three must be set to see this dump */
#ifdef BROADCOM_DEBUG
    if (BCM_DEBUG_CHECK (BCM_DBG_OAM | BCM_DBG_VVERBOSE | BCM_DBG_VERBOSE)) {
        OAM_OUT((_SBX_D(unit, "Retrieved egress path :\n")));
        OAM_OUT(("fte Idx=0x%x\n", egrPath->ftIdx));
        soc_sbx_g2p3_ft_print(unit, &egrPath->fte);
        OAM_OUT(("oh Idx=0x%x\n", egrPath->ohIdx));
        soc_sbx_g2p3_oi2e_print(unit, &egrPath->oh);
        OAM_OUT(("ete encap Idx=0x%x\n", egrPath->eteEncapIdx));
        soc_sbx_g2p3_eteencap_print(unit, &egrPath->eteEncap);
        OAM_OUT(("ete l2 Idx=0x%x\n", egrPath->eteL2Idx));
        soc_sbx_g2p3_etel2_print(unit, &egrPath->eteL2);
    }
#endif
    return rv;
}

/*
 *  Allocate & initialize FTE,OHI,ETE L2, & ETE Encap for DOWN meps only..
 *
 *   Does NOT commit to hardware
 */
int _oam_egr_path_alloc(int unit, bcm_mac_t smac, egr_path_desc_t *egrPath)
{
    int rv = BCM_E_NONE;
    uint32 smac_idx;

    /* allocate and initalize the egress path */
    rv = _sbx_gu2_esmac_idx_alloc(unit, 0, smac, &smac_idx);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "failed to allocate egress smac " L2_6B_MAC_FMT 
                        "  %s\n"),
                 L2_6B_MAC_PFMT(smac), bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(ESMAC, egrPath->allocated);

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_FTE_UNICAST, 1,
                                 &egrPath->ftIdx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to alloc FTE: %d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(FT, egrPath->allocated);

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_OHI, 1,
                                 &egrPath->ohIdx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate ohi: %d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(OH, egrPath->allocated);

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1,
                                 &egrPath->eteEncapIdx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate ete encap: %d %s\n"),
                 rv, bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(ETEENCAP, egrPath->allocated);

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_L2, 1,
                                 &egrPath->eteL2Idx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate ete l2: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
        return rv;
    }
    ALLOC_SET(ETEL2, egrPath->allocated);

    OAM_DEBUG((_SBX_D(unit, "FTE=0x%08X OI=0x%08X ETEENCAP=0x%08X ETEL2=0x%08X\n"),
               egrPath->ftIdx, egrPath->ohIdx, egrPath->eteEncapIdx, 
               egrPath->eteL2Idx));

    soc_sbx_g2p3_ft_t_init(&egrPath->fte);
    soc_sbx_g2p3_etel2_t_init(&egrPath->eteL2);
    soc_sbx_g2p3_eteencap_t_init(&egrPath->eteEncap);
    soc_sbx_g2p3_oi2e_t_init(&egrPath->oh);

    /****************** config ETEs *********************/
    egrPath->eteEncap.l2ete = egrPath->eteL2Idx;
    egrPath->eteEncap.ipttldec = FALSE;
    egrPath->eteEncap.ttlcheck = FALSE;
    egrPath->eteEncap.nosplitcheck = TRUE;
    egrPath->eteEncap.dmacset = TRUE;
    egrPath->eteEncap.dmacsetlsb = FALSE;
    egrPath->eteEncap.dmac0 = 0x01;
    egrPath->eteEncap.dmac1 = 0x80;
    egrPath->eteEncap.dmac2 = 0xC2;
    egrPath->eteEncap.dmac3 = 0x00;
    egrPath->eteEncap.dmac4 = 0x00;
    egrPath->eteEncap.smacset = TRUE;

    egrPath->eteL2.smacindex = smac_idx;
    egrPath->eteL2.mtu = SBX_DEFAULT_MTU_SIZE;
    egrPath->eteL2.nostrip = TRUE;
    egrPath->eteL2.nosplitcheck = TRUE;

    /******************** config OIX *********************/
    egrPath->oh.eteptr = egrPath->eteEncapIdx;

    /******************** configure FTE *********************/

    egrPath->fte.oi = egrPath->ohIdx;
    egrPath->fte.excidx = 0;

    return rv;
}

int
_oam_egr_path_clone_downmep(int unit, egr_path_desc_t *clone, uint32_t rec_idx)
{
    int                  rv;
    egr_path_desc_t      src;
    bcm_mac_t            smac;
    soc_sbx_g2p3_esmac_t esmac;

    rv = _oam_egr_path_get(unit, &src, rec_idx);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get src copy of egr path: %s\n"),
                 bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_esmac_get(unit, src.eteL2.smacindex, &esmac);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read esmac[0x%x]: %s\n"),
                 src.eteL2.smacindex, bcm_errmsg(rv)));
        return rv;
    }

    smac[0] = esmac.smac0;
    smac[1] = esmac.smac1;
    smac[2] = esmac.smac2;
    smac[3] = esmac.smac3;
    smac[4] = esmac.smac4;
    smac[5] = esmac.smac5;

    /* allocate the path to increment reference to smac entry */
    rv = _oam_egr_path_alloc(unit, smac, clone);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to alloc egrPath: %s\n"),
                 bcm_errmsg(rv)));
        return rv;
    }

    /* copy src to clone */
    sal_memcpy(&clone->fte,      &src.fte,      sizeof(clone->fte));
    sal_memcpy(&clone->eteL2,    &src.eteL2,    sizeof(clone->eteL2));
    sal_memcpy(&clone->eteEncap, &src.eteEncap, sizeof(clone->eteEncap));
    
    /* reset clone pointers */
    clone->eteEncap.l2ete = clone->eteL2Idx;
    clone->fte.oi         = clone->ohIdx;

    return rv;
}

int
_oam_egr_path_clone_upmep(int unit, egr_path_desc_t *clone, bcm_gport_t gport, int vid, bcm_mac_t smac)
{
    int                  rv;
    bcm_module_t         mod;
    bcm_port_t           port;
    int fabUnit, fabNode, fabPort;

    /* for up MEP, the egr path is significantly different between
     * the CCM and LM/DM endpoints.  for up MEP CCM, the egr-path
     * is a broadcast using the pass-through ETE on egress.  for
     * LM & DM we need a unicast path with an ETE that will over-
     * write the encoded DMAC & SMAC.
     */

    /* for UP MEPs, the smac table entry might not exist, so create
     * it now.  all we can do is copy the smac, have to wait until
     * after egr_path_alloc to write the smac into memory (needs the
     * smac-index generated by _alloc.
     */
    OAM_DEBUG((_SBX_D(unit, "UP MEP cloned smac:  %02x:%02x:%02x:%02x:%02x:%02x \n"), 
               smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]));

    /* allocate the path to increment reference to smac entry */
    rv = _oam_egr_path_alloc(unit, smac, clone);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to alloc egrPath: %s\n"),
                 bcm_errmsg(rv)));
        return rv;
    }

    /* write the smac into the egress smac table. */
    _oam_egr_path_smac_set(unit, clone, smac);

    clone->eteL2.usevid = TRUE;

    /* BCM_VLAN_VALID is:  0 < vid < 0x1000
     * G2P3 reserves 0xFFF as an ETE flag specifying "untagged-vid".
     */
    if ((BCM_VLAN_VALID(vid)) && (vid != _BCM_VLAN_G2P3_UNTAGGED_VID)) {
        clone->eteL2.vid = vid;
        OAM_DEBUG((_SBX_D(unit, "Tagged local endpoint using vid %d (0x%04x)\n"), 
                   clone->eteL2.vid, clone->eteL2.vid));
    } else {
        clone->eteL2.vid = _BCM_VLAN_G2P3_UNTAGGED_VID;
        OAM_DEBUG((_SBX_D(unit, "Untagged local endpoint\n")));
    }

    /* this gport is used to construct the FTE.
     * the gport should be the destination port
     * where the peer endpoint resides.
     */
    if (BCM_GPORT_IS_LOCAL(gport)) {
        rv = bcm_stk_my_modid_get(unit, &mod);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get modid: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
            return rv;
        }
        port = BCM_GPORT_LOCAL_GET(gport);

    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        mod = BCM_GPORT_MODPORT_MODID_GET(gport);
        port = BCM_GPORT_MODPORT_PORT_GET(gport);

    } else {
        OAM_ERR((_SBX_D(unit, "Unsupported GPORT 0x%08x\n"), gport));
        return BCM_E_PARAM;
    }

    if ((port < 0 || port >= SBX_MAX_PORTS) || !SOC_MODID_ADDRESSABLE(unit, mod)) {
        OAM_ERR((_SBX_D(unit, "invalid gport passed: 0x%08x %s %s\n"),
                 gport, SOC_PORT_VALID(unit, port) ? " " : "invalid port",
                 SOC_MODID_ADDRESSABLE(unit, mod) ? " " : "invalid module id"));
        return BCM_E_PARAM;
    }

    /* get the fab node/port from the dest node/port */
    rv = soc_sbx_node_port_get(unit, mod, port,
                               &fabUnit, &fabNode, &fabPort);

    if (BCM_FAILURE(rv)) {
        /* failed to get target information */
        OAM_ERR((_SBX_D(unit, "soc_sbx_node_port_get(%d,%d,%d,&(%d),&(%d),&(%d)) "
                        "returned %d (%s)\n"),
                 unit, mod, port, fabUnit, fabNode, fabPort,
                 rv, bcm_errmsg(rv)));
        return rv;
    } else {
        /* check destination node */
        if (!SOC_SBX_NODE_ADDRESSABLE(unit, fabNode)) {
            /* inaccessible destination node */
            OAM_ERR((_SBX_D(unit, "target node %d inaccessible\n"),fabNode));
            return rv;;
        }
    }

    /* get the qid from the fab node/port */
    clone->fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,fabNode, fabPort, 
                                                NUM_COS(unit));
    
    return rv;
}

int
_oam_egr_path_commit(int unit, egr_path_desc_t *egrPath)
{
    int rv;

    /* write the L2 ETE first */
    rv = soc_sbx_g2p3_etel2_set(unit, egrPath->eteL2Idx, &egrPath->eteL2);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write ete[%08X]L2: %d (%s)\n"),
                 egrPath->eteL2Idx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* write the encap ETE */
    rv = soc_sbx_g2p3_eteencap_set(unit, egrPath->eteEncapIdx, 
                                   &egrPath->eteEncap);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write ete[%08X]Encap: %d (%s)\n"),
                 egrPath->eteEncapIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_oi2e_set(unit, egrPath->ohIdx - SBX_RAW_OHI_BASE,
                               &egrPath->oh);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write oi2e[%08X]: %d (%s)\n"),
                 egrPath->ohIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_ft_set(unit, egrPath->ftIdx, &egrPath->fte);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write ft[%08X]: %d (%s)\n"),
                 egrPath->ftIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

/* all three must be set to see this dump */
#ifdef BROADCOM_DEBUG
    if (BCM_DEBUG_CHECK (BCM_DBG_OAM | BCM_DBG_VVERBOSE | BCM_DBG_VERBOSE)) {
        OAM_OUT((_SBX_D(unit, "Updated egress path :\n")));
        OAM_OUT(("fte Idx=0x%x\n", egrPath->ftIdx));
        soc_sbx_g2p3_ft_print(unit, &egrPath->fte);
        OAM_OUT(("oh Idx=0x%x\n", egrPath->ohIdx));
        soc_sbx_g2p3_oi2e_print(unit, &egrPath->oh);
        OAM_OUT(("ete encap Idx=0x%x\n", egrPath->eteEncapIdx));
        soc_sbx_g2p3_eteencap_print(unit, &egrPath->eteEncap);
        OAM_OUT(("ete l2 Idx=0x%x\n", egrPath->eteL2Idx));
        soc_sbx_g2p3_etel2_print(unit, &egrPath->eteL2);
    }
#endif
    return rv;
}

/*
 * Update the egress path based on the endpoint, and commit to hardware
 */
int _oam_egr_path_update(int unit,
                     egr_path_desc_t *egrPath, 
                     bcm_oam_endpoint_info_t *ep_info,
                     bcm_trunk_add_info_t *trunk_info)
{
    int          rv;
    bcm_module_t mod;
    bcm_port_t   port;
    int fabUnit, fabNode, fabPort;
    int dir;
    bcm_gport_t gport;
    int index;

    if (!egrPath || !ep_info || !trunk_info) {
        OAM_ERR((_SBX_D(unit, "NULL pointer\n")));
        return BCM_E_PARAM;
    }

    dir = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);

    if (dir == OAM_DIR_UP) {
        rv = soc_sbx_g2p3_vlan_ft_base_get(unit, &egrPath->ftIdx);
        egrPath->ftIdx += SBX_VSI_FROM_VID(ep_info->vlan);
        OAM_DEBUG((_SBX_D(unit, "mapped Vlan 0x%08X to FTE 0x%08X \n"),
                   ep_info->vlan, egrPath->ftIdx));
        return rv;
    }

    egrPath->eteL2.usevid = TRUE;

    /* when p2e.customer = 1, (customer port) this pkt_pri field will
     * be copied into the vlan tag.pricfi.  cfi is always zero.
     */
    egrPath->eteL2.defpricfi = (ep_info->pkt_pri << 1) + 0;

    /* when p2e.customer = 0, (provider port) the remark table, rcos, and rdp
     * are all specified in the int_pri field.
     * 7 bits table, 3 bits rcos, 2 bits rdp
     * set the table number in the encap ETE, and the rcos/rdp into the oamEp entry.
     */
    if(ep_info->flags & BCM_OAM_ENDPOINT_USE_QOS_MAP) {
        egrPath->eteEncap.remark = ep_info->egr_map;
    } else {
        egrPath->eteEncap.remark = ((int)ep_info->int_pri >> 5) & 0x7f;
    }

    /* BCM_VLAN_VALID is:  0 < vid < 0x1000
     * G2P3 reserves 0xFFF as an ETE flag specifying "untagged-vid".
     */
    if ((BCM_VLAN_VALID(ep_info->vlan)) && (ep_info->vlan != _BCM_VLAN_G2P3_UNTAGGED_VID)) {
        egrPath->eteL2.vid = ep_info->vlan;
        OAM_DEBUG((_SBX_D(unit, "Tagged local endpoint using vid %d (0x%04x)\n"), 
                   egrPath->eteL2.vid, egrPath->eteL2.vid));
    } else {
        egrPath->eteL2.vid = _BCM_VLAN_G2P3_UNTAGGED_VID;
        OAM_DEBUG((_SBX_D(unit, "Untagged local endpoint\n")));
    }

    gport = ep_info->gport;

    /* for BFD, use physical gport under mpls gport
     * In case of LSP, use the modport of the tunnel egress interface
    */
    if (BCM_GPORT_IS_MPLS_PORT(ep_info->gport) || 
        OAM_IS_MPLS_Y1731(ep_info->type) ||
        OAM_IS_LSP_BFD(ep_info->type) ||
        OAM_IS_PSC(ep_info->type)) {
        gport = egrPath->port;
    }

    /* BFD over LSP over trunk notes:
     *    None of the code under BCM_GPORT_IS_TRUNK is applicable in this case.
     *    See notes under bcm_fe2000_oam_endpoint_create
     *    At this point, the endpoint->gport actually refers to the MODPORT 
     *    of the tunnel egress port
     */

    /* get dest node/port from the gport - which could be a trunk */
    if (BCM_GPORT_IS_TRUNK(gport)) {

        /* select the "designate" port/mod */
        index = ep_info->trunk_index;
        if ((index > trunk_info->num_ports) || (index < 0)) {
            index = 0;
        }
        mod  = trunk_info->tm[index];
        port = trunk_info->tp[index];

    } else if (BCM_GPORT_IS_LOCAL(gport)) {
        rv = bcm_stk_my_modid_get(unit, &mod);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get modid: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
            return rv;
        }
        port = BCM_GPORT_LOCAL_GET(gport);

    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        mod = BCM_GPORT_MODPORT_MODID_GET(gport);
        port = BCM_GPORT_MODPORT_PORT_GET(gport);

    } else {
        OAM_ERR((_SBX_D(unit, "Unsupported GPORT 0x%08x\n"), gport));
        return BCM_E_PARAM;
    }

    if ((port < 0 || port >= SBX_MAX_PORTS) || !SOC_MODID_ADDRESSABLE(unit, mod)) {
        OAM_ERR((_SBX_D(unit, "invalid gport passed: 0x%08x %s %s\n"),
                 gport, SOC_PORT_VALID(unit, port) ? " " : "invalid port",
                 SOC_MODID_ADDRESSABLE(unit, mod) ? " " : "invalid module id"));
        return BCM_E_PARAM;
    }

    /* get the fab node/port from the dest node/port */
    rv = soc_sbx_node_port_get(unit, mod, port,
                               &fabUnit, &fabNode, &fabPort);
    
    if (BCM_FAILURE(rv)) {
        /* failed to get target information */
        OAM_ERR((_SBX_D(unit, "soc_sbx_node_port_get(%d,%d,%d,&(%d),&(%d),&(%d)) "
                        "returned %d (%s)\n"),
                 unit, mod, port, fabUnit, fabNode, fabPort,
                 rv, bcm_errmsg(rv)));
        return rv;
    } else {
        /* check destination node */
        if (!SOC_SBX_NODE_ADDRESSABLE(unit, fabNode)) {
            /* inaccessible destination node */
            OAM_ERR((_SBX_D(unit, "target node %d inaccessible\n"),fabNode));
            return rv;;
        }
    }

    /* get the qid from the fab node/port */
    egrPath->fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,fabNode, fabPort, 
                                                NUM_COS(unit));

    return _oam_egr_path_commit(unit, egrPath);
}


int _oam_egr_path_free(int unit, egr_path_desc_t *egrPath)
{
    int rv = BCM_E_NONE;
 
    OAM_DEBUG((_SBX_D(unit, "Freeing resources: fti=0x%08x oi=0x%08x "
                      "etencapi=0x%08x etel2i=0x%08x\n"),
               egrPath->ftIdx, egrPath->ohIdx, egrPath->eteEncapIdx, 
               egrPath->eteL2Idx));

    if (ALLOC_GET(ESMAC, egrPath->allocated)) {
        bcm_mac_t smac;

        rv = _sbx_gu2_esmac_idx_free(unit, smac, &egrPath->eteL2.smacindex);

        if (rv == BCM_E_EMPTY) {
            soc_sbx_g2p3_esmac_t esm;
            soc_sbx_g2p3_esmac_t_init (&esm);

            OAM_VERB((_SBX_D(unit, "Found no more users of idx %d, clearing hw\n"),
                      egrPath->eteL2.smacindex));
            rv = soc_sbx_g2p3_esmac_set (unit, egrPath->eteL2.smacindex, &esm);
            
            if (BCM_FAILURE(rv)) {
                OAM_WARN((_SBX_D(unit, "Failed to delete egress smac: %d %s\n"),
                          rv, bcm_errmsg(rv)));
            }
        }
        
        OAM_DEBUG((_SBX_D(unit, "Freed smac idx %d - " L2_6B_MAC_FMT " -> %s\n"), 
                   egrPath->eteL2.smacindex, L2_6B_MAC_PFMT(smac),
                   bcm_errmsg(rv)));

    }

    soc_sbx_g2p3_ft_t_init(&egrPath->fte);
    soc_sbx_g2p3_etel2_t_init(&egrPath->eteL2);
    soc_sbx_g2p3_eteencap_t_init(&egrPath->eteEncap);
    soc_sbx_g2p3_oi2e_t_init(&egrPath->oh);

    /* none of these errors are fatal, they are here for diagnostics only */

    if (ALLOC_GET(ETEL2, egrPath->allocated)) {
        rv = soc_sbx_g2p3_etel2_set(unit, egrPath->eteL2Idx, &egrPath->eteL2);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to clear ete[%08X]L2: %d (%s)\n"),
                     egrPath->eteL2Idx, rv, bcm_errmsg(rv)));
        }

        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2, 1,
                                    &egrPath->eteL2Idx, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to free ete l2: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
        }
    }

    if (ALLOC_GET(ETEENCAP, egrPath->allocated)) {
        rv = soc_sbx_g2p3_eteencap_set(unit, egrPath->eteEncapIdx, 
                                       &egrPath->eteEncap);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to clear ete[%08X]Encap: %d (%s)\n"),
                     egrPath->eteEncapIdx, rv, bcm_errmsg(rv)));
        }
        
        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1,
                                    &egrPath->eteEncapIdx, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to free ete encap: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
        }
    }

    if (ALLOC_GET(OH, egrPath->allocated)) {
        rv = soc_sbx_g2p3_oi2e_set(unit, egrPath->ohIdx - SBX_RAW_OHI_BASE,
                                   &egrPath->oh);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to clear oi2e[%08X]: %d (%s)\n"),
                     egrPath->ohIdx, rv, bcm_errmsg(rv)));
        }

        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_OHI, 1,
                                    &egrPath->ohIdx, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to free ohi: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
        }
    }

    if (ALLOC_GET(FT, egrPath->allocated)) {
        rv = soc_sbx_g2p3_ft_set(unit, egrPath->ftIdx, &egrPath->fte);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to clear ft[%08X]: %d (%s)\n"),
                     egrPath->ftIdx, rv, bcm_errmsg(rv)));
        }

        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_FTE_UNICAST, 1,
                                    &egrPath->ftIdx, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to free FTE: %d %s\n"), 
                     rv, bcm_errmsg(rv)));
        }
    }
    return rv;
}

int _oam_egr_path_dmac_set(int unit, egr_path_desc_t *egrPath, bcm_mac_t dmac, int dmaclsbset)
{
    int rv = BCM_E_NONE;

    egrPath->eteEncap.dmacset = TRUE;
    egrPath->eteEncap.dmacsetlsb = dmaclsbset;
    egrPath->eteEncap.dmac0 = dmac[0];
    egrPath->eteEncap.dmac1 = dmac[1];
    egrPath->eteEncap.dmac2 = dmac[2];
    egrPath->eteEncap.dmac3 = dmac[3];
    egrPath->eteEncap.dmac4 = dmac[4];
    egrPath->eteEncap.dmac5 = dmac[5];
    
    egrPath->eteEncap.smacset = TRUE;

    rv = soc_sbx_g2p3_eteencap_set(unit, egrPath->eteEncapIdx,
                                   &egrPath->eteEncap);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write eteEncap[0x%04x]: %d %s\n"),
                 egrPath->eteEncapIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}

int _oam_egr_path_smac_set(int unit, egr_path_desc_t *egrPath, bcm_mac_t smac)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_esmac_t esmac;
    uint32_t             esmacIdx = egrPath->eteL2.smacindex;

    esmac.smac0 = smac[0];
    esmac.smac1 = smac[1];
    esmac.smac2 = smac[2];
    esmac.smac3 = smac[3];
    esmac.smac4 = smac[4];
    esmac.smac5 = smac[5];

    rv = soc_sbx_g2p3_esmac_set(unit, esmacIdx, &esmac);

    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write esmac idx 0x%x \n"), esmacIdx));
        return rv;
    }

    return rv;
}

int _oam_copy_endpoint(int unit, oam_sw_hash_data_t *hash_data,
                       bcm_oam_endpoint_info_t *ep_info)
{
    int rv = BCM_E_NONE;

    if (!hash_data || !ep_info) {
        return BCM_E_PARAM;
    }

    ep_info->id         = hash_data->rec_idx;
    ep_info->flags      = hash_data->flags;
    ep_info->type       = hash_data->type;
    ep_info->group      = hash_data->group;
    ep_info->gport      = hash_data->gport;
    ep_info->vlan       = hash_data->vid_label;
    ep_info->name       = hash_data->ep_name;
    ep_info->mpls_label = hash_data->vid_label;
    ep_info->local_id   = hash_data->maid_cookie;
    ep_info->level      = hash_data->mdlevel;
    ep_info->ccm_period = hash_data->interval;
    ep_info->trunk_index = hash_data->trunk_index;

    sal_memcpy( ep_info->src_mac_address,
                hash_data->mac_address,
                sizeof(bcm_mac_t));

    return rv;
}

/* break this function in two at some point?  _oam_endpoint_idx_find and
 *  _oam_idx_endpoint_find.  they are doing two different things.  the
 *  second half that looks up the entry using a hash key does not even
 *  need the endpoint_info struct filled out. */

STATIC int
_oam_endpoint_idx_find(int unit, bcm_oam_endpoint_info_t *ep_info, 
                       uint32_t *idx)
{
    int rv = BCM_E_NONE;
    oam_sw_hash_key_t hash_key;
    oam_sw_hash_data_t *hash_data = NULL;
    egr_path_desc_t egrPath;
    soc_sbx_g2p3_oamep_t oamep;

    if (ep_info->flags & BCM_OAM_ENDPOINT_WITH_ID) {
        if (ep_info->id > _state[unit]->max_endpoints) {
            return BCM_E_PARAM;
        }

        *idx = ep_info->id;

        /* assume ONLY the flags are valid for get-by-id */
        ep_info->flags |= _state[unit]->hash_data_store[ep_info->id].flags;
        ep_info->type   = _state[unit]->hash_data_store[ep_info->id].type;
        ep_info->group  = _state[unit]->hash_data_store[ep_info->id].group;
        ep_info->gport  = _state[unit]->hash_data_store[ep_info->id].gport;
        ep_info->name   = _state[unit]->hash_data_store[ep_info->id].ep_name;
        ep_info->local_id   = _state[unit]->hash_data_store[ep_info->id].maid_cookie;
        ep_info->level      = _state[unit]->hash_data_store[ep_info->id].mdlevel;
        ep_info->ccm_period = _state[unit]->hash_data_store[ep_info->id].interval;

        if (OAM_IS_ETHERNET(ep_info->type)) {
            ep_info->vlan =
                _state[unit]->hash_data_store[ep_info->id].vid_label;
        } else {
            ep_info->mpls_label =
                _state[unit]->hash_data_store[ep_info->id].vid_label;
        }

        sal_memcpy( ep_info->src_mac_address,
                    _state[unit]->hash_data_store[ep_info->id].mac_address,
                    sizeof(bcm_mac_t));

    } else {
        _init_ep_hash_key(unit, hash_key, ep_info);

        *idx = 0;

        rv = shr_htb_find(_state[unit]->mamep_htbl, hash_key,
                          (shr_htb_data_t *)&hash_data,
                          0 /* don't remove */);

        if (BCM_FAILURE(rv)) {
            hash_data = NULL;
            rv = BCM_E_INTERNAL;
            OAM_ERR((_SBX_D(unit, "found null data at valid sw hash entry\n")));
        } 

        if (hash_data) {
            *idx                = hash_data->rec_idx;
            ep_info->flags      = hash_data->flags;
            ep_info->type       = hash_data->type;
            ep_info->group      = hash_data->group;
            ep_info->vlan       = hash_data->vid_label;
            ep_info->name       = hash_data->ep_name;
            ep_info->mpls_label = hash_data->vid_label;
            ep_info->local_id   = hash_data->maid_cookie;
            ep_info->level      = hash_data->mdlevel;
            ep_info->ccm_period = hash_data->interval;

            sal_memcpy( ep_info->src_mac_address,
                        hash_data->mac_address,
                        sizeof(bcm_mac_t));
            
            /* verify key data passed is what is stored */
            if (ep_info->gport != hash_data->gport) {
                rv = BCM_E_PARAM;
                OAM_ERR((_SBX_D(unit, "invalid key data mismatch: "
                                "gport=(0x%08x/0x%08x)\n"),
                         ep_info->gport, hash_data->gport));
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* do not get egrPath for up or remote endpoints */
    if (((ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING) == 0) &&
        ((ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) == 0)) {
        rv = soc_sbx_g2p3_oamep_get(unit, ep_info->id, &oamep);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        rv = _oam_egr_path_get(unit, &egrPath, ep_info->id);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    
        ep_info->pkt_pri = egrPath.eteL2.defpricfi >> 1;
        ep_info->int_pri = egrPath.eteEncap.remark << 5 | oamep.intpri;
    }
    
    return rv;
}


STATIC int
_oam_get_endpoint_by_timer(int unit, uint32_t timer, uint32_t *ep_rec_idx)
{
    int rv;
    rv = shr_htb_find(_state[unit]->timer_htbl, &timer,
                      (shr_htb_data_t *)ep_rec_idx,
                      0 /* == don't remove */);

    return rv;
}


int _oam_timer_allocate(int unit, int ep_rec_index, 
                        double interval_ms, double window_multiplier,
                        uint32 *id)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_timer_t cfg;

    rv = shr_idxres_list_alloc(_state[unit]->timer_pool, id);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Out of timer resources\n")));
        return rv;
    }

    interval_ms = interval_ms * window_multiplier;

    /* The deadline for timers defines the nubmer of 10ms-ticks before the
     * timer expires.  The rate is somewhat irrelevant for timers, but must
     * be set to SB_FE_2000_PM_PRATE_100KBPS for the ilib.
     */
    cfg.ms         = interval_ms / 10;
    if (cfg.ms < 2) {cfg.ms = 2;} /* due to the granularity of this timer,
                                   * 2 (20 ms) is the smallest number we can set. */

    cfg.started    = 1; /* start before first ucode access */
    cfg.ucodereset = 1; /* ucode resets timer */
    cfg.interrupt  = 1; /* generate interrupt when timer expires */

    rv = soc_sbx_g2p3_oamtimer_set(unit, *id, &cfg);
    if (rv != BCM_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Failed to create OAM timer: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = shr_htb_insert(_state[unit]->timer_htbl, id,
                        (shr_htb_data_t)ep_rec_index);
    if (rv != BCM_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Failed to insert timer_id=%d and ep_id=%d: %d %s\n"),
                 *id, ep_rec_index, rv, bcm_errmsg(rv)));
    }

    OAM_VERB((_SBX_D(unit, "allocated timer: id=%d (0x%x) rate=%d ep=%d\n"),
              *id, *id, (int)interval_ms, ep_rec_index));

    return rv;
}

int _oam_timer_free(int unit, uint32 *id)
{
    int rv = BCM_E_NONE;
    int idx;

    shr_idxres_list_free(_state[unit]->timer_pool, *id);

    OAM_VERB((_SBX_D(unit, "freed timer: id=%d\n"), *id));

    rv = soc_sbx_g2p3_oamtimer_delete(unit, *id);
    if (rv != BCM_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Failed to delete OAM timer id=%d: %d %s\n"),
                 *id, rv, bcm_errmsg(rv)));
    }

    rv = shr_htb_find(_state[unit]->timer_htbl, id,
                      (shr_htb_data_t *)&idx,
                      1 /* == remove */);

    *id = 0;
    return rv;
}

STATIC void
_init_ep_hash_key(int unit, oam_sw_hash_key_t key, 
                  bcm_oam_endpoint_info_t *ep_info)
{
    uint8* loc = key;
    uint32 direction = 0;
    
    sal_memset(key, 0, sizeof(oam_sw_hash_key_t));

    if(ep_info) {
        direction = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);
        OAM_KEY_PACK(loc,OAM_GROUP_INFO(unit,ep_info->group)->name, MAID_MAX_LEN);
        OAM_KEY_PACK(loc, &ep_info->group, sizeof(ep_info->group));
        OAM_KEY_PACK(loc, &ep_info->name, sizeof(ep_info->name));
        OAM_KEY_PACK(loc, &ep_info->gport, sizeof(ep_info->gport));

        if (OAM_IS_ETHERNET(ep_info->type)) {
            OAM_KEY_PACK(loc, &ep_info->level, sizeof(ep_info->level));
            OAM_KEY_PACK(loc, &ep_info->vlan, sizeof(ep_info->vlan));
            OAM_KEY_PACK(loc, &direction, sizeof(direction));
        }
    }
    assert ((int)(loc - key) <= sizeof(oam_sw_hash_key_t));    
}

int _oam_dump_ep(int unit, bcm_oam_endpoint_info_t *ep_info)
{
    char buf[128], *pbuf;
    int i, base;
    bcm_oam_group_info_t *group_info = NULL;

    OAM_OUT(("EP: flags=0x%08x type=%s\n", ep_info->flags, 
             (OAM_IS_ETHERNET(ep_info->type) ? "enet" : 
              (OAM_IS_BFD(ep_info->type) ? "bfd" : 
              (OAM_IS_LSP_BFD(ep_info->type) ? "lsp-bfd" : "unknown")))));
    OAM_OUT(("  id=%d gport=0x%08x vid=0x%04x\n", ep_info->id, 
             ep_info->gport, ep_info->vlan));
    OAM_OUT(("  group=%d\n", ep_info->group));


    if (ep_info->group < _state[unit]->max_endpoints) {
        group_info = OAM_GROUP_INFO(unit, ep_info->group);
        for (base=0; base<2; base++) {
            pbuf = buf;
            pbuf += sal_sprintf(pbuf, "0x");
            for (i=0; i<16; i++) {
                pbuf += sal_sprintf(pbuf, "%02x", group_info->name[(base * 16) + i]);
                if ((i & 3) == 3) {
                    pbuf += sal_sprintf(pbuf, " ");
                }
            }
            OAM_OUT(("maid=%s\n", buf));
        }
    }

    OAM_OUT(("mep=%d\n", ep_info->name));

    return 0;
}


static 
void print_ep_idx(dq_t* ep_node, int* idx) 
{
    oam_sw_hash_data_t *oam_sw = NULL;
    oam_sw = DQ_ELEMENT(oam_sw_hash_data_t*, ep_node, oam_sw, list_node);

    if (oam_sw != NULL) {
        if (idx == NULL) {
            soc_cm_print("%d", oam_sw->rec_idx);
            return;
        }

        if (*idx > 31) {
            *idx = 0;
            soc_cm_print("\n");
        }
        if (*idx == 0) {
            soc_cm_print("    %d", oam_sw->rec_idx);
        } else {
            soc_cm_print(", %d", oam_sw->rec_idx);
        }
    }

    (*idx)++;
}


void
_bcm_fe2000_oam_sw_dump(int unit)
{
    int grp_idx, trunk_idx, ndx, print_header;
    bcm_oam_group_info_t  *info;

    if (BCM_OAM_IS_INIT(unit) == FALSE) {
        soc_cm_print("Oam sub system not initialized.\n");
        return;
    }

    soc_cm_print("Oam Internal SW state\n");
    soc_cm_print("max_pids=%d (0x%x), max_endpoints=%d (0x%0x) "
                 "timer_base=0x%x\n",
                 _state[unit]->max_pids, _state[unit]->max_pids,
                 _state[unit]->max_endpoints, _state[unit]->max_endpoints,
                 _state[unit]->timer_base);
    soc_cm_print("flags=%s\n", _state[unit]->up_init ? "UpMemInit" : "");

    soc_cm_print("ucode Types/Functions: bfd=%d lm=%d dm=%d ccm_first=%d "
                 "ccm_second=%d \n",
                 _state[unit]->ep_type_ids.bfd, _state[unit]->ep_type_ids.lm,
                 _state[unit]->ep_type_ids.dm, 
                 _state[unit]->ep_type_ids.ccm_first,
                 _state[unit]->ep_type_ids.ccm_second);
    soc_cm_print("                       ccm_peer=%d psc=%d\n",
                 _state[unit]->ep_type_ids.ccm_peer,
                 _state[unit]->ep_type_ids.psc);


    print_header = 1;
    for (trunk_idx=0; trunk_idx < SBX_MAX_TRUNKS; trunk_idx++) {
        if (!DQ_EMPTY(&_state[unit]->trunk_data[trunk_idx].endpoint_list)) {
            if (print_header) {
                soc_cm_print("Trunk data: \n");
                soc_cm_print("TID  EpList\n");
                print_header = 0;
            }

            soc_cm_print("%3d  ", trunk_idx);
            DQ_MAP(&_state[unit]->trunk_data[trunk_idx].endpoint_list,
                   print_ep_idx, NULL);
            soc_cm_print("\n");
        }
    }

    print_header = 1;
    for (grp_idx=0; grp_idx < _state[unit]->max_endpoints; grp_idx++) {
        
        info = OAM_GROUP_INFO(unit, grp_idx);
        if (info) {
            if (print_header) {
                soc_cm_print("Group Info:\n");
                soc_cm_print("ID: Endpoints\n");
                print_header = 0;
            }
            soc_cm_print("%3d: 0x", grp_idx);
            for (ndx=0; ndx<BCM_OAM_GROUP_NAME_LENGTH; ndx++) {

                soc_cm_print("%02x", info->name[ndx]);

                if ((ndx & 3) == 3) {
                    soc_cm_print(" ");
                }

                if (ndx == 23) {
                    soc_cm_print("\n     0x");
                }
            }
            soc_cm_print("\n");

            ndx=0;
            DQ_MAP(&OAM_GROUP_EP_LIST(unit, grp_idx), print_ep_idx, &ndx);

            soc_cm_print("\n");

        }
    }

}

#else   /* INCLUDE_L3 */
int bcm_fe2000_oam_not_empty;
#endif  /* INCLUDE_L3 */
