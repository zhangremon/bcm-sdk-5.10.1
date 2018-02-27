/*
 * $Id: port.h 1.39.12.1 Broadcom SDK $
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
 * File:        port.h
 * Purpose:     PORT internal definitions to the BCM library.
 */

#ifndef   _BCM_INT_SBX_PORT_H_
#define   _BCM_INT_SBX_PORT_H_

#include <bcm/debug.h>
#include <soc/sbx/sbFabCommon.h>

typedef struct bcm_sbx_port_info_s {
    mac_driver_t   *p_mac;          /* Per port MAC driver */
    uint32         flags;
} bcm_sbx_port_info_t;

/*
 *  For some unit types, the PHYs do not support the bit that indicates a
 *  state transition has occurred.  We therefore need to track an up/down state
 *  so the port state get can make the callout only on transition, rather than
 *  making it continuously or never.
 */
#define SBX_PORT_STATE_IS_UP (0x00000001)

#define SBX_SUBPORT_FLAG_ON_TS                (1<<0)
#define SBX_SUBPORT_FLAG_ON_ES                (1<<1)
#define SBX_SUBPORT_FLAG_TRUNK_UCAST          (1<<2)
#define SBX_SUBPORT_FLAG_TRUNK_MCAST          (1<<3)
#define SBX_SUBPORT_FLAG_EGRESS_REQUEUE       (1<<4)
#define SBX_SUBPORT_FLAG_INGRESS_MCAST        (1<<5)
#define SBX_SUBPORT_FLAG_IN_TRUNK             (1<<6)
#define SBX_SUBPORT_FLAG_INTERNAL             (SBX_SUBPORT_FLAG_TRUNK_UCAST    | \
					       SBX_SUBPORT_FLAG_TRUNK_MCAST    | \
					       SBX_SUBPORT_FLAG_EGRESS_REQUEUE | \
					       SBX_SUBPORT_FLAG_INGRESS_MCAST)


#ifdef BCM_BM9600_SUPPORT
/* For use in the common code to set up the crossbar mapping correctly */
#define SBX_SFI_PBMP_OFFSET(unit, node, port, sfi_port) \
{ \
   if (SOC_IS_SBX_BM9600(unit)) {  \
       sfi_port = port; \
   } else if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) || \
       (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2)) { \
        sfi_port = port - 50; \
   } else { \
        sfi_port = port - 9; \
   } \
}
#else /* !BCM_BM9600_SUPPORT */
#define SBX_SFI_PBMP_OFFSET(unit, node, port, sfi_port) \
{ \
   if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) || \
       (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2)) { \
        sfi_port = port - 50; \
   } else { \
        sfi_port = port - 9; \
   } \
}
#endif

#define SBX_SFI_PBMP_REVERSE_OFFSET(unit, node, sfi_port, port) \
{ \
   if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) || \
       (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2)) { \
        port = sfi_port + 50; \
   } else { \
        port = sfi_port + 9; \
   } \
}

#define ATTACH_ID_SET(_attach_id, _fcd, _sysport)                       \
        _SHR_COSQ_GPORT_ATTACH_ID_SET(_attach_id,_fcd,_sysport)
#define ATTACH_ID_SYSPORT_GET(_attach_id)                               \
        _SHR_COSQ_GPORT_ATTACH_ID_SYSPORT_GET(_attach_id)
#define ATTACH_ID_FCD_GET(_attach_id)                                   \
        _SHR_COSQ_GPORT_ATTACH_ID_FCD_GET(_attach_id)

/* egress group information */
typedef struct bcm_sbx_egroup_info_s {
    uint16          es_scheduler_level0_node;   /* Allocated Fifo number */
    uint16          ef_fcd;                     /* EF FCD */
    uint16          nef_fcd;                    /* Non-EF FCD */
    uint8           num_fifos;                  /* Number of fifos for the subport in egress */
    bcm_gport_t     egroup_gport;               /* Egress Group */
    uint32          fcd[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE]; /* assigned fcd */
    uint16          mem_index;                  /* Memory Allocation index associated with subport */
    uint16          mem_size;                   /* Number of memory buffers associated with subport */
    uint32          port_speed;                 /* Actual port speed */
} bcm_sbx_group_info_t;

/* indexed by the subport handle returned to the user */
typedef struct bcm_sbx_subport_info_s {
    int             valid;                      /* Valid subport */
    uint32          flags;                      /* Subport flags */
    bcm_gport_t     parent_gport;               /* Parent interface gport */
    bcm_gport_t     original_gport;             /* Used by trunking only, the original gport of the internal trunking gport */
    int             group_shaper;               /* Used by trunking only, the group shaper used for this subport */
    int             port_offset;                /* Port offset within the parent interface */
    int             ts_scheduler_level;         /* Node level for the subport in TS scheduler tree */
    int             ts_scheduler_node;          /* Node id for the subport in TS scheduler tree */
    int             es_scheduler_level2_node;   /* Level 2 (channel) node id in ES scheduler tree */
    int             es_scheduler_level1_node[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX];   /* Level 1 (subport) node id in ES scheduler tree */
    uint8           unused_fifos;               /* Allocated, but unused */
    bcm_sbx_group_info_t egroup[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX];
} bcm_sbx_subport_info_t;

typedef struct bcm_sbx_port_state_s {
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_scache_handle_t wb_handle;  /* Warmboot memory handle */
#endif
    int            init;            /* TRUE if port module has been initialized */
    sal_mutex_t    port_lock;       /* port lock */
    uint8          *ability[SB_FAB_DEVICE_MAX_PHYSICAL_SERDES];/* array of port abilities - only used for SFI/SCI backplane links on a given node */
                                                               /* ability[physical_xbar][node] = ability SFI/DUAL_SFI/SCI/SFI_SCI                 */
    uint32         uPrbsModeSi;
    uint32         uPrbsForceTxError[SB_FAB_USER_MAX_NUM_SERIALIZERS];
    uint32         uDriverEqualizationFarEnd[SB_FAB_USER_MAX_NUM_SERIALIZERS];
    uint32         uDriverPriorPostCursorNegative[SB_FAB_USER_MAX_NUM_SERIALIZERS];

    int            cpu_fabric_port;        /* fabric port handle for CPU port */
    uint8          fabric_header_format;    /* fabric header format */
    bcm_sbx_port_info_t    *port_info;     /* per bcm port info */
    bcm_sbx_subport_info_t *subport_info;  /* per subport info */
} bcm_sbx_port_state_t;

enum bcm_sbx_port_egress_shaper_type_e {
      BCM_SBX_PORT_EGRESS_SHAPER_UNKNOWN = 0,
      BCM_SBX_PORT_EGRESS_SHAPER_PORT,
      BCM_SBX_PORT_EGRESS_SHAPER_FIFO
};

#define BCM_SBX_PORT_EGRESS_SHAPER_FULL_MATCH      0x0001
#define BCM_SBX_PORT_EGRESS_SHAPER_PARTIAL_MATCH   0x0002


typedef struct bcm_sbx_port_shaper_state_s {
    int                         is_free;              /* template is not utlized */
    uint32                      shaper_type;           /* queue size for this template */
    uint32                      shaper_src;           /* queue size for this template */
    uint32                      hi_side;              /* hi/lo shaper selection control   */
    uint32                      port;                 /* Port index */
} bcm_sbx_port_shaper_state_t;


typedef struct bcm_sbx_port_congestion_info_s {
    bcm_port_congestion_config_t config;
} bcm_sbx_port_congestion_info_t;

/* 4 FIFO model currently */
#define BCM_INT_XCORE_COS_FIFO_UNICAST_EF     SB_FAB_XCORE_COS_FIFO_UNICAST_EF   
#define BCM_INT_XCORE_COS_FIFO_UNICAST_NEF    SB_FAB_XCORE_COS_FIFO_UNICAST_NEF  
#define BCM_INT_XCORE_COS_FIFO_MULTICAST_EF   SB_FAB_XCORE_COS_FIFO_MULTICAST_EF 
#define BCM_INT_XCORE_COS_FIFO_MULTICAST_NEF  SB_FAB_XCORE_COS_FIFO_MULTICAST_NEF

/*
 * Debug Output Macros
 */
#define PORT_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_PORT, stuff)
#define PORT_OUT(stuff)           PORT_DEBUG(BCM_DBG_PORT, stuff)
#define PORT_WARN(stuff)          PORT_DEBUG(BCM_DBG_WARN, stuff)
#define PORT_ERR(stuff)           PORT_DEBUG(BCM_DBG_ERR, stuff)
#define PORT_VERB(stuff)          PORT_DEBUG(BCM_DBG_VERBOSE, stuff)


/*
 * General Utility Macros
 */

/* Parameter and module initialization checking */
#define PARAM_NULL_CHECK(arg) \
    if ((arg) == NULL) { return BCM_E_PARAM; }

/* Port Interfaces */
#define INTERFACE_MAC(unit, port)   (IS_E_PORT(unit,port) || \
                                     IS_HG_PORT(unit,port))
#define INTERFACE_SPI(unit, port)   (IS_SPI_PORT(unit,port) || \
                                     IS_SPI_SUBPORT_PORT(unit,port))

#define IS_SPI_BUS_PORT(unit, port)  IS_SPI_PORT(unit, port)


/*
 * External functions
 */
extern int bcm_sbx_port_init(int unit);

extern int bcm_sbx_port_get_scheduler(int unit,
				      bcm_gport_t scheduler_gport,
				      int *scheduler_level,
				      int *scheduler_node);

extern int
bcm_sbx_port_get_port_portoffset(int unit,
				 int subport,
				 bcm_port_t *port,
				 int *port_offset);

extern int
bcm_sbx_port_get_intf_portoffset(int unit,
				 int hg_subport,
				 int *intf,
				 int *port_offset);

extern int
bcm_sirius_port_all_subports_speed_get(int unit,
                                       bcm_port_t port,
                                       int *speed);

extern int
bcm_sirius_port_subport_getnext(int unit,
                                bcm_port_t port,
				uint32 flags_out,				
                                bcm_gport_t *subport);

extern int
bcm_sbx_port_ability_local_get(int unit,
                               bcm_port_t port,
                               bcm_port_ability_t *ability_mask);

extern int
bcm_sbx_port_any_is_encap_higig(int unit, int *is_encap_higig);

#ifdef BCM_SIRIUS_SUPPORT
extern int
bcm_sbx_port_qinfo_get(int unit, bcm_gport_t gport, int *eg_n,
		       bcm_sbx_subport_info_t *sp_info);
#endif

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
extern int
bcm_sbx_port_get_state(int unit, char *pbuf);
#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* BCM_EASY_RELOAD_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
extern int
bcm_sbx_wb_port_state_sync(int unit, int sync);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_port_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif /* _BCM_INT_SBX_PORT_H_ */
