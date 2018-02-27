/*
 * $Id: bm9600_diags.h 1.2 Broadcom SDK $
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
 * == bm9600_diags.h - Polaris Diagnostics      ==
 */

#ifndef _BM9600_DIAGS_H_
#define _BM9600_DIAGS_H_

#include <soc/sbx/hal_pl_auto.h>
#include <bcm/port.h>
#include "sbx_diags.h"

/* theoretical maximum is 96 */
#define MAX_QE_NODES 8
#define MAX_LINK_ENABLES 20 /* includes SCI links */

#define PRBS_TO_PL 0
#define PRBS_TO_QEXX 1

#define PRBS_TO_PL 0
#define PRBS_TO_QEXX 1

/* to do move dev_cmd_rec from sbx.c to sbx.h */
/* make extern in sbx.h */
#if 1
typedef struct my_dev_cmd_rec_s {
    int cmd;
    int unit;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    int arg6;
} my_dev_cmd_rec;
#endif

int sbBme9600DiagsPrbsTest(sbxDiagsInfo_t *pDiagsInfo);
int sbBme9600DiagsSCIPrbsTest(sbxDiagsInfo_t *pDiagsInfo);
int sbBme9600DiagsSFIPrbsTest(sbxDiagsInfo_t *pDiagsInfo,
			      my_dev_cmd_rec *pcmd,
			      int qexx_unit);

int sbBme9600DiagsFindAllQExx(int unit,
				   int qexx_lcm_ids[]);

int sbBme9600DiagsStartPrbs(int rx_unit,
			    int rx_port,
			    int tx_unit,
			    int tx_port,
			    uint8_t bForceError,
			    bcm_port_prbs_t prbs_type,
			    int link,
			    int *prbs_status);

int sbBme9600DiagsStopPrbs(int rx_unit,
			   int rx_port,
			   int tx_unit,
			   int tx_port);

int sbBme9600DiagsGetQExxXbarMapping(my_dev_cmd_rec *p,
				     int qexx_modid, 
				     int nQePhysicalPort);


int sbBme9600DiagsRegTest(sbxDiagsInfo_t *pDiagsInfo);
void polaris_sigcatcher(void);

#endif /* _BM9600_DIAGS_H_ */
