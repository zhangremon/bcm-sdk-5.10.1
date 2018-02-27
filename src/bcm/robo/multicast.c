/*
 * $Id: multicast.c 1.34.2.5 Broadcom SDK $
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
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/multicast.h>
#include <bcm/subport.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/subport.h>
#include <bcm_int/robo/mcast.h>

#include <bcm_int/common/multicast.h>

#ifdef  BCM_TB_SUPPORT
/* Replication list of (mc_index, port) pairs. */
static _bcm_robo_mcast_repl_t   *robo_mcast_repl_config[BCM_LOCAL_UNITS_MAX];
/* Replicated vlan(s) */
static _bcm_robo_mcast_repl_vlan_t *robo_mcast_repl_vid[BCM_LOCAL_UNITS_MAX];

static void
_mcast_repl_used_set(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_t *cfg;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
    int offset;

    offset = ((port << _TB_MCAST_REPL_INDEX_PORT_OFFSET) |\
                (mc_index << _TB_MCAST_REPL_INDEX_MGID_OFFSET));
    cfg = robo_mcast_repl_config[unit];
    repl_index = &cfg->repl_index[offset];

    SHR_BITSET(repl_index->repl_vport_id, bit);
}


static void
_mcast_repl_used_clr(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_t *cfg;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
    int offset;

    offset = ((port << _TB_MCAST_REPL_INDEX_PORT_OFFSET) |\
                (mc_index << _TB_MCAST_REPL_INDEX_MGID_OFFSET));
    cfg = robo_mcast_repl_config[unit];
    repl_index = &cfg->repl_index[offset];

    SHR_BITCLR(repl_index->repl_vport_id, bit);
}

static int
_mcast_repl_used_isset(int unit, int mc_index, bcm_port_t port, int bit)
{
    _bcm_robo_mcast_repl_t *cfg;
    _bcm_robo_mcast_repl_vport_id_t *repl_index;
    int offset;

    offset = ((port << _TB_MCAST_REPL_INDEX_PORT_OFFSET) |\
                (mc_index << _TB_MCAST_REPL_INDEX_MGID_OFFSET));
    cfg = robo_mcast_repl_config[unit];
    repl_index = &cfg->repl_index[offset];

    return (SHR_BITGET(repl_index->repl_vport_id, bit));
}

static void
_mcast_repl_vid_set(int unit, bcm_port_t port, int vport_id, bcm_vlan_t vid)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count !=0) {
        /* Debug purpose */
        soc_cm_debug(DK_ERR, \
            "ERROR: replication vlan %d(port=%d, vport=%d) be overwritten\n", \
            repl_vid_idx->vid,port,vport_id);
    }

    repl_vid_idx->vid = vid;
}

static bcm_vlan_t
_mcast_repl_vid_get(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    return (repl_vid_idx->vid);
}

static void
_mcast_repl_vid_cnt_inc(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0xffff) {
        /* Debug purpose */
        soc_cm_debug(DK_ERR, \
            "ERROR: replication vlan %d(port=%d, vport=%d), increase underflow\n", \
            repl_vid_idx->vid,port,vport_id);
    }

    repl_vid_idx->count++;
}

static void
_mcast_repl_vid_cnt_dec(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0) {
        /* Debug purpose */
        soc_cm_debug(DK_ERR, \
            "ERROR: replication vlan %d(port=%d, vport=%d), decrease underflow\n", \
            repl_vid_idx->vid,port,vport_id);
    }

    repl_vid_idx->count--;

}

static int
_mcast_repl_vid_cnt_is_zero(int unit, bcm_port_t port, int vport_id)
{
    _bcm_robo_mcast_repl_vlan_t *repl_vlan_cfg;
    _bcm_robo_mcast_repl_vid_t *repl_vid_idx;
    int offset;

    offset = ((port << _TB_MCAST_REPL_VID_PORT_OFFSET) |\
                (vport_id << _TB_MCAST_REPL_VID_VPORT_OFFSET));
    repl_vlan_cfg = robo_mcast_repl_vid[unit];
    repl_vid_idx = &repl_vlan_cfg->repl_vid[offset];

    if (repl_vid_idx->count == 0) {
        return 1;
    } else {
        return 0;
    }
}

static int
_mcast_repl_param_check(int unit, int mc_index, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    uint32 mcrep_num = 0;
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }

    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
            &mcrep_num);
    BCM_IF_ERROR_RETURN(rv);
    if ((mc_index < 0) || (mc_index >= mcrep_num)) {
        return BCM_E_BADID;
    }

    if (!robo_mcast_repl_config[unit]) {
        return BCM_E_INIT;
    } 
    if (!robo_mcast_repl_vid[unit]) {
        return BCM_E_INIT;
    } 

    return rv;
}

#define MCAST_REPL_USED_SET(u, id, p, n) _mcast_repl_used_set(u, id, p, n)
#define MCAST_REPL_USED_CLR(u, id, p, n) _mcast_repl_used_clr(u, id, p, n)
#define MCAST_REPL_USED_ISSET(u, id, p, n) _mcast_repl_used_isset(u, id, p, n)

#define MCAST_REPL_VID_SET(u, p, vport, vid) _mcast_repl_vid_set(u, p, vport, vid)
#define MCAST_REPL_VID_GET(u, p, vport) _mcast_repl_vid_get(u, p, vport)
#define MCAST_REPL_VID_CNT_INC(u, p, vport) _mcast_repl_vid_cnt_inc(u, p, vport)
#define MCAST_REPL_VID_CNT_DEC(u, p, vport) _mcast_repl_vid_cnt_dec(u, p, vport)
#define MCAST_REPL_VID_CNT_IS_ZERO(u, p, vport) _mcast_repl_vid_cnt_is_zero(u, p, vport)
#endif  /* BCM_TB_SUPPORT */

#define ONE_MULTICAST_TYPE_IS_SET(_type) \
        (_shr_popcount((_type)) == 1)

#define MCAST_EGRESS_OP_ADD 1
#define MCAST_EGRESS_OP_DEL 2
#define MCAST_EGRESS_OP_CLR 3
#define MCAST_EGRESS_OP_GET 4
#define MCAST_EGRESS_OP_SET 5

/* _bcm_robo_multicast_egress_op()
 *  - internal function to configuring ROBO's MCAST_PBMP table 
 *
 *  Parmeter :
 *      op          : add/delete/clear/get/set
 *      mc_index    : the MCAST_PBMP table index
 *      op_value    : the value for the usage per op
 *
 *  Return :
 *
 *  Note :
 *  1. op_value must be verified before calling to this function.
 *  2. op_value for op at {add/delete} will be treate as port_id and thid id 
 *      must be validate again.
 *
 */
static int
_bcm_robo_multicast_egress_op(int unit,
                        uint32 op,
                        uint32 mc_index,
                        uint32 *op_value){

    int     rv = BCM_E_NONE;
    int     mem_cnt = 0;
    bcm_pbmp_t pbmp;
    bcm_port_t port = 0;
    marl_pbmp_entry_t  mcast_entry;

    /* pre-processing check ...*/
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }
    
    /* valid index check */
    if (mc_index >= mem_cnt) {
        return BCM_E_BADID;
    } else {
        /* check if this group is created */
        if (_bcm_robo_l2mc_id_check(unit, mc_index) == 0){
            return BCM_E_NOT_FOUND;
        }
    }

    /* valid op_value check */
    if (op_value == NULL){
        return BCM_E_INTERNAL;
    }

    /* retrieve the MCAST_PBMPm enry */
    sal_memset(&mcast_entry, 0, sizeof (marl_pbmp_entry_t));
    rv = DRV_MEM_READ(unit, DRV_MEM_MCAST, mc_index, 1, 
            (uint32 *)&mcast_entry);
    if (rv){
        soc_cm_debug(DK_ERR, 
                "%s, failed on reading MCAST_PBMP entry%d!\n",
                FUNCTION_NAME(), mc_index);
        return rv;
    }
    
    BCM_PBMP_CLEAR(pbmp);
    rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MCAST, 
            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)&mcast_entry, 
            (uint32 *)&pbmp);
    if (rv){
        soc_cm_debug(DK_ERR, 
                "%s, Failed on field get of MCAST_PBMP entry!\n",
                FUNCTION_NAME());
        return rv;
    }
    
    switch (op) {
    case MCAST_EGRESS_OP_ADD:
        port = (int)*op_value;
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PARAM;
        } else {
            BCM_PBMP_PORT_ADD(pbmp, port);
        }
        break;
    case MCAST_EGRESS_OP_DEL:
        port = (int)*op_value;
        if (!SOC_PORT_VALID(unit, port)) {
            rv = BCM_E_PARAM;
        } else {
            BCM_PBMP_PORT_REMOVE(pbmp, port);
        }
        break;
    case MCAST_EGRESS_OP_CLR:
        BCM_PBMP_CLEAR(pbmp);
        break;
    case MCAST_EGRESS_OP_GET:
        sal_memset(op_value, 0, sizeof(op_value));
        sal_memcpy(op_value, (uint32 *)&pbmp, sizeof(bcm_pbmp_t));

        return BCM_E_NONE;
        break;
    case MCAST_EGRESS_OP_SET:
        sal_memcpy((uint32 *)&pbmp, op_value, sizeof(bcm_pbmp_t));
        break;
    default:
        rv = BCM_E_INTERNAL;
    }

    if (rv == BCM_E_NONE) {
        rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MCAST, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&mcast_entry, (uint32 *)&pbmp);
        if (rv) {
            soc_cm_debug(DK_ERR, 
                    "%s,%d, Failed on field set of MCAST_PBMP entry!\n",
                    FUNCTION_NAME(), __LINE__);
        }
        rv = DRV_MEM_WRITE(unit, DRV_MEM_MCAST, mc_index, 1, 
                (uint32 *)&mcast_entry);
        
        if (rv){
            soc_cm_debug(DK_ERR, 
                    "%s, failed on writing MCAST_PBMP entry%d!\n",
                    FUNCTION_NAME(), mc_index);
            return BCM_E_INTERNAL;
        }
    }
   
    return rv;
}

/* 
 * For the HW spec. the vport in TB will be bounded with a physical port and 
 *  mcast group. That means a new vport is created with known mc_group,
 *  phy_port and also CVID through subport API. The vport's VID in TB is only
 *  port basis. That is, a vp of a port in different group must with the same 
 *  CVID configuration.
 *
 * [Important] :
 *  1. This type for TB is not implemented for HW spec. limitation TB's 
 *      subport is port basis not the port and group basis like TRX devices.
 *  2. If user wish to add a subport into a mcast group, a proer API,
 *      bcm_subport_port_add(), can appropach this request for more clear
 *      configuration(add a subport with mc_group, phy_port and CVLAN 
 *      information
 */

/*
 * Function:
 *      bcm_multicast_create
 * Purpose:
 *      Allocate a multicast group index
 * Parameters:
 *      unit       - (IN)   Device Number
 *      flags      - (IN)   BCM_MULTICAST_*
 *      group      - (OUT)  Group ID
 * Returns:
 *      BCM_E_XXX
 * Note :
 */
int
bcm_robo_multicast_create(int unit,
                         uint32 flags,
                         bcm_multicast_t *group)
{
    int     type, mc_index = 0, rv = BCM_E_NONE;
    int     mem_cnt = 0, mcrep_num = 0;

    /* valid type check : return BCM_E_PARAM for 
     *  1. no type is indicated.
     *  2. one more type been indicated.
     */
    type = flags & BCM_MULTICAST_TYPE_MASK;
    if (!ONE_MULTICAST_TYPE_IS_SET(type)) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_MULTICAST_WITH_ID){
        /* The designing flow here for ROBO is created in SW only for the L2MC 
         *  table in ROBO devices (like bcm53242/5348/TB) doestn't with the 
         *  same HW design of ESW to has valid bit in each L2MC table entry.
         */
        mc_index = _BCM_MULTICAST_ID_GET(*group);
    }

    /* Pre-proceeding TB specific feature for L2MC replication */
    if (SOC_IS_TBX(unit)) {
        rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                (uint32 *)&mcrep_num);
        if (rv){
            soc_cm_debug(DK_ERR, "%s, %d, device property get problem!\n",
                    FUNCTION_NAME(), __LINE__);
            return BCM_E_INTERNAL;
        }
        assert(mcrep_num > 0);      /* assert for TB devices */
        
        if (flags & BCM_MULTICAST_WITH_ID){
        
            /* Reasign the type to subport if user requests to create group but 
             *  assigned the id which is reserved for multicast replication.
             */
            if (type == BCM_MULTICAST_TYPE_L2){
                if ((mc_index >= 0) && (mc_index < mcrep_num)){
                    type = BCM_MULTICAST_TYPE_SUBPORT;
                } 
            }
        }
    }
    
    if (type == BCM_MULTICAST_TYPE_L2){
        /* support for the switch chip which has L2MC_PBMP table only */
        rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
        if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
            return BCM_E_UNAVAIL;
        }
        
        /* user can request id 0 to l2mc_size without the limit to indicate 
         *  the Mcast Replication group seciton (0-255) for creat process.
         * Once user request SW to choose a free L2MC_ID for L2 group create.
         *  the SW design in the internal routine _bcm_robo_l2mc_free_index()
         *  will avoid index in the Mcast Replication seciton(0-255).
         */
        if (flags & BCM_MULTICAST_WITH_ID){
            if ((mc_index < 0) || (mc_index >= mem_cnt)) {
                return BCM_E_BADID;
            } else {
                /* check if ID is not free */
                if (_bcm_robo_l2mc_id_check(unit, mc_index)){
                    /* id is not free for group create */
                    return BCM_E_EXISTS;
                }
            }
            
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_id_alloc(unit, mc_index));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_free_index(
                    unit, _BCM_MULTICAST_TYPE_L2, &mc_index));
        } 
        _BCM_MULTICAST_GROUP_SET(
                *group, _BCM_MULTICAST_TYPE_L2, mc_index);
    } else if (type == BCM_MULTICAST_TYPE_SUBPORT){

        if (!soc_feature(unit, soc_feature_subport)) {
            return BCM_E_UNAVAIL;
        }

        if (!SOC_IS_TBX(unit)) {
            return BCM_E_UNAVAIL;
        }

        /* TB's Mcast Replication group_id is limited between 0-0x1cff.
         * For the HW specific design on TB, this group_id can be explained
         *  in detail as :
         *  - group_id = Port_id(bit12-bit8) | L2MC_ID(bit7-bit0)
         *
         *  That is, the creation request with subport type must provide the 
         *  group id in system basis format.(port+l2mc_id)
         */
        
        if (flags & BCM_MULTICAST_WITH_ID){
            if ((mc_index < 0) ||(mc_index >= mcrep_num)) {
                return BCM_E_BADID;
            } else {
                /* check if ID is not free */
                if (_bcm_robo_l2mc_id_check(unit, mc_index)){
                    /* id is not fee for group create */
                    return BCM_E_EXISTS;
                }
            }
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_l2mc_id_alloc(unit, mc_index));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_free_index(unit, 
                        _BCM_MULTICAST_TYPE_SUBPORT, &mc_index));
            _BCM_MULTICAST_GROUP_SET(
                    *group, _BCM_MULTICAST_TYPE_SUBPORT, mc_index);        
        }
        
    } else { 
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_destroy
 * Purpose:
 *      Free a multicast group index
 * Parameters:
 *      unit       - (IN) Device Number
 *      group      - (IN) Group ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_destroy(int unit, bcm_multicast_t group)
{

    int type, mc_index, rv = BCM_E_NONE;
    int mem_cnt = 0;
    marl_pbmp_entry_t  mcast_entry;
    int mcrep_num =0;
    
    type = _BCM_MULTICAST_TYPE_GET(group);

    if (type == _BCM_MULTICAST_TYPE_L2) {
        type = BCM_MULTICAST_TYPE_L2;
    } else if (type == _BCM_MULTICAST_TYPE_SUBPORT) {
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* type in after this line is asserted at BCM_MULTICAST_TYPE_L2 or 
     *  BCM_MULTICAST_TYPE_SUBPORT only.
     */

    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (SOC_IS_TBX(unit)) {
        /* enable multicast vport replication feature if any mcast replication 
         *  group(0-255) created.
         */
        rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                (uint32 *)&mcrep_num);
        if (rv){
            soc_cm_debug(DK_ERR, "%s, %d, device property get problem!\n",
                    FUNCTION_NAME(), __LINE__);
            return BCM_E_INTERNAL;
        }
    }

    sal_memset(&mcast_entry, 0, sizeof (marl_pbmp_entry_t));
    if (type == BCM_MULTICAST_TYPE_L2){
        
        /* support for the switch chip which has L2MC_PBMP table only */
        rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
        if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
            return BCM_E_UNAVAIL;
        }
                
        if ((mc_index < 0) || (mc_index >= mem_cnt )) {
            return BCM_E_BADID;
        }
    } else {    /* BCM_MULTICAST_TYPE_SUBPORT */
        if (!soc_feature(unit, soc_feature_subport)) {
            return BCM_E_UNAVAIL;
        }

        if ((mc_index < 0) || (mc_index >= mcrep_num )) {
            return BCM_E_BADID;
        }
    }
    
    /* check if the id is existed */
    if (_bcm_robo_l2mc_id_check(unit, mc_index) == 0){
        return BCM_E_NOT_FOUND;
    }
    
    /* Clear the HW entry */
    rv = DRV_MEM_WRITE(unit, DRV_MEM_MCAST, mc_index, 1, 
            (uint32 *)&mcast_entry);
    if (rv){
        soc_cm_debug(DK_L2TABLE, 
                "%s: faield on get the mcast_bmp table\n", FUNCTION_NAME());
        return rv;
    }
    
    /* Free the L2MC index */
    BCM_IF_ERROR_RETURN(_bcm_robo_l2mc_id_free(unit, mc_index));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_l3_encap_get
 * Purpose:
 *      Get the Encap ID for L3.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      group - (IN) Multicast group ID.
 *      port  - (IN) Physical port.
 *      intf  - (IN) L3 interface ID.
 *      encap_id - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_multicast_l3_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_if_t intf, 
                               bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_multicast_l2_encap_get
 * Purpose:
 *      Get the Encap ID for L2.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      group    - (IN) Multicast group ID.
 *      port     - (IN) Physical port.
 *      vlan     - (IN) Vlan.
 *      encap_id - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_robo_multicast_l2_encap_get(int unit, 
                               bcm_multicast_t group, 
                               bcm_gport_t port, 
                               bcm_vlan_t vlan, 
                               bcm_if_t *encap_id)
{
    /* Encap ID is not used for L2 in ROBO(the same design with ESW) */
    *encap_id = BCM_IF_INVALID;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_vpls_encap_get
 * Purpose:
 *      Get the Encap ID for a MPLS port.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      group        - (IN) Multicast group ID.
 *      port         - (IN) Physical port.
 *      mpls_port_id - (IN) MPLS port ID.
 *      encap_id     - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_vpls_encap_get(int unit, 
                                 bcm_multicast_t group,
                                 bcm_gport_t port,
                                 bcm_gport_t mpls_port_id,
                                 bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_subport_encap_get
 * Purpose:
 *      Get the Encap ID for a subport.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      subport   - (IN) Subport ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 * In ESW on TR serial chips, the subport replication is implemented 
 *  through EGR_L3_INTF table. A new l3_index of EGR_L3_INTF table will 
 *  allocated for every individual active subport. A subport can 
 *  individually configurred the packet VID can boundled into any valid 
 *  phsicall port. Thus the encap_id for ESW will return a l3_index for 
 *  a specific valid subport. And this encap_id will be used through 
 *  _bcm_tr_ipmc_egress_intf_add()
 *
 *  In Robo on TB chip, SW designed the index system to represent an 
 *  individual subport(system based id). For the HW designing Arch. the 
 *  subport for replication is per mcast group per physical port. 
 *  This system based id is valid between 0~0x1cfff. Any possible mcast 
 *  replication must be implemented through L2MC table. 
 */
int
bcm_robo_multicast_subport_encap_get(int unit,
                                    bcm_multicast_t group,
                                    bcm_gport_t port,
                                    bcm_gport_t subport,
                                    bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_mim_encap_get
 * Purpose:
 *      Get the Encap ID for MiM.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      mim_port  - (IN) MiM port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_mim_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                bcm_gport_t mim_port, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_wlan_encap_get
 * Purpose:
 *      Get the Encap ID for WLAN.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      wlan_port - (IN) WLAN port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_wlan_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                 bcm_gport_t wlan_port, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_vlan_encap_get
 * Purpose:
 *      Get the Encap ID for VLAN port (L2 logical port).
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      vlan_port - (IN) VLAN port ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_multicast_vlan_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                 bcm_gport_t vlan_port_id, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_multicast_egress_add
 * Purpose:
 *      Add a GPORT to the replication list
 *      for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 *      port      - (IN) GPORT Identifier
 *      encap_id  - (IN) Encap ID.
 * Returns:
 *      BCM_E_XXX
 *
 * Note :
 *  1. The L2 type in this API can serve the chip within Mcast group table
 *      on indicating group member port bitmap relationship. And the encap_id
 *      in this type will be ignored.
 *      (i.e. TB, 5348, 5347 can support this type)
 *      
 */
int
bcm_robo_multicast_egress_add(int unit, 
                             bcm_multicast_t group, 
                             bcm_gport_t port,
                             bcm_if_t encap_id)
{
    int     type, mc_index, port_in;
    
    type = _BCM_MULTICAST_TYPE_GET(group);
    
    if (type == _BCM_MULTICAST_TYPE_SUBPORT){
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* port_id check */
    BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port_in));
    
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (type == BCM_MULTICAST_TYPE_L2){

        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_ADD, mc_index, (uint32 *)&port_in));
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_egress_delete
 * Purpose:
 *      Delete GPORT from the replication list
 *      for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 *      port      - (IN) GPORT Identifier
 *      encap_id  - (IN) Encap ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_egress_delete(int unit, 
                                bcm_multicast_t group, 
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    int     type, mc_index, port_in;
    
    /* two type supported in this API. 
     *  - BCM_MULTICAST_TYPE_L2: remove physicall port from a L2MC group.
     *  - BCM_MULTICAST_TYPE_SUBPORT: remove existed subport from L2MC group. 
     */
    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_SUBPORT){
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* port_id check */
    BCM_IF_ERROR_RETURN(
            _bcm_robo_port_gport_validate(unit, port, &port_in));
    
    mc_index = _BCM_MULTICAST_ID_GET(group);

    if (type == BCM_MULTICAST_TYPE_L2){

        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_DEL, mc_index, (uint32 *)&port_in));
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_egress_delete_all
 * Purpose:
 *      Delete all replications for the specified multicast index.
 * Parameters:
 *      unit      - (IN) Device Number
 *      group     - (IN) Multicast group ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_egress_delete_all(int unit, bcm_multicast_t group)
{
    int     type, mc_index;
    
    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_SUBPORT){
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (type == BCM_MULTICAST_TYPE_L2){
        uint32  op_value = 0;
        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                MCAST_EGRESS_OP_CLR, mc_index, &op_value));
    } else {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*  
 * Function:
 *      bcm_multicast_egress_set
 * Purpose:
 *      Assign the complete set of egress GPORTs in the
 *      replication list for the specified multicast index.
 * Parameters:
 *      unit       - (IN) Device Number
 *      group      - (IN) Multicast group ID
 *      port_count - (IN) Number of ports in replication list
 *      port_array - (IN) List of GPORT Identifiers
 *      encap_id_array - (IN) List of encap identifiers
 * Returns:
 *      BCM_E_XXX
 */     
int     
bcm_robo_multicast_egress_set(int unit,
                             bcm_multicast_t group,
                             int port_count,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array)
{
    int     i, type, mc_index, rv = BCM_E_NONE;
    int     port_in = BCM_GPORT_INVALID;

    bcm_port_t *local_port_array = NULL;
    bcm_pbmp_t l2_pbmp;

    if (port_count < 0) {
        return BCM_E_PARAM;
    } else if (port_count == 0) {
        /* Nothing to do */
        return BCM_E_NONE;
    } else {    /* port_count > 0 */
        if (NULL == port_array) {
            return BCM_E_PARAM;
        }
    }

    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_SUBPORT){
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    
    /* Convert GPORT array into local port numbers */
    local_port_array = sal_alloc(sizeof(bcm_port_t) * port_count, 
            "local_port array");

    if (local_port_array == NULL) {
        return BCM_E_MEMORY;
    }
    for (i = 0; i < port_count ; i++) {
        port_in = BCM_GPORT_INVALID;
        rv = _bcm_robo_port_gport_validate(
                unit, port_array[i], &port_in);
        if (BCM_FAILURE(rv) || (port_in == BCM_GPORT_INVALID)){
            sal_free(local_port_array);
            return (BCM_E_PARAM);   /* esw's return is BCM_E_PORT */
        } else {
            local_port_array[i] = port_in;
        }
    }

    BCM_PBMP_CLEAR(l2_pbmp);
    for (i = 0; i < port_count; i++) {
        BCM_PBMP_PORT_ADD(l2_pbmp, local_port_array[i]);
    }
    sal_free(local_port_array);

    /* retrieve the l2mc_index */
    mc_index = _BCM_MULTICAST_ID_GET(group);
    
    if (type == BCM_MULTICAST_TYPE_L2){

        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                        MCAST_EGRESS_OP_SET, mc_index, (uint32 *)&l2_pbmp));
    } else {
        return BCM_E_UNAVAIL;
    }
    
    return BCM_E_NONE;
}   

/*
 * Function:
 *      bcm_multicast_egress_get
 * Purpose:
 *      Retrieve a set of egress multicast GPORTs in the
 *      replication list for the specified multicast index.
 * Parameters: 
 *      unit           - (IN) Device Number
 *      mc_index       - (IN) Multicast index
 *      port_max       - (IN) Number of entries in "port_array"
 *      port_array     - (OUT) List of ports
 *      encap_id_array - (OUT) List of encap identifiers
 *      port_count     - (OUT) Actual number of ports returned
 * Returns:
 *      BCM_E_XXX
 */
int     
bcm_robo_multicast_egress_get(int unit,
                             bcm_multicast_t group,
                             int port_max,
                             bcm_gport_t *port_array, 
                             bcm_if_t *encap_id_array, 
                             int *port_count)
{
    int     type, i, mc_index;
    bcm_pbmp_t l2_pbmp;
    bcm_port_t port_iter = 0;

    /* port_array and encap_id_array are allowed to be NULL */
    if ((NULL == port_count) || (port_max < 0) ){
        return BCM_E_PARAM;
    }

    /* If port_max = 0, port_array and encap_id_array must be NULL */
    if (port_max == 0) {
        if ((NULL != port_array) || (NULL != encap_id_array)) {
            return BCM_E_PARAM;
        }
    }

    type = _BCM_MULTICAST_TYPE_GET(group);

    /* retrieve BCM type from TB's specific type */
    if (type == _BCM_MULTICAST_TYPE_SUBPORT){
        type = BCM_MULTICAST_TYPE_SUBPORT;
    } else if (type == _BCM_MULTICAST_TYPE_L2){
        type = BCM_MULTICAST_TYPE_L2;
    } else {
        /* ROBO chips currently support no other type */
        return BCM_E_PARAM;
    }
    /* type in after this line is asserted at BCM_MULTICAST_TYPE_L2 or 
     *  BCM_MULTICAST_TYPE_SUBPORT only.
     */
    
    /* retrieve the l2mc_index */
    mc_index = _BCM_MULTICAST_ID_GET(group);

    if (type == BCM_MULTICAST_TYPE_L2){

        BCM_PBMP_CLEAR(l2_pbmp);
        BCM_IF_ERROR_RETURN(_bcm_robo_multicast_egress_op(unit,
                    MCAST_EGRESS_OP_GET, mc_index, (uint32 *)&l2_pbmp));

        i = 0;
        PBMP_ITER(l2_pbmp, port_iter) {
            if (i >= port_max) {
                break;
            }
            BCM_IF_ERROR_RETURN(bcm_port_gport_get(
                    unit, port_iter, (port_array + i)));
            i++;
        }
        *port_count = i;
        
    } else {    /* BCM_MULTICAST_TYPE_SUBPORT */
        /* to get the port list and encap_id(for vport) list seems can be 
         *  implemented without impact. Considering the matched get/set API
         *  design, here we still leave this section to be unavailable.
         */
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}   

/*
 * Function:
 *      bcm_robo_multicast_init
 * Purpose:
 *      Initialize the multicast module.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_init(int unit)
{
#ifdef  BCM_TB_SUPPORT
    int len =0;
    int     ctrl_val = 0;
    uint32  ctrl_cnt = 0, ctrl_type = 0;
    int     rv = BCM_E_NONE;
    
    if (SOC_IS_TBX(unit)) {
        if (robo_mcast_repl_config[unit] == NULL){
            len = sizeof(_bcm_robo_mcast_repl_t);
            robo_mcast_repl_config[unit] = sal_alloc(len, "Mcast replication");
            sal_memset(robo_mcast_repl_config[unit], 0, len);
            robo_mcast_repl_config[unit]->mcast_repl_enable = FALSE;
        }
    
        if (robo_mcast_repl_vid[unit] == NULL){
            len = sizeof(_bcm_robo_mcast_repl_vlan_t);
            robo_mcast_repl_vid[unit] = 
                    sal_alloc(len, "Mcast replication vids");
            if (robo_mcast_repl_vid[unit] == NULL) {
                sal_free(robo_mcast_repl_config[unit]);
                robo_mcast_repl_config[unit] = NULL;
            }
            sal_memset(robo_mcast_repl_vid[unit], 0, len);
        }

        /* disable vport replication */
        ctrl_cnt = 1;
        ctrl_type = DRV_DEV_CTRL_MCASTREP;
        ctrl_val = FALSE;
        rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, &ctrl_val);
        BCM_IF_ERROR_RETURN(rv);

        /* Delete all existing subports configurations */
        rv = DRV_MEM_CLEAR(unit, DRV_MEM_VPORT_VID_MAP);
        BCM_IF_ERROR_RETURN(rv);
    
        /* Delete all subport groups configurations */
        rv = DRV_MEM_CLEAR(unit, DRV_MEM_MCAST_VPORT_MAP);
        BCM_IF_ERROR_RETURN(rv);
    }
#endif  /* BCM_TB_SUPPORT */
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_detach
 * Purpose:
 *      Shut down (uninitialize) the multicast module.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_detach(int unit)
{
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)) {
        if (NULL != robo_mcast_repl_config[unit]) {
            sal_free(robo_mcast_repl_config[unit]);
        }
        robo_mcast_repl_config[unit] = NULL;
    
        if (NULL != robo_mcast_repl_vid[unit]) {
            sal_free(robo_mcast_repl_vid[unit]);
        }
        robo_mcast_repl_vid[unit] = NULL;
    }
#endif  /* BCM_TB_SUPPORT */
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_group_get
 * Purpose:
 *      Retrieve the flags associated with a mulitcast group.
 * Parameters:
 *      unit - (IN) Unit number.
 *      group - (IN) Multicast group ID
 *      flags - (OUT) BCM_MULTICAST_*
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_group_get(int unit, bcm_multicast_t group, uint32 *flags)
{
    int     rv = BCM_E_NONE, mem_cnt = 0;
    uint32  mc_index, type_flag, mc_group_count = 0;
    
    /* support for the switch chip which has L2MC_PBMP table only */
    rv = DRV_MEM_LENGTH_GET(unit, DRV_MEM_MCAST, (uint32 *)&mem_cnt);
    if (rv == BCM_E_UNAVAIL || rv == BCM_E_PARAM || mem_cnt == 0){
        return BCM_E_UNAVAIL;
    }

    *flags = 0;
    mc_index = _BCM_MULTICAST_ID_GET(group);

    if (_BCM_MULTICAST_IS_L2(group)) {
        mc_group_count = mem_cnt;
        type_flag = BCM_MULTICAST_TYPE_L2;
    } else if (_BCM_MULTICAST_IS_SUBPORT(group)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                &mc_group_count));
        type_flag = BCM_MULTICAST_TYPE_SUBPORT;
    } else {
        return BCM_E_PARAM;
    }

    /* check valid ID and report the flag */
    if (mc_index >= mc_group_count) {
        return BCM_E_BADID;
    } else {
        /* check if ID is existed */
        if (_bcm_robo_l2mc_id_check(unit, (int)mc_index) == TRUE){
            *flags = type_flag | BCM_MULTICAST_WITH_ID;
        } else {
            return BCM_E_NOT_FOUND;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_multicast_group_traverse
 * Purpose:
 *      Iterate over the defined multicast groups of the type
 *      specified in 'flags'.  If all types are desired, use
 *      MULTICAST_TYPE_MASK.
 * Parameters:
 *      unit - (IN) Unit number.
 *      trav_fn - (IN) Callback function.
 *      flags - (IN) BCM_MULTICAST_*
 *      user_data - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_robo_multicast_group_traverse(int unit,
                                 bcm_multicast_group_traverse_cb_t trav_fn, 
                                 uint32 flags, void *user_data)
{
    int mc_index;
    uint32 group_flags, flags_mask; 
    uint32 mc_group_count = 0;
    bcm_multicast_t group;

    flags_mask = BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_TYPE_SUBPORT;
    
    if (0 == (flags & flags_mask)) {
        /* No recognized multicast types to traverse */
        return BCM_E_PARAM;
    }
    flags &= flags_mask;

    if (flags & BCM_MULTICAST_TYPE_L2) {
        mc_group_count = soc_robo_mem_index_count(unit, INDEX(MARL_PBMPm));
        group_flags = BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID;

        for (mc_index = 0; mc_index < mc_group_count; mc_index++) {
            if (_bcm_robo_l2mc_id_check(unit, mc_index) == TRUE){
                _BCM_MULTICAST_GROUP_SET(group, 
                        _BCM_MULTICAST_TYPE_L2, mc_index);
                BCM_IF_ERROR_RETURN
                        ((*trav_fn)(unit, group, group_flags, user_data));
            }
        }
    }

    if (flags & BCM_MULTICAST_TYPE_SUBPORT) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_REP_NUM, 
                &mc_group_count));
        group_flags = BCM_MULTICAST_TYPE_SUBPORT | BCM_MULTICAST_WITH_ID;

        for (mc_index = 0; mc_index < mc_group_count; mc_index++) {
            if (_bcm_robo_l2mc_id_check(unit, mc_index) == TRUE){
                _BCM_MULTICAST_GROUP_SET(group, 
                        _BCM_MULTICAST_TYPE_SUBPORT, mc_index);
                BCM_IF_ERROR_RETURN
                        ((*trav_fn)(unit, group, group_flags, user_data));
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      bcm_robo_multicast_repl_set
 * Purpose:
 *      Assign set of VLANs provided to port's replication list for chosen
 *      L2 multicast group.
 * Parameters:
 *      unit     - Switch device unit number.
 *      mc_index  - The index number.
 *      port     - port to list.
 *      vlan_vec - (IN) vector of replicated VLANs common to selected ports.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_robo_multicast_repl_set(int unit, int mc_index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
#ifdef  BCM_TB_SUPPORT
    int rv = BCM_E_NONE;
    int vport_idx;
    bcm_vlan_t vid;
    int found = 0;
    int empty_slot_idx = -1;
    uint32 vport_bmp = 0;
    static _bcm_robo_mcast_repl_t old_mcast_repl_cfg;
    static _bcm_robo_mcast_repl_vlan_t old_mcast_repl_vlan;
    bcm_vlan_vector_t vlan_vec_check;
    int mcast_repl_en = FALSE;
    int len = 0;
    int ctrl_val = 0;
    uint32 ctrl_cnt = 0;
    uint32 ctrl_type = 0;
    bcm_vlan_vector_t vlan_vec_add;
    bcm_vlan_vector_t vlan_vec_del;
    
    if (SOC_IS_TBX(unit)) {
        /* Verify parameters */
        rv = _mcast_repl_param_check(unit, mc_index, port);
        BCM_IF_ERROR_RETURN(rv);

        /* Return if the set doesn't change anything */
        BCM_VLAN_VEC_ZERO(vlan_vec_check);
        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx)) {
                vid = MCAST_REPL_VID_GET(unit, port, vport_idx);
                BCM_VLAN_VEC_SET(vlan_vec_check, vid);
            }
        }
        if (!sal_memcmp(vlan_vec_check, vlan_vec, sizeof(bcm_vlan_vector_t))) {
            /* Nothing changed for the pair of (mc_index, port) */
            return BCM_E_NONE;
        }
        
        BCM_VLAN_VEC_ZERO(vlan_vec_add);
        BCM_VLAN_VEC_ZERO(vlan_vec_del);
        /* Prepare the vid lists that going to be added and deleted */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec, vid) && !BCM_VLAN_VEC_GET(vlan_vec_check, vid)) {
                /* The vid is going to be added this time */
                BCM_VLAN_VEC_SET(vlan_vec_add, vid);
            }
            if (!BCM_VLAN_VEC_GET(vlan_vec, vid) && BCM_VLAN_VEC_GET(vlan_vec_check, vid)) {
                /* The vid is going to be deleted this time */
                BCM_VLAN_VEC_SET(vlan_vec_del, vid);
            }
        }

        /* 
          * Save a copy of original software configuration,
          * in case of error return and needs to recover.
          */
        sal_memcpy(&old_mcast_repl_cfg, robo_mcast_repl_config[unit], 
                              sizeof(old_mcast_repl_cfg));
        sal_memcpy(&old_mcast_repl_vlan, robo_mcast_repl_vid[unit], 
                              sizeof(_bcm_robo_mcast_repl_vlan_t));

        /* Do deletion first. In order to clean up the unused vports. */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec_del, vid)) {
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx) &&
                         MCAST_REPL_VID_GET(unit, port, vport_idx) == vid) {
                        MCAST_REPL_USED_CLR(unit, mc_index, port, vport_idx);
                        MCAST_REPL_VID_CNT_DEC(unit, port, vport_idx);
                    }
                }
            }
        }

        /* Then do addition */
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec_add, vid)) {
                found = 0;
                empty_slot_idx = -1;
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if (MCAST_REPL_VID_CNT_IS_ZERO(unit, port, vport_idx)) {
                        /* Look for the first empty slot in case it needed later */
                        if (empty_slot_idx < 0) {
                            empty_slot_idx = vport_idx;
                        }
                    } else {
                        if (vid == MCAST_REPL_VID_GET(unit, port, vport_idx)) {
                            /* Check if the new vid is already in the repl list of the physical port */
                            found = 1;
                            break;
                        }
                    }
                }

                if (found) {
                    MCAST_REPL_USED_SET(unit, mc_index, port, vport_idx);
                    MCAST_REPL_VID_CNT_INC(unit, port, vport_idx);
                } else {
                    /* Put it in the first empty slot */
                    if (empty_slot_idx < 0) {
                        /* 
                          * No empty slot for the port's vport replication list 
                          * Restore the original configuration and return.
                          */
                        sal_memcpy(robo_mcast_repl_config[unit], 
                                              &old_mcast_repl_cfg, 
                                              sizeof(old_mcast_repl_cfg));
                        sal_memcpy(robo_mcast_repl_vid[unit], 
                                              &old_mcast_repl_vlan, 
                                              sizeof(_bcm_robo_mcast_repl_vlan_t));
                        return BCM_E_RESOURCE;
                    } else {
                        MCAST_REPL_USED_SET(unit, mc_index, port, empty_slot_idx);
                        MCAST_REPL_VID_SET(unit, port, empty_slot_idx, vid);
                        MCAST_REPL_VID_CNT_INC(unit, port, empty_slot_idx);
                    }
                }
            }
        }


        /* Update hardware tables */ 
        vport_bmp = 0;
        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, mc_index, port, vport_idx)) {
                vport_bmp |= (1 << vport_idx);
            }
        }

        if (DRV_MCREP_VPGRP_VPORT_CONFIG_SET(unit, mc_index, port,
                DRV_MCREP_VPGRP_OP_VPORT_MEMBER, (int *)&vport_bmp)){
            return BCM_E_INTERNAL;
        }

        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
            if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
                for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
                    if ((MCAST_REPL_VID_GET(unit, port, vport_idx) == vid) &&
                        (!MCAST_REPL_VID_CNT_IS_ZERO(unit, port, vport_idx))) {
                        /* Update Virtual Port VID table */
                        /* Set the replicated vlan id */
                        if (DRV_MCREP_VPORT_CONFIG_SET(unit, port, 
                                DRV_MCREP_VPORT_OP_VID, vport_idx, vid)) {
                            return BCM_E_INTERNAL;
                        }
                        if (vid == BCM_VLAN_UNTAG) {
                            /* Mark the untagged replication */
                            if (DRV_MCREP_VPORT_CONFIG_SET(unit, port, 
                                    DRV_MCREP_VPORT_OP_UNTAG_VP, vport_idx, 0)){
                                return BCM_E_INTERNAL;
                            }
                        }
                    }
                }
            }
        }

        /* Enable/Disable Multicast Replication feature if necessary */
        sal_memset(&old_mcast_repl_cfg, 0, sizeof(_bcm_robo_mcast_repl_t));
        len = sizeof(_bcm_robo_mcast_repl_vport_id_t)*_MAX_REPL_CFG_INDEX_NUM;
        if (!sal_memcmp(old_mcast_repl_cfg.repl_index, 
                                  robo_mcast_repl_config[unit]->repl_index, 
                                  len)) {
            /* all mcast replication configuration not extsted*/
            mcast_repl_en = FALSE;
        } else {
            mcast_repl_en = TRUE;
        }

        if (mcast_repl_en != robo_mcast_repl_config[unit]->mcast_repl_enable) {
            /* Enable status changes */
            ctrl_cnt = 1;
            ctrl_type = DRV_DEV_CTRL_MCASTREP;
            ctrl_val = mcast_repl_en;
            rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, &ctrl_val);
            if (rv) {
                soc_cm_debug(DK_WARN, "%s, %d, SOC Error!\n", 
                        FUNCTION_NAME(), __LINE__);
                return rv;
            }
            robo_mcast_repl_config[unit]->mcast_repl_enable = mcast_repl_en;
        }
        return BCM_E_NONE;
    }
#endif   /* BCM_TB_SUPPORT */        

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_robo_multicast_repl_get
 * Purpose:
 *      Return set of VLANs selected for port's replication list for chosen
 *      L2 multicast group.
 * Parameters:
 *      unit     - Switch device unit number.
 *      index    - The index number.
 *      port     - port for which to retrieve info.
 *      vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_multicast_repl_get(int unit, int index, bcm_port_t port,
                      bcm_vlan_vector_t vlan_vec)
{
#ifdef  BCM_TB_SUPPORT
    int rv = BCM_E_NONE;
    int vport_idx;
    bcm_vlan_t vlan_id;

    if (SOC_IS_TBX(unit)) {
        /* Verify parameters */
        rv = _mcast_repl_param_check(unit, index, port);
        BCM_IF_ERROR_RETURN(rv);

        BCM_VLAN_VEC_ZERO(vlan_vec);

        for (vport_idx = 0; vport_idx < _TB_SUBPORT_NUM_VPORT_PER_GROUP; vport_idx++) {
            if (MCAST_REPL_USED_ISSET(unit, index, port, vport_idx)) {
                vlan_id = MCAST_REPL_VID_GET(unit, port, vport_idx);
                BCM_VLAN_VEC_SET(vlan_vec, vlan_id);
            }
        }
        
        return BCM_E_NONE;
    }
#endif   /* BCM_TB_SUPPORT */        

    return BCM_E_UNAVAIL;
}
