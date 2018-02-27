/*
 * $Id: trunk.c 1.64.6.3 Broadcom SDK $
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
 * File:    trunk.c
 * Purpose: BCM level APIs for trunking (a.k.a. Port Aggregation)
 */

#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/mcast.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/switch.h>
#include <bcm/stack.h>

#include <bcm_int/robo/trunk.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/common/lock.h>

#include <bcm/link.h>

/*
 * Workaround(provided by ASIC team) 
 * for BCM5324 fe trunk may stop forwarding issue . 
 */

#define BCM5324_FE_TRUNK_ERRATA_WORKAROUND_DEBUG  0
#define MAX_TRUNK_5324  3

typedef struct {
    int in_use;
    int num_ports;
    int sync_to_hw;
    bcm_port_t  tp[BCM_TRUNK_MAX_PORTCNT];  /* Ports in trunk */        
    int link_status[BCM_TRUNK_MAX_PORTCNT];
    pbmp_t pbmp;
} fe_trunk_patch_5324;

static fe_trunk_patch_5324 private_info_5324[BCM_MAX_NUM_UNITS][MAX_TRUNK_5324];

typedef struct fe_trunk_patch_lock_5324_s{
    sal_mutex_t         lock;
} fe_trunk_patch_lock_5324_t;

static fe_trunk_patch_lock_5324_t *fe_trunk_patch_lock_5324[BCM_MAX_NUM_UNITS];
static int fe_trunk_patch_lock_init[BCM_MAX_NUM_UNITS] = {FALSE};

int _bcm_robo_trunk_destroy(int unit, bcm_trunk_t tid, trunk_private_t *t_info);

#define FE_TRUNK_PATCH_LOCK(unit) \
        sal_mutex_take(fe_trunk_patch_lock_5324[unit]->lock, sal_mutex_FOREVER)

#define FE_TRUNK_PATCH_UNLOCK(unit) \
        sal_mutex_give(fe_trunk_patch_lock_5324[unit]->lock)

typedef struct trunk_cntl_s {
    int                ngroups;     /* number of trunk groups */
    int                nports;      /* port count per trunk group */
    trunk_private_t   *t_info;
} trunk_cntl_t;

/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */

static trunk_cntl_t bcm_robo_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_CNTL(unit)    bcm_robo_trunk_control[unit]
#define TRUNK_INFO(unit, tid)   bcm_robo_trunk_control[unit].t_info[tid]

#define SDK_GNATS_897       /* supporting definition on SW GNATS #897 */

/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized.
 */

#define TRUNK_INIT(unit)    \
      if (TRUNK_CNTL(unit).ngroups <= 0 ||TRUNK_CNTL(unit).t_info == NULL) {\
          return BCM_E_INIT; }

/*
 * Make sure TID is within valid range.
 */

#define TRUNK_CHECK(unit, tid) \
    if ((tid) < 0 || (tid) >= TRUNK_CNTL(unit).ngroups) { \
        return BCM_E_PARAM; }

/*
 * Make sure PSC is within valid range.
 */

/* BCM_53242_A0 || BCM_53280_A0 */
#if defined(BCM_53242_A0) || defined(BCM_53280_A0)
#define TRUNK_PSC_CHECK(unit, psc) \
    if ((psc & _BCM_TRUNK_PSC_VALID_VAL) < BCM_TRUNK_PSC_SRCMAC ) { \
        return BCM_E_BADID; } \
    else if ((psc & _BCM_TRUNK_PSC_VALID_VAL) > BCM_TRUNK_PSC_SRCDSTIP) { \
        return BCM_E_UNAVAIL; }
#else /* !(BCM_53242_A0 || BCM_53280_A0)*/
#define TRUNK_PSC_CHECK(unit, psc) \
    if ((psc & _BCM_TRUNK_PSC_VALID_VAL) < BCM_TRUNK_PSC_SRCMAC ) { \
        return BCM_E_BADID; } \
    else if ((psc & _BCM_TRUNK_PSC_VALID_VAL) > BCM_TRUNK_PSC_SRCDSTMAC) { \
        return BCM_E_UNAVAIL; }
#endif /* !(BCM_53242_A0 || BCM_53280_A0) */

/*********************************************************
 *  Internal Routines
 *********************************************************/
#if BCM5324_FE_TRUNK_ERRATA_WORKAROUND_DEBUG
static void print_fe_trunk_patch_info_5324(int unit)
{
    int i,j;
    
    FE_TRUNK_PATCH_LOCK(unit);
    for (i=0;i<MAX_TRUNK_5324;i++) {
        if (private_info_5324[unit][i].in_use) { 
            soc_cm_debug(DK_LINK,"TID:%d, num_ports:%d, sync_to_hw:%d\n",i, 
                private_info_5324[unit][i].num_ports,
                private_info_5324[unit][i].sync_to_hw);
            for (j=0;j<private_info_5324[unit][i].num_ports;j++) {       
                soc_cm_debug(DK_LINK,"tp[%d]: %d\n", j,
                private_info_5324[unit][i].tp[j]);
            }            
            for (j=0;j<BCM_TRUNK_MAX_PORTCNT;j++) {       
                soc_cm_debug(DK_LINK,"link_status[port=%d]: %d\n", j,
                private_info_5324[unit][i].link_status[j]);
            }            
        }
    }
    FE_TRUNK_PATCH_UNLOCK(unit);
}
#endif /* BCM5324_FE_TRUNK_ERRATA_WORKAROUND_DEBUG */

/*
 * Workaround(provided by ASIC team) 
 * for BCM5324 errata, fe trunk may stop forwarding issue . 
 */
void
bcm5324_trunk_patch_linkscan(int unit, soc_port_t port, bcm_port_info_t *info)
{
    int i,j;
    int tid = -1;
    int rv = BCM_E_NONE;
    uint32 flag;
    
    soc_cm_debug(DK_LINK, "5324 trunk: port:%d, linkstatus=%d\n", port, info->linkstatus);
    FE_TRUNK_PATCH_LOCK(unit);
    for (i=0;i<MAX_TRUNK_5324;i++) {
        if (private_info_5324[unit][i].in_use) {
            for (j=0; j<private_info_5324[unit][i].num_ports;j++) {
                if (private_info_5324[unit][i].tp[j] == port) {
                    /* store trunk id, and update member port link status */
                    tid = i;                    
                    private_info_5324[unit][i].link_status[j] = info->linkstatus?1:0;
                    break;
                }
            }
        }
    }
    
    if (tid != -1) {
        /* port in trunk */
        int has_link = 0;
        bcm_trunk_add_info_t tmp_trunk_info;
        pbmp_t trunk_pbmp;
        
        
        soc_cm_debug(DK_LINK, "5324 trunk: port=%d in trunk tid = %d (linkstatus 0x%x)\n", 
            port, tid, info->linkstatus);
            
        memset(&tmp_trunk_info, 0, sizeof(bcm_trunk_add_info_t));
        
        /* check all member link status */
        for (i=0;i<BCM_TRUNK_MAX_PORTCNT;i++) {
            has_link += private_info_5324[unit][tid].link_status[i];
        }

        if (info->linkstatus) { /* linkup event */
            soc_cm_debug(DK_LINK, "5324 trunk: port=%d linkup, has_link=%d\n", port, has_link);        
            if (has_link) {
                /* sync to hw */
                soc_cm_debug(DK_LINK, "5324 trunk: sync trunk %d to hw\n", tid);
                
                SOC_PBMP_ASSIGN(trunk_pbmp, private_info_5324[unit][tid].pbmp);
                flag = DRV_TRUNK_FLAG_BITMAP | DRV_TRUNK_FLAG_ENABLE;
                rv = DRV_TRUNK_SET(unit, tid, trunk_pbmp, flag, 0);
                if (rv < 0) {
                    soc_cm_print("DRV_TRUNK_SET rv = %d\n", rv);
                }
                

            } 
            /* else do nothing to ignore linkup event for non trunk port */
        } else { /* linkdown event */
            if (!has_link) { /* clear register in hardware */
                soc_cm_debug(DK_LINK, "5324 trunk: destroy trunk %d from hw\n", tid);            

            /* bcm_l2_addr_delete_by_trunk */                
            for (i = 0 ; i < private_info_5324[unit][tid].num_ports ; i++){
                rv = bcm_l2_addr_delete_by_port
                    (unit, -1, private_info_5324[unit][tid].tp[i], 0);
            }
            
            if (rv < 0) {
                soc_cm_print("bcm_l2_addr_delete_by_trunk rv = %d\n", rv);
            }

            /* disable this trunk and clear the port bitmap */
            flag = DRV_TRUNK_FLAG_DISABLE | DRV_TRUNK_FLAG_BITMAP;
            BCM_PBMP_CLEAR(trunk_pbmp);
            rv = DRV_TRUNK_SET(unit, tid, trunk_pbmp, flag, 0);
                
                if (rv != BCM_E_NONE) {
                    soc_cm_print("DRV_TRUNK_SET failed rv=%d\n", rv);
                }            
            }
            /* else do nothing since there is at least one linkup member */
        }
    }
    FE_TRUNK_PATCH_UNLOCK(unit);
      
}

/*
 * Function:
 *  _bcm_robo_trunk_destroy
 * Purpose:
 *  Multiplexed trunking function for Robo
 */
int
_bcm_robo_trunk_destroy(int unit, bcm_trunk_t tid, trunk_private_t *t_info)
{
    bcm_port_t      port;
    pbmp_t          tr_pbmp;
    int             size;
    pbmp_t          old_trunk_pbmp, trunk_pbmp;
    uint32          flag, hash_op = 0;

    /* 
     * get trunk pbmp :
     *      - get the old trunk portmap but finally set its' port bitmap 
     *          been 0x0 still.
     */
    flag = DRV_TRUNK_FLAG_BITMAP;
    BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
        (unit, tid, &trunk_pbmp, flag, &hash_op));
    BCM_PBMP_CLEAR(old_trunk_pbmp);
    SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
    BCM_PBMP_COUNT(old_trunk_pbmp, size);
    
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_ITER(old_trunk_pbmp, port) {
        BCM_PBMP_PORT_ADD(tr_pbmp, port);
    }
    
    /* clear the learned trunk ports from L2 table */ 
    BCM_IF_ERROR_RETURN(bcm_l2_addr_delete_by_trunk(unit, tid, 0));
    
    /* disable this trunk and clear the port bitmap */
    flag = DRV_TRUNK_FLAG_DISABLE | DRV_TRUNK_FLAG_BITMAP;
    SOC_PBMP_CLEAR(trunk_pbmp);
    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
        (unit, tid, trunk_pbmp, flag, hash_op));
    
    return (BCM_E_NONE);
    
}

/*
 * Function:
 *  _bcm_robo_trunk_mcast_join
 * Purpose:
 *  Multiplexed function for Robo
 */

int
_bcm_robo_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid,
                           sal_mac_addr_t mac, trunk_private_t *t_info)
{
    bcm_mcast_addr_t    mc_addr;
    pbmp_t          pbmp, m_pbmp, trunk_pbmp;
    uint32          flag, hash_op;
    int rv = BCM_E_NONE;


    if (SOC_IS_ROBO5324(unit)) {
        /* get the ports in the trunk group */
        /*
         *  FE trunk erratum patch for 5324
         *  redirect to private_info_5324 to get the trunk info
         */
        FE_TRUNK_PATCH_LOCK(unit);                              
        if (private_info_5324[unit][tid].in_use) {
            SOC_PBMP_ASSIGN(trunk_pbmp, private_info_5324[unit][tid].pbmp);
        } else {
            /* read trunk info from ASIC for GE trunk */
            flag = DRV_TRUNK_FLAG_BITMAP;
            rv = DRV_TRUNK_GET
                        (unit, tid, &trunk_pbmp, flag, &hash_op);
            if (rv < 0) {
                FE_TRUNK_PATCH_UNLOCK(unit); 
                return rv;
            }
        }
        FE_TRUNK_PATCH_UNLOCK(unit); 
    } else {
        flag = DRV_TRUNK_FLAG_BITMAP;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
            (unit, tid, &trunk_pbmp, flag, &hash_op));
    }

    soc_cm_debug(DK_VERBOSE, 
                "trunk_mcast_join: \n\t trunk_pbm=0x%x\n",SOC_PBMP_WORD_GET(trunk_pbmp, 0));
    BCM_PBMP_CLEAR(pbmp);
    SOC_PBMP_ASSIGN(pbmp, trunk_pbmp);

    BCM_IF_ERROR_RETURN(bcm_mcast_port_get(unit, mac, vid, &mc_addr));
    soc_cm_debug(DK_VERBOSE, 
                "\t mcast_pbm=0x%x\n",SOC_PBMP_WORD_GET(mc_addr.pbmp, 0));
    BCM_PBMP_ASSIGN(m_pbmp, mc_addr.pbmp);
    BCM_PBMP_XOR(m_pbmp, pbmp);
    BCM_PBMP_AND(m_pbmp, pbmp);
    if (SOC_PBMP_IS_NULL(m_pbmp)) {
        return (BCM_E_CONFIG);
    }

    BCM_IF_ERROR_RETURN(bcm_mcast_addr_remove(unit, mac, vid));
    SOC_PBMP_REMOVE(mc_addr.pbmp, pbmp);
    if (t_info->mc_port_used >= 0) {
        BCM_PBMP_PORT_ADD(mc_addr.pbmp, t_info->mc_port_used);
    }
    soc_cm_debug(DK_VERBOSE, 
          "\t last mc_addr.pbmp=0x%x\n",SOC_PBMP_WORD_GET(mc_addr.pbmp,0));
    BCM_IF_ERROR_RETURN(bcm_mcast_addr_add(unit, &mc_addr));

    return (BCM_E_NONE);
}

#ifndef SDK_GNATS_897
/*
 * Function:
 *        _robo_trunk_check_vlan
 * Purpose:
 *        Make sure all ports in t_pbmp_new are in the same VLANs
 * Parameters:
 *        t_pbmp_new - ports to be added to a trunk group
 *        t_pbmp_old - previous ports in this trunk group.
 */

STATIC int
_robo_trunk_check_vlan(int unit, bcm_pbmp_t t_pbmp_new, bcm_pbmp_t t_pbmp_old)
{
    bcm_vlan_data_t *list;
    bcm_pbmp_t      pbmp;
    int             i, count, port;
    bcm_vlan_t      vid0, vid;
    uint32          vlan_mode;

    BCM_IF_ERROR_RETURN(DRV_VLAN_MODE_GET
        (unit, (uint32 *)&vlan_mode));
    if  (vlan_mode == DRV_VLAN_MODE_PORT_BASE){           /* port based VLAN */
        /* check port based VLAN table to see if any conflicts */
        vid = 0;
        PBMP_ITER(t_pbmp_new, port) {
            /* call port API */
            BCM_IF_ERROR_RETURN(bcm_port_untagged_vlan_get(unit, port, &vid0));
            break;
        }
    
        PBMP_ITER(t_pbmp_new, port) {
            /* call port API */
            BCM_IF_ERROR_RETURN(bcm_port_untagged_vlan_get(unit, port, &vid));
            if (vid != vid0) {
                soc_cm_debug(DK_ERR, "Trunk ports in different VLANs\n");
                return (BCM_E_CONFIG);
            }
        }
    }else{                          /* tagged VLAN */
        /* check tagged based VLAN table  */
        BCM_IF_ERROR_RETURN
            /* call vlan API */
            (bcm_vlan_list_by_pbmp(unit, t_pbmp_new, &list, &count));
    
        for (i = 0; i < count; i++) {
            BCM_PBMP_ASSIGN(pbmp, list[i].port_bitmap);
            BCM_PBMP_OR(pbmp, t_pbmp_old);
    
            PBMP_ITER(t_pbmp_new, port) {
                if (!PBMP_MEMBER(pbmp, port)) {
                    soc_cm_debug(DK_ERR,
                     "Trunk conflicts VLAN %d: port %d\n",
                                 list[i].vlan_tag, port);
                    /* call vlan API */
                    BCM_IF_ERROR_RETURN
                        (bcm_vlan_list_destroy(unit, list, count));
                    return (BCM_E_CONFIG);
                }
            }
        }
        /* call vlan API */
        BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, list, count));
    
    }   
    return (BCM_E_NONE);

}
#endif

/*
 * Function:
 *  _bcm_robo_trunk_get
 * Purpose:
 *  Multiplexed function for Robo
 * Note:
 *
 */
int
_bcm_robo_trunk_get(int unit, bcm_trunk_t tid,
                    bcm_trunk_add_info_t *t_data,
                    trunk_private_t *t_info)
{
    bcm_pbmp_t  old_trunk_pbmp;

    bcm_port_t  port;
    int         size;
    uint32      flag, hash_op;
    uint32      port_counter = 0;
    pbmp_t      trunk_pbmp;

    if (!t_info->in_use) {
        t_data->num_ports = 0;
    } else {
        if (SOC_IS_ROBO5324(unit)) {
            int rv = BCM_E_NONE;
            /*
             *  FE trunk erratum patch for 5324
             *  redirect to private_info_5324 to get the trunk info
             */
            FE_TRUNK_PATCH_LOCK(unit);                              
            if (private_info_5324[unit][tid].in_use) {
                int i;
                t_data->num_ports = private_info_5324[unit][tid].num_ports;
                for (i=0;i<t_data->num_ports;i++) {
                    t_data->tp[i] = private_info_5324[unit][tid].tp[i];
                }
            } else {
                /* read trunk info from ASIC for GE trunk */
                /*
                 * Read the old trunk member pbmp
                 */
                flag = DRV_TRUNK_FLAG_BITMAP;
                if ((rv = DRV_TRUNK_GET
                                (unit, tid, &trunk_pbmp, flag, &hash_op)) < 0) {
                    FE_TRUNK_PATCH_UNLOCK(unit);                     
                    return rv;
                }

                BCM_PBMP_CLEAR(old_trunk_pbmp);
                SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
                BCM_PBMP_COUNT(old_trunk_pbmp, size);
                t_data->num_ports = size;
                
                BCM_PBMP_ITER(old_trunk_pbmp, port) {
                    if (port_counter < size){
                        t_data->tp[port_counter] = port;
                        t_data->tm[port_counter] = unit;
                        port_counter++;
                    }else{
                        break;
                    }
                }
            }
            FE_TRUNK_PATCH_UNLOCK(unit);                     
        } else {
            /*
             * Read the old trunk member pbmp
             */
            flag = DRV_TRUNK_FLAG_BITMAP;
            BCM_PBMP_CLEAR(trunk_pbmp);
            BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
                (unit, tid, &trunk_pbmp, flag, &hash_op));
            BCM_PBMP_CLEAR(old_trunk_pbmp);
            SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
            BCM_PBMP_COUNT(old_trunk_pbmp, size);
            t_data->num_ports = size;
            
            BCM_PBMP_ITER(old_trunk_pbmp, port) {
                if (port_counter < size){
                    t_data->tp[port_counter] = port;
                    t_data->tm[port_counter] = unit;
                    port_counter++;
                }else{
                    break;
                }
            }
        }
    }

    t_data->psc = t_info->psc;
    t_data->mc_index = t_info->mc_index_used;

    return (BCM_E_NONE);
    
}

/*
 * Function:
 *  _bcm_robo_trunk_set
 * Purpose:
 *  internal trunk_set function for Robo.
 * Note:
 *  1. assume this tid has been created and the trunk_pbmp
 *      is a valid value. (bcm_trunk_set() has verified on 
 *      tid & trunk ports)
 */
int
_bcm_robo_trunk_set(int unit, bcm_trunk_t tid,
             bcm_trunk_add_info_t *add_info,
                     trunk_private_t *t_info)
{
    bcm_port_t  port;
    pbmp_t      old_trunk_pbmp;
    pbmp_t      new_trunk_pbmp;
    pbmp_t      diff_pbmp;
    pbmp_t      trunk_pbmp;
    int         i, mc_index;
    int         hasfe = 0, hasge;
    int         my_modid;
    uint32      flag, hash_op, hash_op_new = 0;

    /* 
     * bcm_stk_my_modid_get() is stack API and not designed for bcm5324!
     * >> The current API is dummy
     * >> Force my_modid=0 here(Should be removed if this API is implemented)
     *      - replace "bcm_stk_my_modid_get(unit, &my_modid);" by 
     *          "my_modid = 0;"
     */
    my_modid = 0;
    
    /*
     * make sure the ports supplied are valid and DLF port is
     * part of the trunk group ports
     */
    for (i = 0; i < add_info->num_ports; i++) {
        if (my_modid != add_info->tm[i]) {
            continue;
        }
        port = add_info->tp[i];
        if (!IS_E_PORT(unit, port) && !IS_CPU_PORT(unit, port)) {
            return (BCM_E_BADID);
        }
    }

    if (add_info->psc <= 0) {
        add_info->psc = BCM_TRUNK_PSC_DEFAULT;
    }

    /* Set the Trunk criteria */
    switch (add_info->psc & _BCM_TRUNK_PSC_VALID_VAL) {
    case BCM_TRUNK_PSC_SRCMAC:
        hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_MACDA | 
                        DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));
        break;
    case BCM_TRUNK_PSC_DSTMAC:
        hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_MACSA | 
                        DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));

        break;
    case BCM_TRUNK_PSC_SRCDSTMAC:
        hash_op = DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));

        break;
    /* this psc may not be suitable for all Robo chips but 
     *      implemented for general purpose on supporting every 
     *      Robo Chip.
     * Below psc type will be implemented in case further RoboChip 
     *      provides those psc type.
     */
    case BCM_TRUNK_PSC_SRCIP:
        hash_op = DRV_TRUNK_HASH_FIELD_IP_MACSA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_IP_MACDA | 
                        DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_IP_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));
        break;
    case BCM_TRUNK_PSC_DSTIP:
        hash_op = DRV_TRUNK_HASH_FIELD_IP_MACDA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_IP_MACSA | 
                        DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_IP_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));

        break;
    case BCM_TRUNK_PSC_SRCDSTIP:
        hash_op = DRV_TRUNK_HASH_FIELD_IP_MACDA | DRV_TRUNK_HASH_FIELD_IP_MACSA;
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
        /* remove the undefined psc 
         *  - below psc may not be suitable for all Robo chips but 
         *      implemented for general purpose on supporting every 
         *      Robo Chip.
         */
        BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                        (unit, 
                        DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                        DRV_TRUNK_HASH_FIELD_IP_VLANID |
                            DRV_TRUNK_HASH_FIELD_L3));

        break;
    default:
        return BCM_E_PARAM;
    }
    
    /* OR-able hash key selection : additional fields for hashing in trunk mode */
    if (add_info->psc & ~_BCM_TRUNK_PSC_VALID_VAL) {
        if (add_info->psc & 
            ~(_BCM_TRUNK_PSC_VALID_VAL | _BCM_TRUNK_PSC_ORABLE_VALID_VAL)) {
            return BCM_E_UNAVAIL;
        }

        hash_op_new = hash_op;
        if (add_info->psc & BCM_TRUNK_PSC_IPMACSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACSA | 
                DRV_TRUNK_HASH_FIELD_MACSA;
        }
        if (add_info->psc & BCM_TRUNK_PSC_IPMACDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACDA | 
                DRV_TRUNK_HASH_FIELD_MACDA;
        }
        if (add_info->psc & BCM_TRUNK_PSC_IPSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACSA;
        }
        if (add_info->psc & BCM_TRUNK_PSC_IPDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACDA;
        }
        if (add_info->psc & BCM_TRUNK_PSC_MACSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_MACSA;
        }
        if (add_info->psc & BCM_TRUNK_PSC_MACDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_MACDA;
        }

        if (hash_op_new != hash_op) {
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op_new));
        }
    }

    /*
     * Read the old trunk member pbmp
     */
    flag = DRV_TRUNK_FLAG_BITMAP;
    BCM_PBMP_CLEAR(trunk_pbmp);
    BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
        (unit, tid, &trunk_pbmp, flag, &hash_op));
    BCM_PBMP_CLEAR(old_trunk_pbmp);
    SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
    
    /*
     * Write new turnk member setting process
     */
    BCM_PBMP_CLEAR(new_trunk_pbmp);
    for (i = 0; i < add_info->num_ports; i++) {
        port = add_info->tp[i];
        BCM_PBMP_PORT_ADD(new_trunk_pbmp, port);
    }

    /*
     * make sure trunked ports are all FE or all GE (except for stack ports)
     */
    /* Limitation for bcm5324 */
    if (SOC_IS_ROBO5324(unit)) {
        BCM_PBMP_ASSIGN(diff_pbmp, PBMP_FE_ALL(unit));
        BCM_PBMP_AND(diff_pbmp, new_trunk_pbmp);
        hasfe = BCM_PBMP_NOT_NULL(diff_pbmp);
        BCM_PBMP_ASSIGN(diff_pbmp, PBMP_GE_ALL(unit));
        BCM_PBMP_AND(diff_pbmp, new_trunk_pbmp);
        hasge = BCM_PBMP_NOT_NULL(diff_pbmp);
        if (hasfe && hasge) {
            soc_cm_debug(DK_ERR, "Trunk ports exist in FE and GE!\n");
            return (BCM_E_CONFIG);
        } else if (hasfe && (tid == 2)) {
            soc_cm_debug(DK_ERR, "Trunk Group ID %d exist FE ports!\n", tid);
            return (BCM_E_PARAM);
        } else if (hasge && (tid <= 1)) {
            soc_cm_debug(DK_ERR, "Trunk Group ID %d exist GE ports!\n", tid);
            return (BCM_E_PARAM);
        }
    }

    BCM_PBMP_ASSIGN(diff_pbmp, old_trunk_pbmp);
    BCM_PBMP_XOR(diff_pbmp, new_trunk_pbmp);
    
    /* VLAN check is now the responsibility of the caller. */
    #ifndef SDK_GNATS_897
    /*
     * Make sure all ports in the trunk group are in the same VLAN.
     */
    BCM_IF_ERROR_RETURN
        (_robo_trunk_check_vlan(unit, new_trunk_pbmp, old_trunk_pbmp));
    #endif
    
    /*
     * Update Trunk setting based on new trunk bitmap, if necessary
     */
    if (BCM_PBMP_NOT_NULL(diff_pbmp)) {
        /* write the pbmp into this trunk */
        SOC_PBMP_ASSIGN(trunk_pbmp, new_trunk_pbmp);
        flag = DRV_TRUNK_FLAG_BITMAP | DRV_TRUNK_FLAG_ENABLE;
        if (SOC_IS_ROBO5324(unit)) {
            /*
             *  FE trunk erratum patch for 5324
             *
             *  1. get link status for each trunk member port
             *  2. update fe_trunk_info_5324[]
             *  3. call driver service to update hw only when there is at least 
             *     one link up member port
             */
            if (hasfe) {            
                /* only deal with fe trunks */
                int port;
                int sync_to_hw = 0;            
                int i;
    
                soc_cm_debug(DK_LINK, "5324 fe trunk workaround start\n");
                /* keep trunk info for this patch */
                FE_TRUNK_PATCH_LOCK(unit);                     
                memset(&private_info_5324[unit][tid], 0, sizeof(fe_trunk_patch_5324));
                private_info_5324[unit][tid].num_ports = add_info->num_ports;
                i = 0;            
    
                BCM_PBMP_ITER(new_trunk_pbmp, port) {
                    int tmpLnk;
                    if (BCM_E_NONE == bcm_port_link_status_get(unit, port, &tmpLnk)) {
                        if (tmpLnk)
                            sync_to_hw = 1;
                             
                        private_info_5324[unit][tid].link_status[i]=tmpLnk ? 1:0;
                    } else {
                        soc_cm_print("bcm_port_link_status_get failed: port = %d\n", port);
                    }   
                    private_info_5324[unit][tid].tp[i] = port;
                    i++;                
                }
                private_info_5324[unit][tid].num_ports = i;
                private_info_5324[unit][tid].sync_to_hw = sync_to_hw;
                BCM_PBMP_ASSIGN(private_info_5324[unit][tid].pbmp, new_trunk_pbmp);
                private_info_5324[unit][tid].in_use = TRUE;
                
                FE_TRUNK_PATCH_UNLOCK(unit);         
                
                if (sync_to_hw) {   /* atleast one link is up */
                    soc_cm_debug(DK_LINK, "sync trunk %d to hw\n", tid);
                    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
                        (unit, tid, trunk_pbmp, flag, 0));
                    
                } else {
                    soc_cm_debug(DK_LINK, "dont sync trunk %d to hw\n", tid);            
                    BCM_PBMP_CLEAR(old_trunk_pbmp);                
                    SOC_PBMP_ASSIGN(trunk_pbmp, old_trunk_pbmp);
                    /* 
                     * This is needed if the user removes all the active 
                     * links from the trunk.
                     */
                    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
                        (unit, tid, trunk_pbmp, flag, 0));
                }
#if BCM5324_FE_TRUNK_ERRATA_WORKAROUND_DEBUG
                print_fe_trunk_patch_info_5324(unit);
#endif /* BCM5324_FE_TRUNK_ERRATA_WORKAROUND_DEBUG */            
            } else { /* for GE trunk */
                BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
                    (unit, tid, trunk_pbmp, flag, 0));
            }
        } else {
            BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
                (unit, tid, trunk_pbmp, flag, 0));
        }
    }
    
    /* set trunk SW information */
    if (BCM_PBMP_NOT_NULL(new_trunk_pbmp)) {
        t_info->mc_index_spec = add_info->mc_index;

        /* determine the multicast port */
        mc_index = t_info->mc_index_spec;
        if (mc_index < 0) {
            mc_index = 0;
        }
        t_info->mc_index_used = mc_index;
        if (add_info->tm[mc_index] == my_modid) {
            t_info->mc_port_used = add_info->tp[mc_index];
        } else {
            t_info->mc_port_used = -1;
        }
    }

    return (BCM_E_NONE);
}

/************** Internal Routine ***********************/

/*
 * Function:
 *  bcm_robo_trunk_init
 * Purpose:
 *  Initializes the trunk tables to empty (no trunks configured)
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int 
bcm_robo_trunk_init(int unit)
{
    int             alloc_size;
    trunk_private_t *t_info;
    bcm_trunk_t     tid;
    uint32          trunk_num = 0;
    uint32          trunk_max_port = 0;
    uint32          flag, hash_op;
    pbmp_t          trunk_pbmp;
    bcm_trunk_add_info_t t_add_info;
    int i;

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_init()..\n");
    TRUNK_CNTL(unit).ngroups = 0;
    /* get the number of allowed turnk groups in this unit */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_num));
    TRUNK_CNTL(unit).ngroups = trunk_num;
    
    /* set the number of ports that allowed in a trunk in this unit */
    flag = DRV_TRUNK_FLAG_HASH_DEFAULT;
    hash_op = DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
    SOC_PBMP_CLEAR(trunk_pbmp);
    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
        (unit, 0, trunk_pbmp, flag, hash_op));
    /* Get the max port number allowed in a given trunk. */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_MAX_PORT_NUM, &trunk_max_port));
    TRUNK_CNTL(unit).nports = trunk_max_port;
   
    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
        /* disable all trunk group and clear all trunk bitmap*/
        flag = DRV_TRUNK_FLAG_DISABLE | DRV_TRUNK_FLAG_BITMAP;
        SOC_PBMP_CLEAR(trunk_pbmp);
        BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
            (unit, tid, trunk_pbmp, flag, 0));
    }

    if (TRUNK_CNTL(unit).t_info != NULL) {
        sal_free(TRUNK_CNTL(unit).t_info);
        TRUNK_CNTL(unit).t_info = NULL;
    }

    if (TRUNK_CNTL(unit).ngroups > 0) {
        alloc_size = TRUNK_CNTL(unit).ngroups * sizeof(trunk_private_t);
        t_info = (trunk_private_t *) sal_alloc(alloc_size, "trunk_priv");
        if (t_info == NULL) {
            return (BCM_E_MEMORY);
        }
        TRUNK_CNTL(unit).t_info = t_info;
        
        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
            t_info->tid = BCM_TRUNK_INVALID;
            t_info->in_use = FALSE;
            t_info->mc_index_spec = -1;
            t_info->mc_index_used = -1;
            t_info++;
        }
    }

    /*
     * Workaround : only for BCM5348
     * GNATS:14279 : trunk issue with fe0 as member port
     */
    if (SOC_IS_ROBO5348(unit)) {
        sal_memset(&t_add_info, 0, sizeof (t_add_info));
        t_add_info.tp[0] = CMIC_PORT(unit);
        t_add_info.num_ports = 1;
        SOC_PBMP_CLEAR(trunk_pbmp);
        SOC_PBMP_PORT_SET(trunk_pbmp, CMIC_PORT(unit));
        /* create all trunk groups with IMP port as trunk member */
        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
            BCM_IF_ERROR_RETURN(bcm_trunk_create_id(unit, tid));
            BCM_IF_ERROR_RETURN(bcm_trunk_set(unit, tid, &t_add_info));
        }

        /* destroy all trunk groups */
        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
            BCM_IF_ERROR_RETURN(bcm_trunk_destroy(unit, tid));
        }
    }

    if (SOC_IS_ROBO5324(unit)) {
        if (!fe_trunk_patch_lock_init[unit]) {
            for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
                fe_trunk_patch_lock_5324[i] = NULL;
            }
            fe_trunk_patch_lock_init[unit] = TRUE;
        }
        
        if (fe_trunk_patch_lock_5324[unit] != NULL) {
            if (fe_trunk_patch_lock_5324[unit]->lock != NULL) {
                sal_mutex_destroy(fe_trunk_patch_lock_5324[unit]->lock);
                fe_trunk_patch_lock_5324[unit]->lock = NULL;
            }
        } else {
            fe_trunk_patch_lock_5324[unit] = \
                sal_alloc(sizeof(fe_trunk_patch_lock_5324_t), "BCM5324 trunk errata lock");
    
            if (fe_trunk_patch_lock_5324[unit] == NULL) {
                return BCM_E_MEMORY;
            }

            sal_memset(fe_trunk_patch_lock_5324[unit], 0, sizeof(fe_trunk_patch_lock_5324_t));
        }

        fe_trunk_patch_lock_5324[unit]->lock = \
            sal_mutex_create("bcm_fe_trunk_patch_lock");
        if (fe_trunk_patch_lock_5324[unit]->lock == NULL) {
            sal_free(fe_trunk_patch_lock_5324[unit]);
            fe_trunk_patch_lock_5324[unit] = NULL;
            return BCM_E_MEMORY;
        }
    
        memset(&private_info_5324, 0, BCM_MAX_NUM_UNITS*MAX_TRUNK_5324*sizeof(fe_trunk_patch_5324));

    }


    return BCM_E_NONE;
}               

/*
 * Function:
 *  bcm_robo_trunk_detach
 * Purpose:
 *  Cleans up the trunk tables.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_detach(int unit)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_detach()..\n");
    if (TRUNK_CNTL(unit).t_info != NULL) {
        sal_free(TRUNK_CNTL(unit).t_info);
        TRUNK_CNTL(unit).t_info = NULL;
    }
    TRUNK_CNTL(unit).ngroups = 0;

    if (SOC_IS_ROBO5324(unit)) {
        if (fe_trunk_patch_lock_5324[unit] != NULL) {
            if (fe_trunk_patch_lock_5324[unit]->lock != NULL) {
                sal_mutex_destroy(fe_trunk_patch_lock_5324[unit]->lock);
                fe_trunk_patch_lock_5324[unit]->lock = NULL;
            }
            sal_free(fe_trunk_patch_lock_5324[unit]);
            fe_trunk_patch_lock_5324[unit] = NULL;
        }
    }

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_create
 * Purpose:
 *      Create the software data structure for a trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - (Out), The trunk ID.
 * Returns:
 *      BCM_E_NONE     - Success.
 *      BCM_E_FULL - run out of TIDs
 */
int 
bcm_robo_trunk_create(int unit, bcm_trunk_t *tid)
{
    trunk_private_t *t_info;
    int rv, i;

    TRUNK_INIT(unit);

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_create()..\n");

    rv = BCM_E_FULL;

    t_info = TRUNK_CNTL(unit).t_info;

    for (i = 0; i < TRUNK_CNTL(unit).ngroups; i++) {
        if (t_info->tid == BCM_TRUNK_INVALID) {
            t_info->tid = i;
            t_info->in_use = FALSE;
            t_info->psc = BCM_TRUNK_PSC_DEFAULT;
            *tid = i;
            rv = BCM_E_NONE;
            break;
        }
        t_info++;
    }

    return rv;
}               
    
/*
 * Function:
 *  _bcm_robo_trunk_id_validate
 * Purpose:
 *  Service routine to validate validity of trunk id.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_INIT    - trunking software not initialized
 *      BCM_E_BADID   - TID out of range
 */
int
_bcm_robo_trunk_id_validate(int unit, bcm_trunk_t tid)
{
    int rv;
    bcm_trunk_add_info_t t_add_info;
        
    TRUNK_CHECK(unit, tid);

    rv = bcm_trunk_get(unit, tid, &t_add_info);
    
    if (rv < 0){
        return BCM_E_PARAM;
    }
    
    /* 
     * If no any port assigned to the current trunk,
     * there will be no further processes.
     * Since no tgid information recorded in the arl entry of robo chip.
     */
    if (!t_add_info.num_ports) {
        return BCM_E_PARAM;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *  bcm_robo_trunk_create_with_tid
 * Purpose:
 *      Create the software data structure for the specified trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE       - Success.
 *      BCM_E_INIT       - trunking software not initialized
 *      BCM_E_EXISTS - TID already used
 *      BCM_E_BADID   - TID out of range
 */
int 
bcm_robo_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_create_with_tid()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        t_info->tid = tid;
        t_info->psc = BCM_TRUNK_PSC_DEFAULT;
        t_info->in_use = FALSE;
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_EXISTS;
    }

    return rv;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be affected.
 *      psc - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_private_t     *t_info;
    bcm_trunk_add_info_t    add_info;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_psc_set()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    if (psc <= 0) {
        psc = BCM_TRUNK_PSC_DEFAULT;
    }

    if (t_info->psc == psc) {
        return BCM_E_NONE;
    }

    if (!t_info->in_use) {
        t_info->psc = psc;
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(bcm_trunk_get(unit, tid, &add_info));
    add_info.psc = psc;
    BCM_IF_ERROR_RETURN(bcm_trunk_set(unit, tid, &add_info));
    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be used.
 *      psc - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_psc_get()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        *psc = 0;
        rv = BCM_E_NOT_FOUND;
    } else {
        *psc = t_info->psc;
        rv = BCM_E_NONE;
    }

    return rv;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_chip_info_get
 * Purpose:
 *      Get the trunk information.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      t_info - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 * Notes:
 *      None.
 */
int 
bcm_robo_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    TRUNK_INIT(unit);

    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_chip_info_get()..\n");
    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = TRUNK_CNTL(unit).ngroups - 1;
    ta_info->trunk_group_count = TRUNK_CNTL(unit).ngroups;
    ta_info->trunk_ports_max = TRUNK_CNTL(unit).nports;

    /* There is no supported fabric trunk info. for ROBO, set -1 as initialized value */
    ta_info->trunk_fabric_id_min = -1;
    ta_info->trunk_fabric_id_max = -1;
    ta_info->trunk_fabric_ports_max = -1;

    return BCM_E_NONE;
}               

/*
 * Function:
 *	_bcm_robo_trunk_gport_construct
 * Purpose:
 *	Converts ports and modules given in t_data structure into gports
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      count - number of failover gports in list
 *      port_list - (IN) list of port numbers
 *      modid_list - (IN) list of module ids
 *      port_array - (OUT) list of gports to return
 *
 * Note:
 *      port_list and port_array may be the same list.  This updates
 *      the list in place.
 */
STATIC int
_bcm_robo_trunk_gport_construct(int unit, int count,
                               bcm_port_t *port_list,
                               bcm_module_t *modid_list,
                               bcm_gport_t *port_array)
{
    bcm_gport_t  gport;
    int  i, mod_is_local;
    _bcm_gport_dest_t  dest;

    sal_memset(&dest, 0, sizeof (_bcm_gport_dest_t));
    
    for (i = 0; i < count; i++) {
        gport = 0;
        /* Stacking ports should be encoded as devport */
        BCM_IF_ERROR_RETURN
            (_bcm_robo_modid_is_local(unit, modid_list[i],
                                     &mod_is_local));
        if (mod_is_local && IS_ST_PORT(unit, port_list[i])) {
            dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
        } else {
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
            dest.modid = modid_list[i];
        }

        dest.port = port_list[i];
        BCM_IF_ERROR_RETURN
            (_bcm_robo_gport_construct(unit, &dest, &gport));
        port_array[i] = gport;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_trunk_grpot_resolve
 * Purpose:
 *  Converts gports given in t_data structure into ports and modules
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      t_data -   Information on the trunk group.
 */
int
_bcm_robo_trunk_gport_resolve(int unit, bcm_trunk_add_info_t *t_data)
{
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             i, id;

    for (i = 0; i < t_data->num_ports; i++) {
        if (BCM_GPORT_IS_SET(t_data->tp[i])) {
            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, t_data->tp[i], &modid, &port, 
                                       &tgid, &id));
            if ((-1 != tgid) || (-1 != id)) {
                return BCM_E_PARAM;
            }
            t_data->tp[i] = port;
            t_data->tm[i] = modid;
        }
    }

    return BCM_E_NONE;
}
    
/*
 * Function:
 *  bcm_robo_trunk_set
 * Purpose:
 *      Adds ports to a trunk group.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be affected.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 * Notes:
 *
 */
int 
bcm_robo_trunk_set(int unit, bcm_trunk_t tid,
             bcm_trunk_add_info_t *add_info)
{
    trunk_private_t *t_info;
    bcm_trunk_add_info_t  t_add;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_set()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    /* Check number of ports in trunk group */
    if (add_info->num_ports < 1 ||
                add_info->num_ports > TRUNK_CNTL(unit).nports) {
        return (BCM_E_PARAM);
    }

    bcm_trunk_add_info_t_init(&t_add);
    sal_memcpy(&t_add, add_info, sizeof(bcm_trunk_add_info_t));

    BCM_IF_ERROR_RETURN(
        _bcm_robo_trunk_gport_resolve(unit, &t_add));

    rv = _bcm_robo_trunk_set(unit, tid, &t_add, t_info);
    if (rv >= 0) {
        t_info->psc = add_info->psc;
        t_info->in_use = TRUE;
    }

    return (rv);
}               

/* 
 * Function:
 *  bcm_534x_trunk_destroy
 * Purpose:
 *  Removes a trunk group.
 * Parameters:
 *      unit - Device unit number (driver internal).
 *      tid - Trunk Id.
 * Returns:
 *      BCM_E_NONE     Success.
 *      BCM_E_XXX
 * Notes:
 *
 */
int 
bcm_robo_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_destroy()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        t_info->tid = BCM_TRUNK_INVALID;
        return (BCM_E_NONE);
    }

    rv =  _bcm_robo_trunk_destroy(unit, tid, t_info);
    
    if (rv >= 0 ) {
        t_info->tid = BCM_TRUNK_INVALID;
        t_info->in_use = FALSE;
    }

    if (SOC_IS_ROBO5324(unit)) {
        /*
         *  FE trunk erratum patch for 5324
         *
         *  clear the private_info_5324 for this patch
         */
        FE_TRUNK_PATCH_LOCK(unit);              
        memset(&private_info_5324[unit][tid], 0, sizeof(fe_trunk_patch_5324));
        private_info_5324[unit][tid].in_use = FALSE;    
    
        FE_TRUNK_PATCH_UNLOCK(unit);             
    }
    return (rv);
}               
    
/*
 * Function:
 *  bcm_robo_trunk_get
 * Purpose:
 *      Return a port information of given trunk ID.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - Trunk ID.
 *      pbmp - (Out), Place to store bitmap of returned ports.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 * Notes:
 *      The bitmap returned always includes the port itself.
 */
int 
bcm_robo_trunk_get(int unit, bcm_trunk_t tid,
             bcm_trunk_add_info_t *t_data)
{
    trunk_private_t *t_info;
    int isGport = 0;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_get()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    rv = _bcm_robo_trunk_get(unit, tid, t_data, t_info);

    BCM_IF_ERROR_RETURN(
        bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_trunk_gport_construct(unit, t_data->num_ports,
                                           t_data->tp, t_data->tm,
                                           t_data->tp));
    }

    return rv;
}               
             
/*
 * Function:
 *  bcm_robo_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all of the ports of that trunk to the bitmap.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 * Notes:
 *      RoboSwitch family not suitable for this API.
 */
int 
bcm_robo_trunk_bitmap_expand(int unit, bcm_pbmp_t *pbmp_ptr)
{
    int                 rv = BCM_E_NONE;
    pbmp_t              t_pbmp, trunk_pbmp;
    int                 i;
    uint32              hash_op;
    bcm_trunk_chip_info_t   ti;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_bitmap_expand().. \n");
    
    BCM_IF_ERROR_RETURN(bcm_trunk_chip_info_get(unit, &ti));
    for (i = ti.trunk_id_min ; i <= ti.trunk_id_max ; i++){
        SOC_PBMP_CLEAR(trunk_pbmp);
        if (SOC_IS_ROBO5324(unit)) {
            /*
             *  FE trunk erratum patch for 5324
             *  redirect to private_info_5324 to get the trunk info
             */
            FE_TRUNK_PATCH_LOCK(unit);
            if (private_info_5324[unit][i].in_use) {
                int j, num_tports;;
                
                num_tports = private_info_5324[unit][i].num_ports;
                for (j=0; j<num_tports; j++) {
                    BCM_PBMP_PORT_ADD(trunk_pbmp, 
                            private_info_5324[unit][i].tp[j]);
                }
            } else {
                /* read trunk info from ASIC for GE trunk */
                /*
                 * Read the old trunk member pbmp
                 */
                BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
                    (unit, i, &trunk_pbmp, DRV_TRUNK_FLAG_BITMAP, &hash_op));
            }
            FE_TRUNK_PATCH_UNLOCK(unit);
        } else {
            BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
                (unit, i, &trunk_pbmp, DRV_TRUNK_FLAG_BITMAP, &hash_op));
        }
        
        SOC_PBMP_ASSIGN(t_pbmp, *pbmp_ptr);
        SOC_PBMP_AND(t_pbmp, trunk_pbmp);
        if (SOC_PBMP_NOT_NULL(t_pbmp)) {
                SOC_PBMP_OR(*pbmp_ptr, trunk_pbmp);
        }
    }

    return rv;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_mcast_join
 * Purpose:
 *  Add the trunk group to existing MAC multicast entry.
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      tid - Trunk Id.
 *      vid - Vlan ID.
 *      mac - MAC address.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *      Applications have to remove the MAC multicast entry and re-add in with
 *      new port bitmap to remove the trunk group from MAC multicast entry.
 */
int 
bcm_robo_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid,
                            bcm_mac_t mac)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_mcast_join()..\n");
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);
    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        return (BCM_E_NONE);
    }

    rv = _bcm_robo_trunk_mcast_join(unit, tid, vid, mac, t_info);

    return rv;
}               
                
/*
 * Function:
 *  bcm_robo_trunk_egress_set
 * Description:
 *  Set switching only to indicated ports from given trunk.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  tid - Trunk Id.  Negative trunk id means set all trunks.
 *  pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_egress_set(int unit, bcm_trunk_t tid, bcm_pbmp_t pbmp)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_egress_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
    
/*
 * Function:
 *  bcm_robo_trunk_egress_get
 * Description:
 *  Retrieve bitmap of ports for which switching is enabled for trunk.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *  tid - Trunk Id.  Negative trunk id means choose any trunk.
 *  pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_egress_get(int unit, bcm_trunk_t tid, bcm_pbmp_t *pbmp)
{
    soc_cm_debug(DK_VERBOSE, 
                "BCM API : bcm_robo_trunk_egress_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
    

/*
 * Function:
 *      bcm_robo_trunk_override_ucast_set
 * Description:
 *      Set the trunk override over UC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_ucast_set(int unit, bcm_port_t port,
                                    bcm_trunk_t tid, int modid, int enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_ucast_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                        
/*
 * Function:
 *      bcm_robo_trunk_override_ucast_get
 * Description:
 *      Get the trunk override over UC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_ucast_get(int unit, bcm_port_t port,
                                    bcm_trunk_t tid, int modid, int *enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_ucast_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                        
/*
 * Function:
 *      bcm_trunk_override_mcast_set
 * Description:
 *      Set the trunk override over MC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_mcast_set(int unit, bcm_port_t port,
                                    bcm_trunk_t tid, int idx, int enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_mcast_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                        
/*
 * Function:
 *      bcm_trunk_override_mcast_get
 * Description:
 *      Get the trunk override over MC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_mcast_get(int unit, bcm_port_t port,
                                    bcm_trunk_t tid, int idx, int *enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_mcast_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                        
/*
 * Function:
 *      bcm_robo_trunk_override_ipmc_set
 * Description:
 *      Set the trunk override over IPMC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_ipmc_set(int unit, bcm_port_t port,
                                bcm_trunk_t tid, int idx, int enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_ipmc_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                       
/*
 * Function:
 *      bcm_robo_trunk_override_ipmc_get
 * Description:
 *      Get the trunk override over IPMC.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_ipmc_get(int unit, bcm_port_t port,
                                bcm_trunk_t tid, int idx, int *enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_ipmc_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                       
/*
 * Function:
 *      bcm_robo_trunk_override_vlan_set
 * Description:
 *      Set the trunk override over VLAN.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_vlan_set(int unit, bcm_port_t port,
                                bcm_trunk_t tid, bcm_vlan_t vid, int enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_vlan_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                       
/*
 * Function:
 *      bcm_robo_trunk_override_vlan_get
 * Description:
 *      Get the trunk override over VLAN.
 * Parameters:
 *      unit   - RoboSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_override_vlan_get(int unit, bcm_port_t port,
                                bcm_trunk_t tid, bcm_vlan_t vid, int *enable)
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_override_vlan_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                                       

/*
 * Function:
 *      bcm_robo_trunk_pool_set
 * Description:
 *      Set trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - RoboSwitch PCI device unit number (driver internal).
 *      port    - Port number, -1 to all ports.
 *      tid     - Trunk id.
 *      size    - Trunk pool size.
 *      weights - Weights for each port, all 0 means weighted fair.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_pool_set(int unit, bcm_port_t port, bcm_trunk_t tid,
                        int size, const int weights[BCM_TRUNK_MAX_PORTCNT])
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_pool_set().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                              
/*
 * Function:
 *      bcm_trunk_pool_get
 * Description:
 *      Get trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - RoboSwitch PCI device unit number (driver internal).
 *      port    - Port number.
 *      tid     - Trunk id.
 *      size    - (OUT) Trunk pool size.
 *      weights - (OUT) Weights (total count) for each port.
 * Returns:
 *      BCM_E_xxxx
 */
int 
bcm_robo_trunk_pool_get(int unit, bcm_port_t port, bcm_trunk_t tid,
                        int *size, int weights[BCM_TRUNK_MAX_PORTCNT])
{
    soc_cm_debug(DK_VERBOSE, 
            "BCM API : bcm_robo_trunk_pool_get().. unavailable\n");
    return BCM_E_UNAVAIL;
}               
                              
int
bcm_robo_trunk_find(int unit, bcm_module_t modid, 
                        bcm_port_t port, bcm_trunk_t *tid)
{
    int idx = 0;
    bcm_trunk_add_info_t t_data;
    bcm_trunk_t  tgid;
    int  id; 
    int rv;
    bcm_module_t hw_mod;
    bcm_port_t   hw_port;

    TRUNK_INIT(unit);

    if (tid == NULL) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));

        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    modid, port,
                                    &hw_mod, &hw_port));

        if (!SOC_MODID_ADDRESSABLE(unit, hw_mod)) {
            return BCM_E_BADID;
        }

        if (!SOC_PORT_VALID(unit, hw_port)) { 
            return BCM_E_PORT; 
        }
    }

    for (idx = 0; idx < TRUNK_CNTL(unit).ngroups; idx++) {
        if (TRUNK_INFO(unit, idx).tid == BCM_TRUNK_INVALID) {
                continue;
        }
             
        sal_memset(&t_data, 0, sizeof(bcm_trunk_add_info_t));
        rv = bcm_trunk_get(unit, idx, &t_data);

        if (BCM_SUCCESS(rv)) {
            int i = 0;

            for (i = 0; i < t_data.num_ports; i++) {
                if (t_data.tm[i] == modid && t_data.tp[i] == port) {
                    *tid = idx;
                    return BCM_E_NONE;
                }
            }
        }
    }
    return BCM_E_NOT_FOUND;
}
