/*
 * $Id: schan.c 1.10.6.3 Broadcom SDK $
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
 * S-Channel (internal command bus) support
 */

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/core/dpc.h>

#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmic.h>

#ifdef BCM_TRIUMPH2_SUPPORT
#include <soc/triumph2.h>
#endif
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#if defined(PLISIM) && defined(PLISIM_DIRECT_SCHAN)
/* Back door into the simulation to perform direct SCHANNEL operations */
extern uint32 plibde_schan_op(int unit, schan_msg_t* msg, int, int);
#endif

/*
 * S-Chanel operation names
 */

STATIC char *_soc_schan_op_names[] = {
    "UNKNOWN_OPCODE",
    "BP_WARN_STATUS",           /* 0x01 */
    "BP_DISCARD_STATUS",        /* 0x02 */
    "COS_QSTAT_NOTIFY",         /* 0x03 */
    "HOL_STAT_NOTIFY",          /* 0x04 */
    "",                         /* 0x05 */
    "",                         /* 0x06 */
    "READ_MEM_CMD",             /* 0x07 */
    "READ_MEM_ACK",             /* 0x08 */
    "WRITE_MEM_CMD",            /* 0x09 */
    "WRITE_MEM_ACK",            /* 0x0a */
    "READ_REG_CMD",             /* 0x0b */
    "READ_REG_ACK",             /* 0x0c */
    "WRITE_REG_CMD",            /* 0x0d */
    "WRITE_REG_ACK",            /* 0x0e */
    "ARL_INSERT_CMD",           /* 0x0f */
    "ARL_INSERT_DONE",          /* 0x10 */
    "ARL_DELETE_CMD",           /* 0x11 */
    "ARL_DELETE_DONE",          /* 0x12 */
    "LINKSTAT_NOTIFY",          /* 0x13 */
    "MEM_FAIL_NOTIFY",          /* 0x14 */
    "INIT_CFAP",                /* 0x15 */
    "",                         /* 0x16 */
    "ENTER_DEBUG_MODE",         /* 0x17 */
    "EXIT_DEBUG_MODE",          /* 0x18 */
    "ARL_LOOKUP_CMD",           /* 0x19 */
    "L3_INSERT_CMD",            /* 0x1a */
    "L3_INSERT_DONE",           /* 0x1b */
    "L3_DELETE_CMD",            /* 0x1c */
    "L3_DELETE_DONE",           /* 0x1d */
    "L3_LOOKUP_CMD",            /* 0x1e */
    "UNKNOWN_OPCODE",           /* 0x1f */
    "L2_LOOKUP_CMD_MSG",        /* 0x20 */
    "L2_LOOKUP_ACK_MSG",        /* 0x21 */
    "L3X2_LOOKUP_CMD_MSG",      /* 0x22 */
    "L3X2_LOOKUP_ACK_MSG",      /* 0x23 */
    "TABLE_INSERT_CMD_MSG",     /* 0x24 */
    "TABLE_INSERT_DONE_MSG",    /* 0x25 */
    "TABLE_DELETE_CMD_MSG",     /* 0x26 */
    "TABLE_DELETE_DONE_MSG",    /* 0x27 */
    "TABLE_LOOKUP_CMD_MSG",     /* 0x28 */
    "TABLE_LOOKUP_DONE_MSG",    /* 0x29 */
    "FIFO_POP_CMD_MSG",         /* 0x2a */
    "FIFO_POP_DONE_MSG",        /* 0x2b */
    "FIFO_PUSH_CMD_MSG",        /* 0x2c */
    "FIFO_PUSH_DONE_MSG",       /* 0x2d */
};

char *
soc_schan_op_name(int op)
{
    if (op < 0 || op >= COUNTOF(_soc_schan_op_names)) {
        op = 0;
    }

    return _soc_schan_op_names[op];
}

#if defined(BCM_TRIUMPH_SUPPORT)
STATIC char *_soc_schan_gen_resp_type_names[] = {
    "FOUND",          /*  0 */
    "NOT_FOUND",      /*  1 */
    "FULL",           /*  2 */
    "INSERTED",       /*  3 */
    "REPLACED",       /*  4 */
    "DELETED",        /*  5 */
    "ENTRY_IS_OLD",   /*  6 */
    "CLEARED_VALID",  /*  7 */
    "L2_FIFO_FULL",   /*  8 */
    "MAC_LIMIT_THRE", /*  9 */
    "MAC_LIMIT_DEL",  /* 10 */
    "L2_STATIC",      /* 11 */
    "UNKNOWN",        /* 12 */
    "UNKNOWN",        /* 13 */
    "UNKNOWN",        /* 14 */
    "ERROR"           /* 15 */
};

STATIC char *
soc_schan_gen_resp_type_name(int type)
{
    if (type < 0 || type >= COUNTOF(_soc_schan_gen_resp_type_names)) {
        type = 8;
    }

    return _soc_schan_gen_resp_type_names[type];
}

STATIC char *_soc_schan_gen_resp_err_names[] = {
    "NONE",            /* 0 */
    "SRAM_P_ERR",      /* 1 */
    "TCAM_SRCH_ERR",   /* 2 */
    "MULTIPLE",        /* 3 */
    "TCAM_RD_ERR",     /* 4 */
    "MULTIPLE",        /* 5 */
    "MULTIPLE",        /* 6 */
    "MULTIPLE",        /* 7 */
    "TCAM_SEQ_ERR",    /* 8 */
    "MULTIPLE",        /* 9 */
    "MULTIPLE",        /* 10 */
    "MULTIPLE",        /* 11 */
    "MULTIPLE",        /* 12 */
    "MULTIPLE",        /* 13 */
    "MULTIPLE",        /* 14 */
    "MULTIPLE",        /* 15 */
};

STATIC char *
soc_schan_gen_resp_err_name(int err)
{
    if (err < 0 || err >= COUNTOF(_soc_schan_gen_resp_err_names)) {
        err = 2;
    }

    return _soc_schan_gen_resp_err_names[err];
}

#endif /* BCM_TRIUMPH_SUPPORT */
/*
 * Resets the CMIC S-Channel interface. This is required when we sent
 * a message and did not receive a response after the poll count was
 * exceeded.
 */

STATIC void
_soc_schan_reset(int unit)
{
    uint32 val;

#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);

    if(soc_feature(unit, soc_feature_cmicm)) {
        val = soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc));
        /* Toggle S-Channel abort bit in CMIC_SCHAN_CTRL register */
        soc_pci_write(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc), val | SC_CMCx_SCHAN_ABORT);
        SDK_CONFIG_MEMORY_BARRIER; 
        soc_pci_write(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc), val);
        SDK_CONFIG_MEMORY_BARRIER;
    } else 
#endif /* CMICM Support */
    {
        val = soc_pci_read(unit, CMIC_CONFIG);

        /* Toggle S-Channel abort bit in CMIC_CONFIG register */
        soc_pci_write(unit, CMIC_CONFIG, val | CC_SCHAN_ABORT);

        SDK_CONFIG_MEMORY_BARRIER; 

        soc_pci_write(unit, CMIC_CONFIG, val);

        SDK_CONFIG_MEMORY_BARRIER;
    }

    if (SAL_BOOT_QUICKTURN) {
        /* Give Quickturn at least 2 cycles */
        sal_usleep(10 * MILLISECOND_USEC);
    }
}

/*
 * Dump an S-Channel message for debugging
 */

void
soc_schan_dump(int unit, schan_msg_t *msg, int dwc)
{
    char                buf[128];
    int                 i, j;

    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        soc_cm_print("  HDR[CPU=%d COS=%d EBIT=%d ECODE=%d "
                     "L=%d ACC_TYPE=%d DST=%d OP=%d=%s]\n",
                     msg->header.cpu, msg->header.cos,
                     msg->header.ebit, msg->header.ecode,
                     msg->header.datalen, msg->header.srcblk,
                     msg->header.dstblk, msg->header.opcode,
                     soc_schan_op_name(msg->header.opcode));
    } else {
        soc_cm_print("  HDR[CPU=%d COS=%d EBIT=%d ECODE=%d "
                     "L=%d SRC=%d DST=%d OP=%d=%s]\n",
                     msg->header.cpu, msg->header.cos,
                     msg->header.ebit, msg->header.ecode,
                     msg->header.datalen, msg->header.srcblk,
                     msg->header.dstblk, msg->header.opcode,
                     soc_schan_op_name(msg->header.opcode));
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops) &&
        ((msg->header.opcode == TABLE_INSERT_DONE_MSG) ||
         (msg->header.opcode == TABLE_DELETE_DONE_MSG) ||
         (msg->header.opcode == TABLE_LOOKUP_DONE_MSG))) {
        soc_cm_print("  RSP[SRC=%d TYPE=%d=%s ERR_INFO=%d=%s "
                     "INDEX=0x%05x]\n",
                     msg->genresp.response.src,
                     msg->genresp.response.type,
                     soc_schan_gen_resp_type_name(msg->genresp.response.type),
                     msg->genresp.response.err_info,
                     soc_schan_gen_resp_err_name(msg->genresp.response.err_info),
                     msg->genresp.response.index);
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    assert(dwc <= CMIC_SCHAN_WORDS(unit));

    for (i = 0; i < dwc; i += 4) {
        buf[0] = 0;

        for (j = i; j < i + 4 && j < dwc; j++) {
            sal_sprintf(buf + sal_strlen(buf),
                        " DW[%2d]=0x%08x", j, msg->dwords[j]);
        }

        soc_cm_print(" %s\n", buf);
    }
}


#if defined(BCM_XGS12_SWITCH_SUPPORT)
/*
 * _soc_schan_op_ipic_read
 *
 * Handler routine for certain XGS IPIC stats counters
 *
 */

STATIC int
_soc_schan_op_ipic_read(int unit,
                        schan_msg_t *msg,
                        int dwc_write, int dwc_read,
                        int intr)
{
#define MAX_SAMPLES 5
    int i, timeout, retry, max_retries, samples, rv;
    uint32 last_read_data[2];
    schan_msg_t ack_msg;

    assert(! sal_int_context());
    assert(dwc_write <= CMIC_SCHAN_WORDS(unit));
    assert(dwc_read <= CMIC_SCHAN_WORDS(unit));

    timeout = 1000;  /* microseconds (quick timeout) */
    samples = MAX_SAMPLES; 
    retry = 0;
    max_retries = -1;
    last_read_data[0] = last_read_data[1] = 0;

    SCHAN_LOCK(unit);

    if (soc_cm_debug_check(DK_SCHAN)) {
        soc_cm_print("S-CHANNEL %s: (unit %d)\n",
                     soc_schan_op_name(msg->header.opcode), unit);
        soc_schan_dump(unit, msg, dwc_write);
    }

    do {
        rv = SOC_E_NONE; 

        /* Write raw S-Channel Data: dwc_write words */

        for (i = 0; i < dwc_write; i++) {
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), msg->dwords[i]);
        }

        /* Tell CMIC to start */

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_START_SET);

        /* Wait for completion using either the interrupt or polling method */

        if (intr && SOC_CONTROL(unit)->schanIntrEnb) {
            soc_intr_enable(unit, IRQ_SCH_MSG_DONE);

            if (sal_sem_take(SOC_CONTROL(unit)->schanIntr,
                             timeout) != 0) {
                rv = SOC_E_TIMEOUT;
            } else {
                soc_cm_debug(DK_SCHAN, "  Interrupt received\n");
                if (SOC_CONTROL(unit)->schan_result & SC_MSG_NAK_TST) {
                    rv = SOC_E_FAIL;
                }
            }

            soc_intr_disable(unit, IRQ_SCH_MSG_DONE);
        } else {
            soc_timeout_t to;
            uint32 schanCtrl;

            soc_timeout_init(&to, timeout, 100);

            while (((schanCtrl = soc_pci_read(unit, CMIC_SCHAN_CTRL)) &
                    SC_MSG_DONE_TST) == 0) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }

            if (rv == SOC_E_NONE) {
                soc_cm_debug(DK_SCHAN, "  Done in %d polls\n", to.polls);
            }

            if (schanCtrl & SC_MSG_NAK_TST) {
                soc_cm_debug(DK_SCHAN, "  NAK received from SCHAN.\n");
                rv = SOC_E_FAIL;
            }

            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);
        }

        if (rv == SOC_E_TIMEOUT) {
	    if (max_retries < 0) {
		max_retries = soc_property_get(unit, spn_SOC_CTR_MAXERR, 5);
	    }
            soc_cm_debug(DK_VERBOSE,
                         "soc_schan_op: IPIC reg_read operation timed out"
                         " (attempt %d of %d)\n", 1+retry,
                         1+max_retries);
            SOC_CONTROL(unit)->stat.err_sc_tmo++;
            _soc_schan_reset(unit);
            if ((++retry) > max_retries) {
                /* Exhausted all retries; return error code */
                break;
            }
            continue; /* Don't read data if command write timed out */ 
        }

        /* Read in data from S-Channel buffer space, if any */

        for (i = 0; i < dwc_read; i++) {
            ack_msg.dwords[i] = 
              soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
        }

        if (soc_cm_debug_check(DK_SCHAN)) {
            soc_schan_dump(unit, &ack_msg, dwc_read);
        }

        
        SOC_CONTROL(unit)->stat.schan_op++;

        if ((samples--) == MAX_SAMPLES) {
            last_read_data[0] = ack_msg.dwords[1];
            last_read_data[1] = ack_msg.dwords[2];
        }
        else if ( (ack_msg.dwords[1] != last_read_data[0]) ||
                  (ack_msg.dwords[2] != last_read_data[1]) ) {
            break;
        }

    } while ( samples || ((rv != SOC_E_NONE) && retry) );

    SCHAN_UNLOCK(unit);

    if (rv == SOC_E_TIMEOUT) {
        if (soc_cm_debug_check(DK_ERR)) {
            soc_cm_print("soc_schan_op: operation timed out\n");
            soc_schan_dump(unit, msg, dwc_write);
        }
    }
    else {
        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = ack_msg.dwords[i];
        }
    }

    return rv;
}

/*
 * _soc_schan_op_ipic_write
 *
 * Handler routine for certain XGS IPIC stats counters
 *
 */

STATIC int
_soc_schan_op_ipic_write(int unit,
                         schan_msg_t *msg,
                         int dwc_write, int dwc_read,
                         int intr)
{
    int i, timeout, retry, max_retries, rv;
    uint32 previous_irq_mask;

    assert(! sal_int_context());
    assert(dwc_write <= CMIC_SCHAN_WORDS(unit));
    assert(dwc_read <= CMIC_SCHAN_WORDS(unit));

    timeout = 1000; /* microseconds (quick timeout) */
    retry = 0;
    max_retries = -1;

    SCHAN_LOCK(unit);

    /* Disable the s-channel error ISR; will process errors here */
    previous_irq_mask = soc_intr_disable(unit, IRQ_SCHAN_ERR);

    if (soc_cm_debug_check(DK_SCHAN)) {
        soc_cm_print("S-CHANNEL %s: (unit %d)\n",
                     soc_schan_op_name(msg->header.opcode), unit);
        soc_schan_dump(unit, msg, dwc_write);
    }

    do {
        rv = SOC_E_NONE; 

        /* Write raw S-Channel Data: dwc_write words */

        for (i = 0; i < dwc_write; i++) {
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), msg->dwords[i]);
        }

        /* Tell CMIC to start */

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_START_SET);

        /* Wait for completion using either the interrupt or polling method */

        if (intr && SOC_CONTROL(unit)->schanIntrEnb) {
            soc_intr_enable(unit, IRQ_SCH_MSG_DONE);

            if (sal_sem_take(SOC_CONTROL(unit)->schanIntr,
                             timeout) != 0) {
                rv = SOC_E_TIMEOUT;
            } else {
                soc_cm_debug(DK_SCHAN, "  Interrupt received\n");
                if (SOC_CONTROL(unit)->schan_result & SC_MSG_NAK_TST) {
                    rv = SOC_E_FAIL;
                }
            }

            soc_intr_disable(unit, IRQ_SCH_MSG_DONE);
        } else {
            soc_timeout_t to;
            uint32 schanCtrl;

            soc_timeout_init(&to, timeout, 100);

            while (((schanCtrl = soc_pci_read(unit, CMIC_SCHAN_CTRL)) &
                    SC_MSG_DONE_TST) == 0) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }

            if (rv == SOC_E_NONE) {
                soc_cm_debug(DK_SCHAN, "  Done in %d polls\n", to.polls);
            }

            if (schanCtrl & SC_MSG_NAK_TST) {
                soc_cm_debug(DK_SCHAN, "  NAK received from SCHAN.\n");
                rv = SOC_E_FAIL;
            }

            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);
        }

        if (rv == SOC_E_TIMEOUT) {
	    if (max_retries < 0) {
		max_retries = soc_property_get(unit, spn_SOC_CTR_MAXERR, 5);
	    }
            soc_cm_debug(DK_VERBOSE,
                         "soc_schan_op: IPIC reg_write operation timed out"
                         " (attempt %d of %d)\n", 1+retry,
                         1+max_retries);
            SOC_CONTROL(unit)->stat.err_sc_tmo++;
            _soc_schan_reset(unit);
            if ((++retry) > max_retries) {
                /* Exhausted all retries; return error code */
                break;
            }
            continue; /* Don't read data if command write timed out */ 
        }
        else if ( SCERR_VALID(soc_pci_read(unit, CMIC_SCHAN_ERR)) ) {
	    soc_pci_write(unit, CMIC_SCHAN_ERR, 0); /* Clears intr in Lynx*/
	    if (max_retries < 0) {
		max_retries = soc_property_get(unit, spn_SOC_CTR_MAXERR, 5);
	    }
	    soc_cm_debug(DK_VERBOSE,
                         "soc_schan_op: IPIC reg_write operation ACK error"
                         " (attempt %d of %d)\n", 1+retry,
                         1+max_retries);
            rv = SOC_E_INTERNAL;
            if ((++retry) > max_retries) {
                /* Exhausted all retries; will return error code */
                break;
            }
            continue; /* Don't read data if command write timed out */
        }
         
        /* Read in data from S-Channel buffer space, if any */

        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
        }

        if (soc_cm_debug_check(DK_SCHAN)) {
            soc_schan_dump(unit, msg, dwc_read);
        }

        SOC_CONTROL(unit)->stat.schan_op++;

    } while ((rv != SOC_E_NONE) && retry);

    /* Restore the s-channel error IRQ mask state */
    soc_intr_enable(unit, previous_irq_mask);

    SCHAN_UNLOCK(unit);

    if (rv == SOC_E_TIMEOUT) {
        if (soc_cm_debug_check(DK_ERR)) {
            soc_cm_print("soc_schan_op: operation timed out\n");
            soc_schan_dump(unit, msg, dwc_write);
        }
    }

    return rv;
}
#endif /* BCM_XGS12_SWITCH_SUPPORT */

/*
 * soc_schan_op
 *
 * Writes a message of dwc_write DWORDs from msg to the S-Channel, waits
 * for the operation to complete, then reads dwc_read DWORDs from the
 * channel into msg.  If there is no return data, use dwc_read = 0.
 *
 * On platforms where it is appropriate, the S-Channel is locked during
 * the operation to prevent multiple tasks trying to use the S-Channel
 * simultaneously.
 *
 * Return value is negative on error, 0 on success.
 *
 * If intr is true, this routine goes to sleep until an S-Channel
 * completion interrupt wakes it up.  Otherwise, it polls for the done
 * bit.  NOTE: if schanIntrEnb is false, intr is always overridden to 0.
 *
 * Polling is more CPU efficient for most operations since they complete
 * much faster than the interrupt processing time would take.  However,
 * due to the chip design, some operations such as ARL insert and delete
 * may have unbounded response time.  In this case, the interrupt should
 * be used.  The worst case we have seen is 1 millisec when the switch
 * is passing max traffic with random addresses at min packet size.
 */

int
soc_schan_op(int unit,
             schan_msg_t *msg,
             int dwc_write, int dwc_read,
             int intr)
{
    int i, rv;
    soc_control_t *soc;    
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif
    
    assert(! sal_int_context());
    assert(dwc_write <= CMIC_SCHAN_WORDS(unit));
    assert(dwc_read <= CMIC_SCHAN_WORDS(unit));

    soc = SOC_CONTROL(unit);

#ifdef INCLUDE_RCPU
    if ((SOC_IS_RCPU_ONLY(unit) || SOC_IS_RCPU_UNIT(unit)) && (NULL != soc->soc_rcpu_schan_op)) {
        return soc->soc_rcpu_schan_op(unit, msg, dwc_write, dwc_read);
    }
#endif /* INCLUDE_RCPU */

#if defined(PLISIM) && defined(PLISIM_DIRECT_SCHAN)
    /* Back door into the simulation to perform direct SCHANNEL operations */
    return plibde_schan_op(unit, msg, dwc_write, dwc_read);
#endif

    if (SOC_HW_ACCESS_DISABLE(unit)) {
        switch (msg->header.opcode) {
        case WRITE_MEMORY_CMD_MSG:
            return SOC_E_NONE;
        case WRITE_REGISTER_CMD_MSG:
            msg->header.opcode = WRITE_REGISTER_ACK_MSG;
            return SOC_E_NONE;
        case ARL_INSERT_CMD_MSG:
            msg->header.opcode = ARL_INSERT_DONE_MSG;
            return SOC_E_NONE;
        case ARL_DELETE_CMD_MSG:
            msg->header.opcode = ARL_DELETE_DONE_MSG;
            return SOC_E_NONE;
        case L3_INSERT_CMD_MSG:
            msg->header.opcode = L3_INSERT_DONE_MSG;
            return SOC_E_NONE;
        case L3_DELETE_CMD_MSG:
            msg->header.opcode = L3_DELETE_DONE_MSG;
            return SOC_E_NONE;
        case INIT_CFAP_MSG:
            return SOC_E_NONE;
        case READ_REGISTER_CMD_MSG:
            if (SOC_IS_DETACHING(unit)) {
                sal_memset(msg->bytes, 0, 
                           sizeof(uint32) * CMIC_SCHAN_WORDS_ALLOC);
                msg->header.opcode = READ_REGISTER_ACK_MSG;
                return SOC_E_NONE;
            }
            break;
        case READ_MEMORY_CMD_MSG:
            if (SOC_IS_DETACHING(unit)) {
                sal_memset(msg->bytes, 0, 
                           sizeof(uint32) * CMIC_SCHAN_WORDS_ALLOC);
                msg->header.opcode = READ_MEMORY_ACK_MSG;
                return SOC_E_NONE;
            } 
            break;
        case L2_LOOKUP_CMD_MSG:
            if (SOC_IS_DETACHING(unit)) {
                msg->header.opcode = L2_LOOKUP_ACK_MSG;
                return SOC_E_NOT_FOUND;
            } 
            break;
        case L3X2_LOOKUP_CMD_MSG:
            if (SOC_IS_DETACHING(unit)) {
                msg->header.opcode = L3X2_LOOKUP_ACK_MSG;
                return SOC_E_NOT_FOUND;
            } 
            break;
        case ARL_LOOKUP_CMD_MSG: 
        case L3_LOOKUP_CMD_MSG:
            if (SOC_IS_DETACHING(unit)) {
                return SOC_E_NOT_FOUND;
            } 
            break;
#ifdef BCM_TRIUMPH_SUPPORT
        case TABLE_INSERT_CMD_MSG:
            msg->header.opcode = TABLE_INSERT_DONE_MSG;
            return SOC_E_NONE;
        case TABLE_DELETE_CMD_MSG:
            msg->header.opcode = TABLE_DELETE_DONE_MSG;
            return SOC_E_NONE;
        case FIFO_POP_CMD_MSG:
            msg->header.opcode = FIFO_POP_DONE_MSG;
            return SOC_E_NONE;
        case FIFO_PUSH_CMD_MSG:
            msg->header.opcode = FIFO_PUSH_DONE_MSG;
            return SOC_E_NONE;
#endif  /* BCM_TRIUMPH_SUPPORT */
        default:
            break;
        }
    }

    /* Certain IPIC counters require special handling */

    if (SOC_IS_DRACO15(unit)) {
#if defined(BCM_DRACO15_SUPPORT)
        if ( (msg->readcmd.address >= 0x00c0005d) &&
             (msg->readcmd.address <= 0x00c0007a) ) {
            if (msg->header.opcode == READ_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_read(unit, msg, dwc_write, 
                                               dwc_read, intr);
            }
            else if (msg->header.opcode == WRITE_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_write(unit, msg, dwc_write, 
                                                dwc_read, intr);
            }
        }
#endif /* BCM_DRACO15_SUPPORT */
    }
#if defined(BCM_DRACO_SUPPORT)
    else if (SOC_IS_DRACO(unit)) {
        if ( (msg->readcmd.address >= 0x00c0004d) &&
             (msg->readcmd.address <= 0x00c00064) ) {
            if (msg->header.opcode == READ_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_read(unit, msg, dwc_write, 
                                               dwc_read, intr);
            }
            else if (msg->header.opcode == WRITE_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_write(unit, msg, dwc_write, 
                                                dwc_read, intr);
            }
        }
    }
#endif /* BCM_DRACO_SUPPORT */
#if defined(BCM_TUCANA_SUPPORT)
    else if (SOC_IS_TUCANA(unit)) {
        if ( (msg->readcmd.address >= 0x00c0005d) &&
             (msg->readcmd.address <= 0x00c00066) ) {
            if (msg->header.opcode == READ_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_read(unit, msg, dwc_write, 
                                               dwc_read, intr);
            }
            else if (msg->header.opcode == WRITE_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_write(unit, msg, dwc_write, 
                                                dwc_read, intr);
            }
        }
    }
#endif /* BCM_TUCANA_SUPPORT */
#if defined(BCM_LYNX_SUPPORT)
    else if (SOC_IS_LYNX(unit)) {
        if ( (msg->readcmd.address >= 0x001001d0) &&
             (msg->readcmd.address <= 0x001001dc) ) {
            if (msg->header.opcode == READ_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_read(unit, msg, dwc_write, 
                                               dwc_read, intr);
            }
            else if (msg->header.opcode == WRITE_REGISTER_CMD_MSG) {
                return _soc_schan_op_ipic_write(unit, msg, dwc_write, 
                                                dwc_read, intr);
            }
        }
    }
#endif /* BCM_LYNX_SUPPORT */

    SCHAN_LOCK(unit);

    if (soc_cm_debug_check(DK_SCHAN)) {
        soc_cm_print("S-CHANNEL %s: (unit %d)\n",
                     soc_schan_op_name(msg->header.opcode), unit);
        soc_schan_dump(unit, msg, dwc_write);
    }

    do {
        rv = SOC_E_NONE; 

        /* Write raw S-Channel Data: dwc_write words */

        for (i = 0; i < dwc_write; i++) {
#ifdef BCM_CMICM_SUPPORT
            if (soc_feature(unit, soc_feature_cmicm)) {
                soc_pci_write(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i), msg->dwords[i]);
            } else 
#endif /* CMICM Support */
            {
                soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), msg->dwords[i]);
            }
        }

        /* Tell CMIC to start */
#ifdef BCM_CMICM_SUPPORT
        if (soc_feature(unit, soc_feature_cmicm)) {
            soc_pci_write(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc),
                soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc)) | SC_CMCx_MSG_START);
        } else 
#endif /* CMICM Support */
        {
            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_START_SET);
        }

        /* Wait for completion using either the interrupt or polling method */

        if (intr && SOC_CONTROL(unit)->schanIntrEnb) {
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                soc_cmicm_intr0_enable(unit, IRQ_CMCx_SCH_OP_DONE);
            } else
#endif
            {
                soc_intr_enable(unit, IRQ_SCH_MSG_DONE);
            }
            if (sal_sem_take(SOC_CONTROL(unit)->schanIntr,
                             SOC_CONTROL(unit)->schanTimeout) != 0) {
                rv = SOC_E_TIMEOUT;
            } else {
                soc_cm_debug(DK_SCHAN, "  Interrupt received\n");
#ifdef BCM_CMICM_SUPPORT
                if(soc_feature(unit, soc_feature_cmicm)) {
                    if (SOC_CONTROL(unit)->schan_result & SC_CMCx_MSG_NAK) {
                        rv = SOC_E_FAIL;
                    }
                    if (soc_feature(unit, soc_feature_schan_hw_timeout)) {
                        if (SOC_CONTROL(unit)->schan_result & SC_CMCx_MSG_TIMEOUT_TST) {
                            rv = SOC_E_TIMEOUT;
                        }
                    }
                } else
#endif
                {
                    if (SOC_CONTROL(unit)->schan_result & SC_MSG_NAK_TST) {
                        rv = SOC_E_FAIL;
                    }
                    if (soc_feature(unit, soc_feature_schan_hw_timeout)) {
                        if (SOC_CONTROL(unit)->schan_result & SC_MSG_TIMEOUT_TST) {
                            rv = SOC_E_TIMEOUT;
                        }
                    }
                }
            }

#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                soc_cmicm_intr0_disable(unit, IRQ_CMCx_SCH_OP_DONE);
            } else
#endif
            {
                soc_intr_disable(unit, IRQ_SCH_MSG_DONE);
            }
        } else {
            soc_timeout_t to;
            uint32 schanCtrl;

            soc_timeout_init(&to, SOC_CONTROL(unit)->schanTimeout, 100);

#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                while (((schanCtrl = soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc))) &
                        SC_CMCx_MSG_DONE) == 0) {
                    if (soc_timeout_check(&to)) {
                        rv = SOC_E_TIMEOUT;
                        break;
                    }
                }
                if (rv == SOC_E_NONE) {
                    soc_cm_debug(DK_SCHAN, "  Done in %d polls\n", to.polls);
                }
                if (schanCtrl & SC_CMCx_MSG_NAK) {
                    rv = SOC_E_FAIL;
                    soc_cm_debug(DK_SCHAN, "  NAK received from SCHAN.\n");
                }
                if ((schanCtrl & SC_CMCx_MSG_SER_CHECK_FAIL) &&
                    soc_feature(unit, soc_feature_ser_parity)){
                    rv = SOC_E_FAIL;
                    soc_cm_debug(DK_SCHAN, "  SER Parity Check Error.\n");
                }
                if (soc_feature(unit, soc_feature_schan_hw_timeout)) {
                    if (schanCtrl & SC_CMCx_MSG_TIMEOUT_TST) {
                        rv = SOC_E_TIMEOUT;
                    }
                }
                soc_pci_write(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc),
                    soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc)) & ~SC_CMCx_MSG_DONE);
            } else 
#endif /* CMICM Support */
            {
                while (((schanCtrl = soc_pci_read(unit, CMIC_SCHAN_CTRL)) &
                        SC_MSG_DONE_TST) == 0) {
                    if (soc_timeout_check(&to)) {
                        rv = SOC_E_TIMEOUT;
                        break;
                    }
                }

                if (rv == SOC_E_NONE) {
                    soc_cm_debug(DK_SCHAN, "  Done in %d polls\n", to.polls);
                }

                if (schanCtrl & SC_MSG_NAK_TST) {
                    rv = SOC_E_FAIL;
                    soc_cm_debug(DK_SCHAN, "  NAK received from SCHAN.\n");
#ifdef BCM_TRIUMPH2_SUPPORT
                    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit)) {
                        schan_msg_t resp_msg;
                        /* This is still the input opcode */
                        resp_msg.dwords[1] =
                            soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, 1));
                        switch (msg->header.opcode) {
                        case TABLE_INSERT_CMD_MSG:
                        case TABLE_DELETE_CMD_MSG:
                        case TABLE_LOOKUP_CMD_MSG:
                            if (SCHAN_GEN_RESP_TYPE_ERROR !=
                                resp_msg.genresp.response.type) {
                                /* Not a parity error */
                                break;
                            }
                            /* Else fallthru */
                        case READ_MEMORY_CMD_MSG:
                            /* All use the same address location */
                            sal_dpc(soc_triumph2_mem_nack, INT_TO_PTR(unit),
                                    INT_TO_PTR(msg->gencmd.address), 0, 0, 0);
                            break;
                        default:
                            break;
                        }
                    }
#endif /* BCM_TRIUMPH2_SUPPORT */
                }
#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
                if ((schanCtrl & SC_MSG_SER_CHECK_FAIL_TST) &&
                    soc_feature(unit, soc_feature_ser_parity)) {
                    rv = SOC_E_FAIL;
                    sal_dpc(soc_ser_fail, INT_TO_PTR(unit),
                            INT_TO_PTR(msg->gencmd.address), 0, 0, 0);
                }
#endif /* BCM_TRIUMPH2_SUPPORT, BCM_TRIDENT_SUPPORT */
                if (soc_feature(unit, soc_feature_schan_hw_timeout)) {
                    if (schanCtrl & SC_MSG_TIMEOUT_TST) {
                        rv = SOC_E_TIMEOUT;
                    }
                }
                soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);
            }
        }

        if (rv == SOC_E_TIMEOUT) {
            soc_cm_debug(DK_VERBOSE,
                         "soc_schan_op: operation attempt timed out\n");
            SOC_CONTROL(unit)->stat.err_sc_tmo++;
            _soc_schan_reset(unit);
            break;
        }

        /* Read in data from S-Channel buffer space, if any */

        for (i = 0; i < dwc_read; i++) {
#ifdef BCM_CMICM_SUPPORT
            if (soc_feature(unit, soc_feature_cmicm)) {
                msg->dwords[i] = soc_pci_read(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i));
            } else 
#endif /* CMICM Support */
            {
                msg->dwords[i] = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
            }
        }

        if (soc_cm_debug_check(DK_SCHAN)) {
            soc_schan_dump(unit, msg, dwc_read);
        }

        SOC_CONTROL(unit)->stat.schan_op++;

    } while (0);

    SCHAN_UNLOCK(unit);

    if (rv == SOC_E_TIMEOUT) {
        if (soc_cm_debug_check(DK_ERR)) {
            soc_cm_print("soc_schan_op: operation timed out\n");
            soc_schan_dump(unit, msg, dwc_write);
        }
    }

    return rv;
}

#ifdef BCM_CMICM_SUPPORT
int fschan_wait_idle(int unit) {
    int cmc = SOC_PCI_CMC(unit);
    soc_timeout_t to;
    soc_timeout_init(&to, SOC_CONTROL(unit)->schanTimeout, 100);
    do {
        if (0==(soc_pci_read(unit, CMIC_CMCx_FSCHAN_STATUS_OFFSET(cmc)) & FSCHAN_BUSY)) {
            return SOC_E_NONE;
        }
    } while (!soc_timeout_check(&to));
    return SOC_E_TIMEOUT;
}
#endif
