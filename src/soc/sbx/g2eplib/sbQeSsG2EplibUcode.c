/*
 * Generated from sbQeSsG2EplibUcode.eg on Fri Aug 14 13:30:19 2009
 * GENERATED FILE DO NOT EDIT
 */

/*
 * -*-  Mode:text; c-basic-offset:4 -*-
 *
 * $Id: sbQeSsG2EplibUcode.c 1.2.112.1 Broadcom SDK $
 *
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
 * Guadalupe2k QE2000 Egress Processor Class Instructions
 *
 *======================================================================================
 *
 *  Processing Class Breakdown
 *
 *  Class | HW Type      |  Description
 *  ------+--------------+-------------------------------------------
 *  0     | VLAN         |  Traditional Bridging Unicast
 *  ------+--------------+-------------------------------------------
 *  1     | VLAN         |  Logical Interface Unicast
 *  ------+--------------+-------------------------------------------
 *  2     | VLAN         |  Traditional Bridging Multicast
 *  ------+--------------+-------------------------------------------
 *  3     | VLAN         |  Logical Interface Multicast
 *  ------+--------------+-------------------------------------------
 *  4     | VLAN         |  Logical Interface Multicast (McGroup > 32K)
 *  ------+--------------+-------------------------------------------
 *
 *======================================================================================
 */

unsigned long aulKaQeSsEpClassInstr[16][8] =
{
	/*
	 * Class 0 Instruction Array
	 */
	{
		0x3406EC0B,
		0x340B4C23,
		0x1176EC29,
		0x1176EC18,
		0x90030417,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 1 Instruction Array
	 */
	{
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x90030417,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 2 Instruction Array
	 */
	{
		0x10025C27,
		0x180A540A,
		0x1A0A540A,
		0x3406EC04,
		0x340B4C23,
		0x1176EC29,
		0x1176EC18,
		0x10030417
	},

	/*
	 * Class 3 Instruction Array
	 */
	{
		0x10025C27,
		0x112A5403,
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x90030417,
		0x00000000
	},

	/*
	 * Class 4 Instruction Array
	 */
	{
		0x10025C27,
		0x132A5001,
		0x112A5403,
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x90030417
	},

	/*
	 * Class 5 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 6 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 7 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 8 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 9 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 10 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 11 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 12 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 13 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 14 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 15 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	}
}; /* end aulKaQeSsEpClassInstr */

/* end array declarations */
