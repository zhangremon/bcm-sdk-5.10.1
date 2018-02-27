/*
 * $Id: brd_sbx.c 1.27 Broadcom SDK $
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
 * File:        brd_sbx.c
 * Purpose:     Board support for SBX reference boards
 *
 */ 

#include <soc/sbx/fe2000.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/bsc.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>

static soc_sbx_functions_t brd_sbx_func[BOARD_TYPE_MAX];

typedef struct brd_sbx_control_s {
    uint8 pre_init;   /* Pre Init status: TRUE or FALE */
} brd_sbx_control_t;

static brd_sbx_control_t brd_sbx_control[BOARD_TYPE_MAX];

#define FPGA_BASE                     (0x10000)
#define FPGA_MASTER_REG_OFFSET        0x14
#define FPGA_MASTER_MODE_BIT          0x10
#define FPGA_SCI_ROUTING_OFFSET       0x19
#define FPGA_SCI_TO_LCM               0xA0
#define FPGA_FE2K_DLL_ENABLE_OFFSET   0x1a
#define FPGA_FE2K_DLL_ENABLE          0x03

#define FPGA_LC_PL_INT_OFFSET         0xa 
#define FPGA_LC_PL_INT                0x08 /* bit 19 of 32 bit reg starting at 0x8 */
#define FPGA_FC_PL_INT_OFFSET         0x8
#define FPGA_FC_PL_INT                0x1  /* bit 0 of 32 bit reg starting at 0x8 */

static int
board_metrocore_fpga_read8(int addr, uint8_t *v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int word = FPGA_BASE + (addr & ~0x3);
    int shift = (3 - (addr & 3)) * 8;

    int i;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(i)){
        if (lcm0==-1)
          lcm0=i;
        else if (lcm1==-1)
          lcm1=i;
      }
    }

    if (lcm1 == -1) {
        printk("LCM/BME with FPGA not found\n");
        return BOARD_E_FAIL;
    }

    *v = (CMREAD(lcm1, word) >> shift) & 0xff;

    return BOARD_E_NONE;
}

static int
board_metrocore_fpga_write8(int addr, uint8_t v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int shift = (3 - (addr & 3)) * 8;
    int word = FPGA_BASE + (addr & ~0x3);
    int i;
    uint32_t v0;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(i)){
        if (lcm0==-1)
          lcm0=i;
        else if (lcm1==-1) 
          lcm1=i;
      } 
    }

    if (lcm1 == -1) {
        printk("LCM/BME with FPGA not found\n");
        return BOARD_E_FAIL;
    }

    v0 = CMREAD(lcm1, word);
    v0 &= ~(0xff << shift);
    v0 |= (v << shift);
    CMWRITE(lcm1, word, v0);

    return BOARD_E_NONE;
}   

static void
brd_metrocore_train_ddr(sbhandle sbh)
{
    sal_usecs_t stime;
    printk("Training DDR:\n");
    stime = sal_time_usecs();
#ifdef BCM_FE2000_SUPPORT
    soc_sbx_fe2000_ddr_train((int)sbh);
#endif /* BCM_FE2000_SUPPORT */
    printk("%s[u:%d] DDRTrain took %u usecs\n", SOC_CHIP_STRING((int)sbh),
	   (int)sbh, SAL_USECS_SUB(sal_time_usecs(),stime));
}

static void
brd_metrocore_reset_sram_dll(sbhandle sbh)
{
    board_metrocore_fpga_write8(FPGA_FE2K_DLL_ENABLE_OFFSET, 0);
    board_metrocore_fpga_write8(FPGA_FE2K_DLL_ENABLE_OFFSET,
                                FPGA_FE2K_DLL_ENABLE);    
}

#if (defined(BCM_FE2000_SUPPORT) && defined(INCLUDE_I2C) && defined(BROADCOM_SVK))
#define FE2KXT_AT24C64_DDRCONFIG_OFFSET 0x100

typedef struct ddrconfig_s {
    uint8_t uValidDelay[6];
    uint8_t uPhaseSelect[6];
    uint8_t uQkMidRange[6];
    uint8_t uQknMidRange[6];
    uint8_t uXMidRange[6];
    uint16_t chksum;
} ddrconfig_t;

static int
brd_fe2kxt_ddrconfig_readwrite(int unit, int write, sbFe2000XtInitParamsMmDdrConfig_t *ddrconfig)
{
    int devno, retv, i, j, index;
    uint32_t length = sizeof(ddrconfig_t);
    uint32_t tmp_length;
    ddrconfig_t nvdata;
    uint16_t tmp_chksum;

    devno = soc_bsc_devopen(unit, BSC_AT24C32_NAME);
        if (devno  < 0) {
            printk("cannot open I2C device %s\n", BSC_AT24C32_NAME);
            return BOARD_E_FAIL;
        }
    if (write) {
        index = 0;
        for (i=0; i < SB_FE2000_NUM_MM_INSTANCES; i++) {
            for (j=0; j < SB_FE2000XT_DDR_PORTS_PER_MMU; j++) {
                nvdata.uValidDelay[index] = ddrconfig[i].uValidDelay[j];
                nvdata.uPhaseSelect[index] = ddrconfig[i].uPhaseSelect[j];
                nvdata.uQkMidRange[index] = ddrconfig[i].uQkMidRange[j];
                nvdata.uQknMidRange[index] = ddrconfig[i].uQknMidRange[j];
                nvdata.uXMidRange[index] = ddrconfig[i].uXMidRange[j];
                index++;
            }
        }
        nvdata.chksum = _shr_crc16(0, (uint8_t*)&nvdata, length-2);

        retv = soc_eep24c64_write(unit, devno, FE2KXT_AT24C64_DDRCONFIG_OFFSET, (uint8 *)&nvdata, length);
    }else{
        tmp_length = length;
        retv = soc_eep24c64_read(unit, devno, FE2KXT_AT24C64_DDRCONFIG_OFFSET, (uint8 *)&nvdata, &tmp_length);
        if (length != tmp_length) {
            printk("%s: read length check failed\n", BSC_AT24C32_NAME);
            return BOARD_E_FAIL;
        }
        tmp_chksum = _shr_crc16(0, (uint8_t*)&nvdata, length-2);
        if (tmp_chksum != nvdata.chksum) {
            printk("ERROR %s: chksum check failed, ddr config NOT loaded\n", BSC_AT24C32_NAME);
            return BOARD_E_FAIL;
        }
        index = 0 ;
        for (i=0; i < SB_FE2000_NUM_MM_INSTANCES; i++) {
            for (j=0; j < SB_FE2000XT_DDR_PORTS_PER_MMU; j++) {
                ddrconfig[i].uValidDelay[j] = nvdata.uValidDelay[index];
                ddrconfig[i].uPhaseSelect[j] = nvdata.uPhaseSelect[index];
                ddrconfig[i].uQkMidRange[j] = nvdata.uQkMidRange[index];
                ddrconfig[i].uQknMidRange[j] = nvdata.uQknMidRange[index];
                ddrconfig[i].uXMidRange[j] = nvdata.uXMidRange[index];
                index++;
            }
            ddrconfig[i].bValid = 1;
        }
    }
    return BOARD_E_NONE;
}
#endif /* BCM_FE2000_SUPPORT && INCLUDE_I2C */

int
board_sbx_register(brd_sbx_type_t brd_type, int max_devices)
{
    int unit, rv = BOARD_E_NONE;
    uint32_t rev;

    if (brd_type != BOARD_TYPE_METROCORE && 
	brd_type != BOARD_TYPE_POLARIS_LC &&
	brd_type != BOARD_TYPE_SIRIUS_SIM &&
	brd_type != BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
        return BOARD_E_PARAM;
    }


    for (unit = 0; unit < max_devices; unit++) {
         if (SOC_IS_SBX_FE2000(unit)) {
             rev = SAND_HAL_READ(unit, CA, PC_REVISION);
             rev = SAND_HAL_GET_FIELD(CA, PC_REVISION, REVISION, rev);

             brd_sbx_func[brd_type].sram_init = brd_metrocore_reset_sram_dll;
             if (rev == BCM88020_A0_REV_ID) {
                 brd_sbx_func[brd_type].ddr_train = NULL;
             } else {
                 brd_sbx_func[brd_type].ddr_train = brd_metrocore_train_ddr;
             }
#if (defined(BCM_FE2000_SUPPORT) && defined(INCLUDE_I2C) && defined(BROADCOM_SVK))
             if (SOC_IS_SBX_FE2KXT(unit)) {
                 brd_sbx_func[brd_type].fe2kxt_ddrconfig_rw = brd_fe2kxt_ddrconfig_readwrite;
             }
#endif

             rv = soc_sbx_register(unit, &brd_sbx_func[brd_type]);
         }
    }

    return rv;
}

void
board_sbx_unregister(brd_sbx_type_t brd_type)
{

}

int
board_preinit(brd_sbx_type_t brd_type)
{
    int rv = BOARD_E_NONE;
    int addr, data;
    uint8 v;

    switch(brd_type) {
    case BOARD_TYPE_METROCORE:
        if (brd_sbx_control[brd_type].pre_init) {
            return BOARD_E_NONE;
        }

        /*
         * mcfpga addr=0x1a data=0x0 write ; \
         * mcfpga addr=0x1a data=0x3 write"
         */
        addr = 0x1a;
        data = 0x0;
        rv = board_metrocore_fpga_read8(addr, &v);
        if (rv) {
            return rv;
        }

        v = ((unsigned int) data) & 0xff;
        rv = board_metrocore_fpga_write8(addr, v);
        if (rv) {
            return rv;
        }

        addr = 0x1a;
        data = 0x3;
        rv = board_metrocore_fpga_read8(addr, &v);
        if (rv) {
            return rv;
        }

        v = ((unsigned int) data) & 0xff;
        rv = board_metrocore_fpga_write8(addr, v);
        if (rv) {
            return rv;
        }

        /*
         * Register board specific sram reset dll 
         */
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

        brd_sbx_control[brd_type].pre_init = TRUE;

        /*
         * Set the FABRIC_CONFIGURATION property
         */
        rv = sal_config_set(spn_FABRIC_CONFIGURATION, "0");
        if (rv != 0) {
            return rv;
        }
        return BOARD_E_NONE;

    case BOARD_TYPE_POLARIS_LC:
    case BOARD_TYPE_SIRIUS_SIM:
    /* same as BOARD_TYPE_SIRIUS_IPASS */
        if (brd_sbx_control[brd_type].pre_init) {
            return BOARD_E_NONE;
        }

        /*
         * Register board specific sram reset dll 
         */
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

        brd_sbx_control[brd_type].pre_init = TRUE;

        return BOARD_E_NONE;
    case BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC:
        if (brd_sbx_control[brd_type].pre_init) {
  	    return BOARD_E_NONE;
        }
	brd_sbx_control[brd_type].pre_init = TRUE;
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

	return BOARD_E_NONE;
    case BOARD_TYPE_LCMODEL:
    case BOARD_TYPE_METROCORE_FABRIC:
        brd_sbx_control[brd_type].pre_init = TRUE;
	/* need to make sure that interrupts are enabled */
	rv = board_metrocore_fpga_read8(0x0, &v);
        if (rv) {
            return rv;
        }
	if (v == 0x15){
	  /* SFM enable FPGAs */
	  rv = board_metrocore_fpga_read8(0x14, &v);
          if (rv) {
              return rv;
          }
	  v|= 0xf;
          board_metrocore_fpga_write8(0x14, v);
        }else if (v == 0x17){
          /* SFM enable interrupts */
          rv = board_metrocore_fpga_read8(0x15, &v);
          if (rv) {
              return rv;
          }
          v|= 0xc;
          rv = board_metrocore_fpga_write8(0x15, v);
          if (rv) {
              return rv;
          }
	}
        return BOARD_E_NONE;
    case BOARD_TYPE_POLARIS_FC:
        brd_sbx_control[brd_type].pre_init = TRUE;
        return BOARD_E_NONE;

    /* The following Board Type is for the QE2K Benchscreen board. */
    case BOARD_TYPE_QE2K_BSCRN_LC:
    {
        if (brd_sbx_control[brd_type].pre_init) {
  	    return BOARD_E_NONE;
        }
	brd_sbx_control[brd_type].pre_init = TRUE;
	return BOARD_E_NONE;
    }
    case BOARD_TYPE_POLARIS_IPASS:
    {
	brd_sbx_control[brd_type].pre_init = TRUE;
        return BOARD_E_NONE;
        break;
    }

    default:
      break;
    }

    return BOARD_E_PARAM;
}
