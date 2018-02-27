/*
 * $Id: l2x.h 1.52.2.2 Broadcom SDK $
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
 * File:        l2x.h
 * Purpose:     Draco L2X hardware table manipulation support
 */

#ifndef _L2X_H_
#define _L2X_H_

#include <sal/core/time.h>
#include <shared/avl.h>
#include <shared/bitop.h>
#include <soc/macipadr.h>
#include <soc/hash.h>

#ifdef BCM_XGS_SWITCH_SUPPORT

extern int soc_l2x_attach(int unit);
extern int soc_l2x_detach(int unit);
extern int soc_l2x_init(int unit);

extern int soc_l2x_entry_valid(int unit, l2x_entry_t *entry);

extern int soc_l2x_entry_compare_key(void *user_data,
                                     shr_avl_datum_t *datum1,
                                     shr_avl_datum_t *datum2);
extern int soc_l2x_entry_compare_all(void *user_data,
                                     shr_avl_datum_t *datum1,
                                     shr_avl_datum_t *datum2);

extern int soc_l2x_entry_dump(void *user_data,
                              shr_avl_datum_t *datum,
                              void *extra_data);

/*
 * L2 hardware insert, delete, and lookup
 */

extern int soc_l2x_insert(int unit, l2x_entry_t *entry);
extern int soc_l2x_delete(int unit, l2x_entry_t *entry);
extern int soc_l2x_delete_all(int unit, int static_too);
extern int soc_l2x_port_age(int unit, soc_reg_t reg0, soc_reg_t reg1);
extern int soc_l2x_lookup(int unit,
                          l2x_entry_t *key, l2x_entry_t *result,
                          int *index);

#define SOC_L2X_INC_STATIC          0x00000001
#define SOC_L2X_NO_CALLBACKS        0x00000002
#define SOC_L2X_EXT_MEM             0x00000004 

/* Modes by which delete can take place */
#define SOC_L2X_NO_DEL                0  
#define SOC_L2X_PORTMOD_DEL           1
#define SOC_L2X_VLAN_DEL              2
#define SOC_L2X_MAC_DEL               3
#define SOC_L2X_PORTMOD_VLAN_DEL      4
#define SOC_L2X_ALL_DEL               5
#define SOC_L2X_TRUNK_DEL             6
#define SOC_L2X_VFI_DEL               7
#define SOC_L2X_TRUNK_VLAN_DEL        8


extern int soc_l2x_sync_delete(int unit, uint32 *del_entry, int index, uint32 flags);
extern int _soc_l2x_sync_delete_by(int, uint32, uint32, uint16, uint32, int,
                                   uint32, uint32);
extern int _soc_l2x_sync_replace(int unit, l2x_entry_t *l2x_match_data,
                                 l2x_entry_t *l2x_match_mask, uint32 flags);
extern int soc_l2x_sync_delete_by_port(int unit, int mod, 
                                       soc_port_t port, uint32 flags);
extern int soc_l2x_sync_delete_by_trunk(int unit, 
                                        int tid, uint32 flags);
extern int soc_er_l2x_entries_external(int unit);
extern int soc_er_l2x_entries_internal(int unit);
extern int soc_er_l2x_entries_overflow(int unit);

/*
 * Registration to receive inserts, deletes, and updates.
 *	For inserts, entry_del is NULL.
 *	For deletes, entry_ins is NULL.
 *	For updates, neither is NULL.
 */

typedef void (*soc_l2x_cb_fn)(int unit,
                              l2x_entry_t *entry_del,
                              l2x_entry_t *entry_add,
                              void *fn_data);

extern int soc_l2x_register(int unit, soc_l2x_cb_fn fn, void *fn_data);
extern int soc_l2x_unregister(int unit, soc_l2x_cb_fn fn, void *fn_data);

#define soc_l2x_unregister_all(unit) \
        soc_l2x_unregister((unit), NULL, NULL)

extern void soc_l2x_callback(int unit,
                             l2x_entry_t *entry_del,
                             l2x_entry_t *entry_add);


/*
 * L2 miscellaneous functions
 */

extern int soc_l2x_freeze(int unit);
extern int soc_l2x_is_frozen(int unit, int *frozen);
extern int soc_l2x_thaw(int unit);
extern int soc_l2x_entries(int unit);	/* Warning: very slow */
extern int soc_l2x_hash(int unit, l2x_entry_t *entry);

extern int soc_l2x_frozen_cml_set(int unit, soc_port_t port, int cml, int cml_move,
				  int *repl_cml, int *repl_cml_move);
extern int soc_l2x_frozen_cml_get(int unit, soc_port_t port, int *cml, int *cml_move);
extern int _soc_l2x_frozen_cml_save(int unit);
extern int _soc_l2x_frozen_cml_restore(int unit);
extern void soc_l2x_cml_vp_bitmap_set(int unit, SHR_BITDCL *vp_bitmap);

/*
 * L2X Thread
 */

extern int soc_l2x_start(int unit, uint32 flags, sal_usecs_t interval);
extern int soc_l2x_running(int unit, uint32 *flags, sal_usecs_t *interval);
extern int soc_l2x_stop(int unit);

/*
 * L2 software-based sanity routines
 */

#define SOC_L2X_BUCKET_SIZE	8
extern int soc_l2x_software_hash(int unit, int hash_select, 
                                 l2x_entry_t *entry);

/*
 * Debugging features - for diagnostic purposes only
 */

extern void soc_l2x_key_dump(int unit, char *pfx,
                             l2x_entry_t *entry, char *sfx);

#endif	/* BCM_XGS_SWITCH_SUPPORT */

#ifdef	BCM_FIREBOLT_SUPPORT

extern int soc_fb_l2x_bank_insert(int unit, uint8 banks, l2x_entry_t *entry);
extern int soc_fb_l2x_bank_delete(int unit, uint8 banks, l2x_entry_t *entry);
extern int soc_fb_l2x_bank_lookup(int unit, uint8 banks, l2x_entry_t *key,
                                  l2x_entry_t *result, int *index_ptr);

extern int soc_fb_l2x_insert(int unit, l2x_entry_t *entry) ;
extern int soc_fb_l2x_delete(int unit, l2x_entry_t *entry);
extern int soc_fb_l2x_lookup(int unit, l2x_entry_t *key,
                             l2x_entry_t *result, int *index_ptr);
extern int soc_fb_l2x_delete_all(int unit);

/*
 * We must use SOC_MEM_INFO here, because we may redefine the
 * index_max due to configuration, but the SCHAN msg uses the maximum size.
 */
#define SOC_L2X_OP_FAIL_POS(unit) \
        ((_shr_popcount(SOC_MEM_INFO(unit, L2Xm).index_max) + \
            soc_mem_entry_bits(unit, L2Xm)) %32)

#endif	/* BCM_FIREBOLT_SUPPORT */


#ifdef	BCM_EASYRIDER_SUPPORT

#define SOC_L2X_MEM_LOCK(unit)\
    if (soc_feature(unit, soc_feature_l2_multiple)) {\
        soc_mem_lock(unit, L2_ENTRY_INTERNALm);\
        soc_mem_lock(unit, L2_ENTRY_EXTERNALm);\
        soc_mem_lock(unit, L2_ENTRY_OVERFLOWm);\
    } else { soc_mem_lock(unit, L2Xm); }


#define SOC_L2X_MEM_UNLOCK(unit)\
    if (soc_feature(unit, soc_feature_l2_multiple)) {\
        soc_mem_unlock(unit, L2_ENTRY_OVERFLOWm);\
        soc_mem_unlock(unit, L2_ENTRY_EXTERNALm);\
        soc_mem_unlock(unit, L2_ENTRY_INTERNALm);\
    } else { soc_mem_unlock(unit, L2Xm); }

#else 

#define SOC_L2X_MEM_LOCK(unit)\
          soc_mem_lock(unit, L2Xm); 
        
#define SOC_L2X_MEM_UNLOCK(unit)\
          soc_mem_unlock(unit, L2Xm); 
#endif

#ifdef	BCM_EASYRIDER_SUPPORT
#define SOC_L2EXT_BUCKET_SIZE	4
#define SOC_L2_BUCKET_SUMS     20
extern int soc_er_l2x_lookup(int unit, l2_entry_internal_entry_t *key,
                             l2_entry_internal_entry_t *result,
                             int *index_ptr, soc_mem_t *mem);
extern int soc_er_l2x_delete_all(int unit);

/* Handy translations */
#define soc_er_l2x_insert(unit, entry) \
        soc_mem_insert((unit),  L2_ENTRY_INTERNALm, COPYNO_ALL, (entry))
#define soc_er_l2x_delete(unit, entry) \
        soc_mem_delete((unit),  L2_ENTRY_INTERNALm, COPYNO_ALL, (entry))


#define SOC_ER_L2_PORTMOD_DEL           0
#define SOC_ER_L2_VLAN_DEL              1
#define SOC_ER_L2_PORTMOD_VLAN_DEL      2
#define SOC_ER_L2_ALL_DEL               3
#define SOC_ER_L2_PORTMOD_REPL          4
#define SOC_ER_L2_VLAN_REPL             5
#define SOC_ER_L2_PORTMOD_VLAN_REPL     6
#define SOC_ER_L2_NORMAL_AGE            7
#define SOC_ER_L2_DELETE_BY_MASK        0x7

extern int soc_er_l2x_delete_by(int unit, uint32 modid, uint32 tgid_port,
                                uint32 vlan, uint32 repl_modid,
                                uint32 repl_tgid_port, uint32 op,
                                int static_too);

#define SOC_ER_L2_CHUNKS_DEFAULT      256

typedef int (*soc_er_l2_cb_fn)(int unit, uint32 *entry, int index,
                               soc_mem_t tmem, void *data);
extern int soc_er_l2_traverse(int unit,
                              soc_er_l2_cb_fn l2_op_fn, void *fn_data);


#endif	/* BCM_EASYRIDER_SUPPORT */

#ifdef  BCM_ROBO_SUPPORT

/* define all ROBO chips l2 bucket size */
#define ROBO_5324_L2_BUCKET_SIZE    2
#define ROBO_5348_L2_BUCKET_SIZE    1
#define ROBO_53242_L2_BUCKET_SIZE   2
#define ROBO_53280_L2_BUCKET_SIZE   4
#define ROBO_5396_L2_BUCKET_SIZE    2
#define ROBO_5389_L2_BUCKET_SIZE    2
#define ROBO_5398_L2_BUCKET_SIZE    1
#define ROBO_5395_L2_BUCKET_SIZE    4
#define ROBO_53115_L2_BUCKET_SIZE   4
#define ROBO_53118_L2_BUCKET_SIZE   4
#define ROBO_53101_L2_BUCKET_SIZE   4
#define ROBO_53125_L2_BUCKET_SIZE   4
#define ROBO_53128_L2_BUCKET_SIZE   4

#define ROBO_MAX_L2_BUCKET_SIZE     4

#endif	/* BCM_ROBO_SUPPORT */

/*
 * L2 Mod Thread
 */

typedef enum l2x_mode_e {
    L2MODE_POLL,
    L2MODE_FIFO
} l2x_mode_t;

#define L2MOD_PASS_CPU  0x01

extern int soc_l2mod_start(int unit, uint32 flags, sal_usecs_t interval);
extern int soc_l2mod_running(int unit, uint32 *flags, sal_usecs_t *interval);
extern int soc_l2mod_stop(int unit);

#endif	/* !_L2X_H_ */
