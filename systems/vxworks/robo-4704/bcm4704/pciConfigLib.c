/* $Id: pciConfigLib.c 1.2.240.1 Broadcom SDK $
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/

/*
modification history
--------------------
01c,06sep02,jmb   Add missing ConfigOut functions and support for config
                  cycles on secondary buses, fix byte-swap on ConfigIn's
01b,14aug02,jmb   Nithya's patches to do pciConfig cycles to devices with
                  devNo > PCI_IDSEL_ALINE_INDIRECT_DEV.
                  Move PCI memory space to uncached address.
                  Also a bunch of temporary patches for PCI testing.
01a,11aug02,jmb   created from sysLib.c,v 1.33 for HNBU 4710 eval board.  Merged
                  in code from Strata idtrp334 BSP.
*/


/* pciConfigLib.c - PCI Configuration space access support for the bcm4710 */

/* This module allows the SB Bus devices to mimic PCI devices on bus 0 */

#include "vxWorks.h"
#include "config.h"

#if defined(INCLUDE_PCI)

#include "dllLib.h"
#include "sysLib.h"
#include "intLib.h"
#include "taskLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "config.h"
#include "drv/pci/pciConfigLib.h"

/* SB -> PCI mapping */
#include "sb2pci.h"
#include "sbconfig.h"
#include "sbextif.h"
#include "sbpci.h"
#include "sbchipc.h"
#ifndef BROADCOM_BSP  /* Don't need for MBZ */
#include "bcmnvram.h"
#endif

#include "vx_osl.h"
#include "bcmenet47xx.h"

void sb_error(void);

/* globals */
int     pciMaxBus = PCI_MAX_BUS;	/* Max number of sub-busses */
STATUS  pciLibInitStatus = NONE;	/* initialization done */
unsigned long pci_intnum = NONE;

#ifdef BROADCOM_BSP
int     pciConfigMech = NONE;
#endif

#define MAX_SB_CORES  9  /* temp limit until sb_errors are handled correctly */

sb2pci_t sbDevices[MAX_SB_CORES];

#define PCI_HOSTBRIDGE_DEV    4
#define PCI_IDSEL_ALINE_START 12
#define PCI_IDSEL_ALINE_INDIRECT_DEV (31 - 6 - PCI_IDSEL_ALINE_START)
#define PCI_IND_DEV (PCI_IDSEL_ALINE_INDIRECT_DEV + 1)
#define PCI_CFG_47XX(d, f, o)  \
        (0xAC000000 | (1 << (PCI_IDSEL_ALINE_START+(d))) | ((f) << 8) | (o)) 

static uint32 sbclk=0;
uint32 wd_ms=0;
static int sb_extif_tpsflag=-1;
static chipcregs_t *cc=NULL;
static extifregs_t *extif=NULL;
static volatile sbpciregs_t *pci;
void platform_init(void);
void unmask_pci_interrupt(unsigned long bit);
void mask_pci_interrupt(unsigned long bit);

/* routine to return the sb clock speed (discovered during init ) */
uint32 get_sb_clock(void) {
    ASSERT(sbclk);
    return(sbclk);
}

uint32 get_sb_mips_clock(void) {
    return(*((volatile UINT32 *)(0xb80000a0)) == 0x5000200?264000000:200000000);
}

/* routine to return the extif tpsflag (discovered during init )  */
uint32 get_sb_extif_tpsflag(void) {
    return(sb_extif_tpsflag);
}


STATUS pciConfigLibInit
    (
    int mechanism, /* configuration mechanism: 0, 1, 2 */
    ULONG addr0,   /* config-addr-reg / CSE-reg */
    ULONG addr1,   /* config-data-reg / Forward-reg */
    ULONG addr2	   /* none            / Base-address */
    )
    {
	char buf[200];
    void    *sbaddr;
    void    *sbh;
    int     dev;
    unsigned long sbvalue, flag, irq;
    unsigned short vendor, core;
    unsigned long class;
    unsigned char revision;

    unsigned long sbips_int_mask[] = { 0, SBIPS_INT1_MASK, SBIPS_INT2_MASK, 
				       SBIPS_INT3_MASK, SBIPS_INT4_MASK };

    unsigned long sbips_int_shift[] = { 0, 0, SBIPS_INT2_SHIFT, 
					SBIPS_INT3_SHIFT, SBIPS_INT4_SHIFT };
    sbconfig_t * sbregs;

    BPRINT("PCI1");
    if (pciLibInitStatus != NONE)
        return (pciLibInitStatus);

    pciLibInitStatus = OK;
#ifdef BROADCOM_BSP
    pciConfigMech = PCI_MECHANISM_0;
#endif

    /* zero out the sbdevices array */
    memset(sbDevices, 0, (sizeof(sb2pci_t)*MAX_SB_CORES));

    BPRINT("PCI2");
    /* init sbutils handle */
    sbh = sb_kattach(BCM4710_DEVICE_ID, 0);
        ASSERT(sbh);

    BPRINT("PCI3");
    sbclk = sb_clock(sbh);            /* save the sb_clock speed */
    
    BPRINT("PCI4");
    /* Scan SB and create a sbDevices table */
    for( dev=0; dev < MAX_SB_CORES; dev++) {
    
        sbaddr = (void*)KSEG1ADDR(SB_ENUM_BASE + (SB_CORE_SIZE*dev) + SBCONFIGOFF);
        sbregs = (sbconfig_t *)sbaddr;
        
                sbvalue = 0;
                /* check if core exists */
                BUSPROBE(sbvalue, (&sbregs->sbidhigh));
                if ((sbvalue == 0) || (sbvalue == 0xffffffff))
                        break;

                sbvalue = sbregs->sbidhigh;

        vendor = (sbvalue & SBIDH_VC_MASK) >> SBIDH_VC_SHIFT;
        core = (sbvalue & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT;
        revision = (sbvalue & SBIDH_RC_MASK);

        /*
        * vendor = sb_corevendor(sbregs);
        * core = sb_coreid(sbregs);
        * revision = sb_corerev(sbregs);
        */

        /* Known vendor translations */
        switch (vendor) {
        case SB_VEND_BCM:
            vendor = VENDOR_BROADCOM;
            break;
        }
    
		sprintf (buf, " dev %d, sbval %x, core %x\n", dev, (int)sbvalue, core);
		sysSerialPrintString(buf);
        /* Determine class based on known core codes */
        switch (core) {
        case SB_ILINE20:
            class = PCI_CLASS_NETWORK_ETHERNET;
            core = BCM47XX_ILINE_ID;
            break;
        case SB_ENET:
            class = PCI_CLASS_NETWORK_ETHERNET;
            core = BCM47XX_ENET_ID;
            break;
        case SB_SDRAM:
            class = PCI_CLASS_MEMORY_RAM;
            break;
        case SB_PCI:
            sb_setcore(sbh, SB_PCI, 0);
            sb_core_reset(sbh, 0);
            class = PCI_CLASS_BRIDGE_PCI;            
            break;
        case SB_MIPS33:
            /* Set up the interrupt mappings */
            sbregs->sbipsflag   = DEF_SBIPSFLAG; /* 0x01020304; */
            sbregs->sbintvec    = DEF_SBINTVEC;  /* 0x00000021; */
            sysSerialPrintString("SB_MIPS33\n");
	    class = PCI_CLASS_PROCESSOR_MIPS;
            break;
#if 1
        case SB_MIPS:
            /* Set up the interrupt mappings */
            sbregs->sbipsflag   = DEF_SBIPSFLAG;
            sbregs->sbintvec    = DEF_SBINTVEC;     
			class = PCI_CLASS_PROCESSOR_MIPS;
            sysSerialPrintString("SB_MIPS\n");
            break;
#endif
        case SB_CODEC:
            class = PCI_CLASS_COMMUNICATION_MODEM;
            core = BCM47XX_V90_ID;
            break;
        case SB_USB:
            class = PCI_CLASS_SERIAL_USB;;
            core = BCM47XX_USB_ID;
            break;
        case SB_EXTIF:
            class = PCI_CLASS_MEMORY_FLASH;
            break;
        default:
            /* Unknown core */
            sbDevices[dev].vendor_devid = 0xffffffff;
            continue;
        }
    
        sbDevices[dev].ibErr = 0;
        sbDevices[dev].toErr = 0;
        sbDevices[dev].vendor_devid = (core << 16) | vendor;
        sbDevices[dev].classcode    = (class << 8) | revision;
        sbDevices[dev].bar0         = (unsigned long)sbaddr - SBCONFIGOFF;

        /* Figure out which vx interrupt num is connected to this core */
        sbDevices[dev].vx_int_num = 0;
        flag = *(unsigned long*)((unsigned long)sbaddr + SBTPSFLAG);

        if(!flag) /* if int not generated by this core leave it as zero */
            continue;

        flag &= SBTPS_NUM0_MASK;

        /* tpsflag for the extif */
	sb_extif_tpsflag = 0;

        sbDevices[dev].tpsflag = flag;
        sbvalue = DEF_SBIPSFLAG; 

        for (irq = 1; irq <= 4; irq++) {
            if (((sbvalue & sbips_int_mask[irq]) >> sbips_int_shift[irq]) == flag) 
                sbDevices[dev].vx_int_num = irq + IV_IORQ0_VEC;
	}
        if(!sbDevices[dev].vx_int_num)            
            sbDevices[dev].vx_int_num = IV_IORQ0_BIT0_VEC + flag;

        if ((sbDevices[dev].classcode >> 8) == PCI_CLASS_BRIDGE_PCI)
            pci_intnum = sbDevices[dev].vx_int_num;
        }

    BPRINT("PCI7");

    sb_detach (sbh);
    BPRINT("PCI8");
    return (pciLibInitStatus);
    }


/* Handler for SB errors */
void sb_error(void)
{
    sbconfig_t *cfg;
    unsigned long sbimstate;
    int dev;

    for (dev=0; dev < MAX_SB_CORES; dev++) 
        {		
        if (sbDevices[dev].vendor_devid != 0xffffffff) 
            {      /* Skip non-SB devices */ 
            cfg = (sbconfig_t *) 
                KSEG1ADDR(SB_ENUM_BASE + (SB_CORE_SIZE*dev) + SBCONFIGOFF);
            sbimstate = *(unsigned long *)&cfg->sbimstate;
            if (sbimstate & SBIM_TO) 
                {
                *(unsigned long *)&cfg->sbimstate = sbimstate & ~SBIM_TO;
                sbDevices[dev].toErr++;
    	        }
            if (sbimstate & SBIM_IBE) 
                {
                *(unsigned long *)&cfg->sbimstate = sbimstate & ~SBIM_IBE;
                sbDevices[dev].ibErr++;
                }
            }
        }
    }

void dumpSB(void)
    {
    unsigned char dev;

    for (dev=0; dev < MAX_SB_CORES; dev++) 
        if ( sbDevices[dev].vendor_devid != 0xffffffff)
            printf ("DEVID: 0x%x  BAR0: 0x%x   INT: 0x%x  CLASS: 0x%x SB TO: 0x%x SB IBE 0x%x\n", 
                (unsigned int)sbDevices[dev].vendor_devid, 
                (unsigned int)sbDevices[dev].bar0, 
                (unsigned int)sbDevices[dev].vx_int_num, 
                (unsigned int)sbDevices[dev].classcode,
                (unsigned int)sbDevices[dev].toErr, 
                (unsigned int)sbDevices[dev].ibErr);

    }

/*******************************************************************************
*
* pciFindDevice - find the nth device with the given device & vendor ID
*
* This routine finds the nth device with the given device & vendor ID.
*
* RETURNS:
* OK, or ERROR if the deviceId and vendorId didn't match.
*/

STATUS pciFindDevice
    (
    int    vendorId,	/* vendor ID */
    int    deviceId,	/* device ID */
    int    index,	/* desired instance of device */
    int *  pBusNo,	/* bus number */
    int *  pDeviceNo,	/* device number */
    int *  pFuncNo	/* function number */
    )
    {
    STATUS status = ERROR;
    int    busNo;
    int    deviceNo;
    int    funcNo;
    UINT32 device;
    volatile UINT32 vendor;
    unsigned char   header;

    if (pciLibInitStatus != OK)			/* sanity check */
        return(status);

    /* PCI bus numbers start at 1 */

    for (busNo = 1; busNo <= pciMaxBus; busNo++)
        {
        for (deviceNo = 0;
	     ((deviceNo < PCI_MAX_DEV));
	     ++deviceNo)
            for (funcNo = 0; funcNo < PCI_MAX_FUNC; funcNo++)
		{
		/* avoid a special bus cycle */

		if ((deviceNo == 0x1f) && (funcNo == 0x07))
		    continue;
        
		pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID,
				 &vendor);

		/*
		 * If nonexistent device, skip to next, only look at
		 * vendor ID field for existence check
		 */
                OSL_DELAY(10);
		if (((vendor & 0x0000ffff) == 0x0000FFFF) && (funcNo == 0))
		    break;

		device  = vendor >> 16;
		device &= 0x0000FFFF;
		vendor &= 0x0000FFFF;
		if ((vendor == (UINT32)vendorId) &&
		    (device == (UINT32)deviceId) &&
		    (index-- == 0))
		    {
                        *pBusNo	= busNo;
                        *pDeviceNo	= deviceNo;
                        *pFuncNo	= funcNo;
                        status	= OK;
                        return(status);
		    } else {
                        /* goto next if current device is single function */

                        pciConfigInByte (busNo, deviceNo, funcNo,
                                            PCI_CFG_HEADER_TYPE, &header);
                        if ((header & PCI_HEADER_MULTI_FUNC) !=
                                    PCI_HEADER_MULTI_FUNC && funcNo == 0) {
                            break;
                        }
                    }
                }

        }
    return (status);
    }

/*******************************************************************************
*
* pciFindClass - find the nth occurence of a device by PCI class code.
*
* This routine finds the nth device with the given 24-bit PCI class code
* (class subclass prog_if).
*
* RETURNS:
* OK, or ERROR if the class didn't match.
*/

STATUS pciFindClass
    (
    int    classCode,	/* 24-bit class code */
    int	   index,	/* desired instance of device */
    int *  pBusNo,	/* bus number */
    int *  pDeviceNo,	/* device number */
    int *  pFuncNo	/* function number */
    )
    {
    STATUS status = ERROR;
    BOOL   cont   = TRUE;
    int    busNo;
    int    deviceNo;
    int    funcNo;
    UINT32    classCodeReg;
    UINT32    vendor;
    unsigned char   header;

    if (pciLibInitStatus != OK)			/* sanity check */
        return(ERROR);

    for (busNo = 0; cont == TRUE && busNo <= pciMaxBus; busNo++)
        for (deviceNo = 0;
	     ((cont == TRUE) && (deviceNo < PCI_MAX_DEV));
	     ++deviceNo)
            for (funcNo = 0; cont == TRUE && funcNo < PCI_MAX_FUNC; funcNo++)
		{

		/* avoid a special bus cycle */

		if ((deviceNo == 0x1f) && (funcNo == 0x07))
		    continue;

		pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID,
				 &vendor);

		/*
		 * If nonexistent device, skip to next, only look at
		 * vendor ID field for existence check
		 */

		if (((vendor & 0x0000ffff) == 0x0000FFFF) && (funcNo == 0))
		    break;
	
		pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_REVISION,
				 &classCodeReg);

		if ((((classCodeReg >> 8) & 0x00ffffff) == classCode) &&
		    (index-- == 0))
		    {
		    *pBusNo	= busNo;
		    *pDeviceNo	= deviceNo;
		    *pFuncNo	= funcNo;
		    status	= OK;
		    cont 	= FALSE;	/* terminate all loops */
		    continue;
		    }

		/* goto next if current device is single function */

		pciConfigInByte (busNo, deviceNo, funcNo, PCI_CFG_HEADER_TYPE, 
				 &header);
		if ((header & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC &&
		    funcNo == 0)
		    break;
		}

    return (status);
    }

/*******************************************************************************
*
* pciConfigBdfPack - pack parameters for the Configuration Address Register
*
* This routine packs three parameters into one integer for accessing the
* Configuration Address Register
*
* RETURNS: packed integer encoded version of bus, device, and function numbers.
*/

int pciConfigBdfPack
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo    /* function number */
    )
    {
    return (((busNo    << 16) & 0x00ff0000) |
            ((deviceNo << 11) & 0x0000f800) |
            ((funcNo   << 8)  & 0x00000700));
    }

/*******************************************************************************
*
* pciConfigInByte - read one byte from the PCI configuration space
*
* This routine reads one byte from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInByte
    (
    int	busNo,    /* bus number */
    int	deviceNo, /* device number */
    int	funcNo,	  /* function number */
    int	offset,	  /* offset into the configuration space */
    UINT8 * pData /* data read from the offset */
    )
    {
    int	key;
    unsigned char tmpdata;
    STATUS rval;

    if (pciLibInitStatus != OK)			/* sanity check */
        return (ERROR);

    if (busNo > pciMaxBus)
        {
        *pData = 0xff;
        return (OK);
        }

    rval = OK;

    /* Bus 0 is hijacked by the sb bus */

    if(busNo == 0) 
        {
        if (funcNo == 0)
            {
            /* Silicon Backplane only supports IRQ byte and multi func byte */
            switch(offset) 
                {
                case PCI_CFG_HEADER_TYPE:
                    *pData = (UINT8)PCI_HEADER_TYPE0;
                    break;
            
                case PCI_CFG_DEV_INT_LINE:
                    *pData = (UINT8)sbDevices[deviceNo].vx_int_num;
                    break;
        
                default:
                    *pData = 0xff;
                    rval = ERROR;
                    break;
                }
            }
        else
            {
            *pData = 0xff;
            rval = ERROR;
            }
        return (rval);
        }

#ifndef  MIPSEL
    offset ^= 3;
#endif

    if (busNo == 1)
	{
	if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV) 
            {
            key = intLock ();	 
	    pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo)); 
            OSL_DELAY(2);
            if (vxMemProbe ((void *) PCI_CFG_47XX (PCI_IND_DEV, funcNo, offset),
                VX_READ, 1, (char *) &tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xff;

	    pci->sbtopci1 = SBTOPCI_CFG0; 
            OSL_DELAY(2);
            intUnlock (key);
	    } 
        else 
            {
            key = intLock();
            if (vxMemProbe ((void *) PCI_CFG_47XX (deviceNo, funcNo, offset),
                VX_READ, 1, (char *)&tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xff;
            intUnlock(key);
            }
        }
    else 
        {
        key = intLock ();
        pci->sbtopci1 = SBTOPCI_CFG1; 
        OSL_DELAY(2); 
        if (vxMemProbe ( (void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) | 
                pciConfigBdfPack (busNo, deviceNo, funcNo)),
                VX_READ, 1, (char *)&tmpdata) == OK)
            *pData=tmpdata;
        else
            *pData=0xff;
        pci->sbtopci1 = SBTOPCI_CFG0; 
        OSL_DELAY(2);
        intUnlock (key);
        }

    return (rval);
    }

/*******************************************************************************
*
* pciConfigInLong - read one longword from the PCI configuration space
*
* This routine reads one longword from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInLong
    (
    int	busNo,     /* bus number */
    int	deviceNo,  /* device number */
    int	funcNo,    /* function number */
    int	offset,    /* offset into the configuration space */
    UINT32 * pData /* data read from the offset */
    )
    {
    int		 key;
    STATUS rval;
    int	tmpdata;

    /* Check lib initialized */

    if (pciLibInitStatus != OK)
	{
        return (ERROR);
	}

    if (busNo > pciMaxBus)
            {
            *pData = 0xffffffff;
            return (OK);
            }

    rval = OK;

    /* Bus 0 is hijacked by the sb bus */
    if (busNo == 0) 
        {
        if (funcNo == 0)
            /* Silicon Backplane only supports IRQ byte and multi func byte */
            switch(offset) 
                {
                case PCI_CFG_BASE_ADDRESS_0:
                    *pData = (UINT32)sbDevices[deviceNo].bar0;
                    break;
            
                case PCI_CFG_VENDOR_ID:
                    *pData = (UINT32)sbDevices[deviceNo].vendor_devid;
                    break;
        
                case PCI_CFG_REVISION:
                    *pData = (UINT32)sbDevices[deviceNo].classcode;
                    break;

                default:
                    *pData = 0xFFFFFFFF;
                    rval = ERROR;
                    break;
                }
        else
            {
            *pData = 0xFFFFFFFF;
            rval = ERROR;
            }

        return (rval);
        }

    if (busNo == 1)
	{
	if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV) 
            {
            key = intLock ();	 /* mutual exclusion start */
	    pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo)); 
            OSL_DELAY(2);
            if (vxMemProbe ((void *) PCI_CFG_47XX (PCI_IND_DEV, funcNo, offset),
                VX_READ, 4, (char *) &tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xffffffff;

	    pci->sbtopci1 = SBTOPCI_CFG0; 
            OSL_DELAY(2);
            intUnlock (key);
	    } 
        else 
            {
            key = intLock();
            if (vxMemProbe ((void *) PCI_CFG_47XX (deviceNo, funcNo, offset),
                VX_READ, 4, (char *)&tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xffffffff;

            intUnlock(key);
            }
        }
    else 
        {
        key = intLock ();
        pci->sbtopci1 = SBTOPCI_CFG1; 
        OSL_DELAY(2); 
        if (vxMemProbe ( (void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) | 
                pciConfigBdfPack (busNo, deviceNo, funcNo)),
                VX_READ, 4, (char *)&tmpdata) == OK)
            *pData=tmpdata;
        else
            *pData=0xffffffff;

        pci->sbtopci1 = SBTOPCI_CFG0; 
        OSL_DELAY(2);
        intUnlock (key);
        }

    return (OK);
    }

/*******************************************************************************
*
* pciConfigOutLong - write one longword to PCI configuration space
*
* This routine writes one longword to PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutLong
    (
    int	busNo,     /* bus number */
    int	deviceNo,  /* device number */
    int	funcNo,    /* function number */
    int	offset,    /* offset into the configuration space */
    UINT32 data    /* data to write at the offset */
    )
    {
    int		 key;
    STATUS rval;
    int tmpdata;

    if (pciLibInitStatus != OK)
	{
        return (ERROR);
	}

    if(busNo == 0 || busNo > pciMaxBus) 
        {
        /* 
        *  pciConfigOutLong is supplied only for use on the PCI
        *  bus, not the silicon backplane.
        */
        return (ERROR);
        }

    rval = OK;

    tmpdata = data;

    key = intLock ();				/* INTLOCK */

    if (busNo == 1) 
        {
        if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV) 
            {
            pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo)); OSL_DELAY(2);

            if (vxMemProbe((void *)PCI_CFG_47XX(PCI_IND_DEV, funcNo, offset),
                VX_WRITE, 4, (char *)&tmpdata) != OK) 
                rval = ERROR;
            pci->sbtopci1 = SBTOPCI_CFG0; OSL_DELAY(2);
            } 
        else 
            if (vxMemProbe((void *)PCI_CFG_47XX(deviceNo, funcNo, offset),
                VX_WRITE, 4, (char *)&tmpdata) != OK) 
                rval = ERROR;
	}
    else  /* Do Type 1 config transaction on secondary bus */
        {
        pci->sbtopci1 = SBTOPCI_CFG1;
        OSL_DELAY(2);
        if (vxMemProbe ((void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) |
            pciConfigBdfPack (busNo, deviceNo, funcNo)),
            VX_WRITE, 4, (char *) &tmpdata) != OK)
            rval = ERROR;
        pci->sbtopci1 = SBTOPCI_CFG0; 
        OSL_DELAY(2);
        }

    intUnlock (key);			/* UNLOCK INTERRUPTS */

    return (rval);
    }

/*******************************************************************************
*
* pciConfigOutWord - write a 2-byte word to PCI configuration space
*
* This routine writes one longword to PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutWord
    (
    int	busNo,     /* bus number */
    int	deviceNo,  /* device number */
    int	funcNo,    /* function number */
    int	offset,    /* offset into the configuration space */
    UINT16 data    /* data to write at the offset */
    )
    {
    int		 key;
    STATUS rval;
    unsigned short tmpdata;

    if (pciLibInitStatus != OK)
        {
        return (ERROR);
        }

    if(busNo == 0 || busNo > pciMaxBus)
        {
        /*
        *  pciConfigOutWord is supplied only for use on the PCI
        *  bus, not the silicon backplane.
        */
        return (ERROR);
        }

    rval = OK;

    tmpdata = data;

#ifndef  MIPSEL
    offset ^= 2;
#endif

    key = intLock ();			/* mutual exclusion start */

    if (busNo == 1) 
        {
        if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV) 
            {
            pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo)); 
            OSL_DELAY(2);
            if (vxMemProbe((void *)PCI_CFG_47XX(PCI_IND_DEV, funcNo, offset),
                VX_WRITE, 2, (char *)&tmpdata) != OK)
                rval = ERROR; 
            pci->sbtopci1 = SBTOPCI_CFG0; 
            OSL_DELAY(2);
            } 
        else 
            {
            if (vxMemProbe((void *)PCI_CFG_47XX(deviceNo, funcNo, offset),
                VX_WRITE, 2, (char *)&tmpdata) != OK) 
                rval = ERROR;
            }
	}
    else 
        {
        pci->sbtopci1 = SBTOPCI_CFG1;
        OSL_DELAY (1);
        if (vxMemProbe ((void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) |
            pciConfigBdfPack (busNo, deviceNo, funcNo)),
            VX_WRITE, 2, (char *) &tmpdata) != OK)
            rval = ERROR;
        pci->sbtopci1 = SBTOPCI_CFG0; 
        OSL_DELAY(2);
        }

    intUnlock (key);				/* mutual exclusion stop */

    return (rval);
    }

/*******************************************************************************
*
* pciConfigOutByte - write a byte to PCI configuration space
*
* This routine writes one byte to PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutByte
    (
    int	busNo,     /* bus number */
    int	deviceNo,  /* device number */
    int	funcNo,    /* function number */
    int	offset,    /* offset into the configuration space */
    UINT8 data    /* data to write at the offset */
    )
    {
    int		 key;
    STATUS rval;
    UINT8 tmpdata;

    if (pciLibInitStatus != OK)
        {
        return (ERROR);
        }

    if(busNo == 0 || busNo > pciMaxBus)
        {
        /*
        *  pciConfigOutByte is supplied only for use on the PCI
        *  bus, not the silicon backplane.
        */
        return (ERROR);
        }

    rval = OK;

    tmpdata = data;

    key = intLock ();                   /* mutual exclusion start */

#ifndef  MIPSEL
    offset ^= 3;
#endif

    if (busNo == 1)
        {
        if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV)
            {
            pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo));
            OSL_DELAY(2);
            if (vxMemProbe((void *)PCI_CFG_47XX(PCI_IND_DEV, funcNo, offset),
                VX_WRITE, 1, (char *)&tmpdata) != OK)
                rval = ERROR;
            pci->sbtopci1 = SBTOPCI_CFG0;
            OSL_DELAY(2);
            }
        else
            {
            if (vxMemProbe((void *)PCI_CFG_47XX(deviceNo, funcNo, offset),
                VX_WRITE, 1, (char *)&tmpdata) != OK)
                rval = ERROR;
            }
        }
    else
        {
        pci->sbtopci1 = SBTOPCI_CFG1;
        OSL_DELAY (1);
        if (vxMemProbe ((void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) |
            pciConfigBdfPack (busNo, deviceNo, funcNo)),
            VX_WRITE, 1, (char *) &tmpdata) != OK)
            rval = ERROR;
        pci->sbtopci1 = SBTOPCI_CFG0;
        OSL_DELAY(2);
        }

    intUnlock (key);                            /* mutual exclusion stop */

    return (rval);
    }


/*******************************************************************************
*
* pciConfigInWord - read one 2-byte word from the PCI configuration space
*
* This routine reads one 2-byte word from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInWord
    (
    int	busNo,     /* bus number */
    int	deviceNo,  /* device number */
    int	funcNo,    /* function number */
    int	offset,    /* offset into the configuration space */
    UINT16 * pData /* data read from the offset */
    )
    {
    int		 key;
    UINT16 tmpdata;

    /* Check lib initialized */

    if (pciLibInitStatus != OK)
	{
        return (ERROR);
	}

    if (busNo > pciMaxBus)
        {
        *pData = 0xffff;
        return (OK);
        }
 

    /* Bus 0 is hijacked by the sb bus */

    if (busNo == 0)
        {
        if (funcNo == 0)
            {
        /* Silicon Backplane only supports IRQ byte and multi func byte */
            switch(offset) 
                {
                case 0:
                    *pData = (UINT16)
                         (sbDevices[deviceNo].vendor_devid & 0xFFFF);
                     break;

                case 2:
                    *pData = (UINT16)
                        ((sbDevices[deviceNo].vendor_devid >> 16)  & 0xFFFF);
                    break;

                default:
                    *pData = 0xFFFF;
                }
            }
        else
            *pData = 0xFFFF;

        return (OK);
        }
     
#ifndef  MIPSEL
    offset ^= 2;
#endif

    if (busNo == 1)
	{
	if (deviceNo > PCI_IDSEL_ALINE_INDIRECT_DEV) 
            {
            key = intLock ();	 
	    pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +deviceNo)); 
            OSL_DELAY(2);
            if (vxMemProbe((void *)PCI_CFG_47XX(PCI_IND_DEV, funcNo, offset),
                VX_READ, 2, (char *)&tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xffff;

	    pci->sbtopci1 = SBTOPCI_CFG0; 
            OSL_DELAY(2);
            intUnlock (key);
	    } 
        else 
            {
            key = intLock();
            if (vxMemProbe((void *)PCI_CFG_47XX(deviceNo, funcNo, offset),
                VX_READ, 2, (char *)&tmpdata) == OK)
                *pData=tmpdata;
            else
                *pData=0xffff;
            intUnlock(key);
            }
        }
    else
        {
        key = intLock ();
        pci->sbtopci1 = SBTOPCI_CFG1; 
        OSL_DELAY(2); 
        if (vxMemProbe ( (void *) (PCI_CFG_47XX(PCI_IND_DEV, 0, offset) | 
                pciConfigBdfPack (busNo, deviceNo, funcNo)),
                VX_READ, 2, (char *)&tmpdata) == OK)
            *pData=tmpdata;
        else
            *pData=0xffff;
        pci->sbtopci1 = SBTOPCI_CFG0; 
        OSL_DELAY(2);
        intUnlock (key);
        }

    return (OK);
    }



IMPORT void sysExtInt (void);

void
platform_init()
{
#ifndef BROADCOM_BSP
	char *board, *cf;
#endif
    uint32  pci_clock, req_sb, req_pci, cpu_clock;
    int     ext_pci_clk = 0;
    int     ext_pci_arb = 0;
    void    *sbh;
    UINT8 irq;
    
    sbh = sb_kattach(BCM4710_DEVICE_ID, 0);
    ASSERT(sbh);

    extif = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0);
    if (!extif) {
            cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0);
#if 0
	    /* CHIPC: connect the external interface interrupt handler */
	    pciConfigInByte(0, sb_coreidx(sbh), 0, PCI_CFG_DEV_INT_LINE, &irq);
	    (void) intConnect(INUM_TO_IVEC(irq), sysExtInt, 0);
#endif
	    ASSERT(cc);
     }


    pci = (sbpciregs_t *) sb_setcore(sbh, SB_PCI, 0);

#if 0
    /* PCI: connect the external interface interrupt handler */
    pciConfigInByte(0, sb_coreidx(sbh), 0, PCI_CFG_DEV_INT_LINE, &irq);
    (void) intConnect(INUM_TO_IVEC(irq), sysExtInt, 0);
#endif

    
    sysLedDsply("PFIN");

    {
	char buf[100];
	sprintf(buf,"pci = %08x\n", (int)pci);
	sysSerialPrintString(buf);
    }
    sysSerialPrintString(" PCI Init\n");

    
    /*
     * Init the pci interface 
     */
    if (ext_pci_clk) {
        sysSerialPrintString(" External Clock!\n");
        pci->control = 0x1;             /* PCI reset, no clock */
        OSL_DELAY(100);
        pci->control = 0x3;
    } else {
        sysSerialPrintString(" Internal Clock!\n");
        pci->control = 0x5;				/* enable the tristate drivers */
        OSL_DELAY(100);					    /* delay 100 us */
        pci->control = 0xd;				/* enable the PCI clock */
        OSL_DELAY(100);					    /* delay 100 us */
        pci->control = 0xf;				/* deassert PCI reset */
    }

    pci->arbcontrol = (ext_pci_arb) ? PCI_EXT_ARB : PCI_INT_ARB;
    OSL_DELAY(2);					/* delay 1 us */

sysSerialPrintString(" mask PCI interrupts\n");
#define MAPPED_ADDR	0xa8000000

	/* mask off pci core interrupts */
    pci->intmask &= ~(0xffff);

sysSerialPrintString(" map PCI space\n");
	/* set up SB to PCI windows for memory mapped device.  
	For io mapped device, set only SBTOPCI_IO to stbopci0
	and the base register address is the value put in pci 
	device configuration register 0x10 plus 0xa8000000 */
	pci->sbtopci0 = SBTOPCI_MEM | MAPPED_ADDR; OSL_DELAY(2);
	pci->sbtopci1 = SBTOPCI_CFG0; OSL_DELAY(2);

sysSerialPrintString(" first PCI config access!\n");
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x04)) = 0xffffffff; OSL_DELAY(2);

sysSerialPrintString(" first PCI config access ok!\n");
	/* Enable 2 func's and enable func1 bar1 with size 128M */ 
	*(uint32 *)&pci->sprom[0x00] = 0x03;
	*(uint32 *)&pci->sprom[0x10] = 0x0f;


sysSerialPrintString(" second PCI config access \n");
	/* Func 0, BAR0 = 0xffff0000, BAR1 = 0 */
    *(unsigned long*)(PCI_CFG_47XX(4, 0, 0x4)) = 0x146;
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x10)) = MAPPED_ADDR + 0x4000; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x14)) = 0xa0000000; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x3c)) = 0x00000101; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x84)) = 0; OSL_DELAY(2);

sysSerialPrintString(" func1 PCI config access \n");
    /* Func 1, BAR0 = 0xffff2000, BAR1 = PCI SWAP REGION */
    *(unsigned long*)(PCI_CFG_47XX(4, 1, 0x4)) = 0x146;
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x04)) = 0xffffffff; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x10)) = MAPPED_ADDR + 0x6000; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x14)) = BCM4704_SDRAM_SWAPPED; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x3c)) = 0x00000101; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x84)) = BCM4704_SDRAM_SWAPPED; OSL_DELAY(2);

sysSerialPrintString(" enable prefetch PCI config access \n");
    /* Enable prefetch & burst for f0 & f1's bar1*/
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x8c)) = 0x03; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x8c)) = 0x03; OSL_DELAY(2);
sysSerialPrintString(" PCI \n");

	pci->intmask |= PCI_INTA + PCI_INTB;
    sb_detach(sbh);
sysSerialPrintString(" PCI done\n");

#define SB_MIPS33_CORE_BA   0xb8005000
#define SB_CHIPC_CORE_BA    0xb8000000
    *((volatile unsigned long*)(SB_MIPS33_CORE_BA + 0xfa8)) = 0x00050076; /* Bump up the timeout */
    *((volatile unsigned long*)(SB_CHIPC_CORE_BA + 0x50)) = 0xfd8;
    *((volatile unsigned long*)(SB_CHIPC_CORE_BA + 0x54)) = 0x0;
}

void
sysPciHostBridgeReadBurstDisable()
{
    /* Disable prefetch & burst for f0 & f1's bar1*/
	*(unsigned long*)(PCI_CFG_47XX(4, 0, 0x8c)) = 0x01; OSL_DELAY(2);
	*(unsigned long*)(PCI_CFG_47XX(4, 1, 0x8c)) = 0x01; OSL_DELAY(2);
}

void unmask_pci_interrupt(unsigned long bit)
{
	int s;
	s = intLock();
	pci->intmask |= bit;
	intUnlock(s);
}

void mask_pci_interrupt(unsigned long bit)
{
	int s;
	s = intLock();
	pci->intmask &= ~bit;
	intUnlock(s);
}

#endif /* defined(INCLUDE_PCI) */


void pctest(int d, int f, int o,int ed,int count)
{
	int fc=0;
	int pc=0;
	int nt;
	unsigned int data;
	int vd;
	unsigned int pd;
	unsigned int err;

	printf("d = %d f = %d o = %d ed = %08x\n",d,f,o,ed);
	vd = d;
	if (d > PCI_IDSEL_ALINE_INDIRECT_DEV) {
	  pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +d)); OSL_DELAY(2);
	  vd = 10;
	}

	if (ed  == 0xFFFFFFFF) {
	/* 	ed = *(volatile unsigned *)(PCI_CFG_47XX(vd, f, o)); */
	 vxMemProbe((void *)PCI_CFG_47XX(vd, f, o),
                       VX_READ, 4, (char *)&ed);
	}

	err=ed;
	for(;count>0;count--) {
	fc =0;
	pc=0;
	for(nt=256;nt>0;nt--) {
		/* data = *(volatile unsigned *)(PCI_CFG_47XX(vd, f, o)); */
                 vxMemProbe((void *)PCI_CFG_47XX(vd, f, o),
                       VX_READ, 4, (char *)&data);
		if (ed != data) {
			err=data; fc++;
			 vxMemProbe((void *)PCI_CFG_47XX((vd+1), f, o),
			       VX_READ, 4, (char *)&data);
			} else pc++;
	}

	printf("pass count = %d Fail Count = %d Expected = %08x Error data = %08x\n",
	pc,
	fc,
	ed,
	err
	);
	}
	pci->sbtopci1 = SBTOPCI_CFG0; OSL_DELAY(2);
}


void pcteste(int d, int f, int o,int ed)
{
	int fc=0;
	int pc=0;
	int nt=256;
	unsigned int data;
	int vd;
	unsigned int pd;
	unsigned int err;

	printf("d = %d f = %d o = %d ed = %08x\n",d,f,o,ed);
	vd = d;
	if (d > PCI_IDSEL_ALINE_INDIRECT_DEV) {
	  pci->sbtopci1 = SBTOPCI_CFG0 | (1<<(PCI_IDSEL_ALINE_START +d)); OSL_DELAY(2);
	  vd = 10;
	}

	if (ed  == 0xFFFFFFFF) {
		ed = *(volatile unsigned *)(PCI_CFG_47XX(vd, f, o));
	 /* vxMemProbe((void *)PCI_CFG_47XX(vd, f, o),
                       VX_READ, 4, (char *)&ed); */
	}

	err=ed;
	for(nt=256;nt>0;nt--) {
		data = *(volatile unsigned *)(PCI_CFG_47XX(vd, f, o));
                 /* vxMemProbe((void *)PCI_CFG_47XX(vd, f, o),
                       VX_READ, 4, (char *)&data); */
		if (ed != data) { err=data; fc++;} else pc++;
	}
	pci->sbtopci1 = SBTOPCI_CFG0; OSL_DELAY(2);

	printf("pass count = %d Fail Count = %d Expected = %08x Error data = %08x\n",
	pc,
	fc,
	ed,
	err
	);
}
