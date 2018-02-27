/*
 * $Id: i2c.h 1.27.24.1 Broadcom SDK $
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
 * Constants and defines for the Broadcom StrataSwitch (BCM56xx) I2C
 * Bus Driver interface controller operating in master mode.
 *
 * See also: Broadcom StrataSwitch (TM) Register Reference Guide
 */

#ifndef _SOC_I2C_H
#define _SOC_I2C_H

#ifdef	INCLUDE_I2C

#include <sal/compiler.h>
#include <sal/core/sync.h>

#define	I2CBUS_VOID(_u)	(SOC_CONTROL(_u)->i2c_bus)
#define	I2CBUS(_u)	((soc_i2c_bus_t *)I2CBUS_VOID(_u))

#define I2C_LOCK(unit) \
	sal_mutex_take(I2CBUS(unit)->i2cMutex, sal_mutex_FOREVER)
#define I2C_UNLOCK(unit) \
	sal_mutex_give(I2CBUS(unit)->i2cMutex)

#define _i2c_abs(x)			((x) < 0 ? -(x) : (x))

/* Definitions for CMIC_I2C_CTRL@0x00000128 */
#define CI2CC_INT_EN                    0x00000080 /* Interrupt enable */
#define CI2CC_BUS_EN                    0x00000040 /* Bus enable */
#define CI2CC_MM_START                  0x00000020 /* Send start */
#define CI2CC_MM_STOP                   0x00000010 /* Send stop */
#define CI2CC_INT_FLAG                  0x00000008 /* Interrupt pending */
#define CI2CC_AACK                      0x00000004 /* Send ACK pulse */

/* All I2C registers are 8bits wide */
#define CMIC_I2C_REG_MASK               0x000000ff
#define CMIC_I2C_SPEED_DEFAULT          400  /* 400Khz */
#define CMIC_I2C_SPEED_SLOW_IO          110  /* 101Khz */

/* Timeout values, for semaphore give/take operations */
#define I2C_TIMEOUT		(100 * MILLISECOND_USEC)
#define I2C_TIMEOUT_QT		(1   * SECOND_USEC)
#define I2C_TIMEOUT_PLI	        (20  * SECOND_USEC)
/* Base I2C saddr address base for the SOC device(s). */
/* Uses 7-bit "datasheet" convention. */
#define SOC_I2C_SLAVE_BASE              0x10

/* Devices allowed per switch chip - arbitrary value selected here */
#define MAX_I2C_DEVICES     48

/* I2C Slave receive (read) bus address byte(s) */
#define SOC_I2C_RX_ADDR(addr) (((addr)<<1) | (I2C_READ_MASK)) 
/* I2C Slave transmit (write) bus address byte(s) */
#define SOC_I2C_TX_ADDR(addr) (((addr)<<1) & ~(I2C_READ_MASK)) 


/*
 * I2C Bus Interface Controller Definitions
 *
 * Status codes: The 31 possible status codes of the CMIC_I2C_STAT
 * register bits: 0-7, enumerated for namespace reasons. Note that
 * all of the status codes are multiples of 8.
 */

typedef enum soc_i2c_status_e {
    SOC_I2C_BERR = 0x00,
    SOC_I2C_START_TX = 0x08,
    SOC_I2C_REP_START_TX = 0x10,
    SOC_I2C_ADDR_WR_BIT_TX_ACK_RX = 0x18,
    SOC_I2C_ADDR_WR_BIT_TX_NO_ACK_RX = 0x20,
    SOC_I2C_DATA_BYTE_TX_ACK_RX = 0x28,
    SOC_I2C_DATA_BYTE_TX_NO_ACK_RX = 0x30,
    SOC_I2C_ARB_LOST = 0x38,
    SOC_I2C_ADDR_RD_BIT_TX_ACK_RX = 0x40,
    SOC_I2C_ADDR_RD_BIT_TX_NO_ACK_RX = 0x48,
    SOC_I2C_DATA_BYTE_RX_ACK_TX = 0x50,
    SOC_I2C_DATA_BYTE_RX_NO_ACK_TX = 0x58,
    SOC_I2C_SADDR_RX_WR_BIT_RX_ACK_TX = 0x60,
    SOC_I2C_ARB_LOST_SADDR_RX_WR_BIT_RX_ACK_TX = 0x68,
    SOC_I2C_GC_ADDR_RX_ACK_TX = 0x70,
    SOC_I2C_ARB_LOST_GC_ADDR_RX_ACK_TX = 0x78,
    SOC_I2C_DATA_BYTE_RX_AFTER_SADDR_RX_ACK_TX = 0x80,
    SOC_I2C_DATA_BYTE_RX_AFTER_SADDR_RX_NO_ACK_TX = 0x88,
    SOC_I2C_DATA_BYTE_RX_AFTER_GC_ADDR_RX_ACK_TX = 0x90,
    SOC_I2C_DATA_BYTE_RX_AFTER_GC_ADDR_RX_NO_ACK_TX = 0x98,
    SOC_I2C_STOP_OR_REP_START_COND_RX_IN_SLAVE_MODE = 0xA0,
    SOC_I2C_SADDR_RX_RD_BIT_RX_ACK_TX = 0xA8,
    SOC_I2C_ARB_LOST_IN_ADDR_PHASE_SADDR_RX_RD_BIT_RX_ACK_TX = 0xB0,
    SOC_I2C_SM_DATA_BYTE_TX_ACK_RX = 0xB8,
    SOC_I2C_SM_DATA_BYTE_TX_NO_ACK_RX = 0xC0,
    SOC_I2C_SM_LAST_BYTE_TX_ACK_RX = 0xC8,
    SOC_I2C_2ND_ADDR_BYTE_TX_WR_BIT_TX_ACK_RX = 0xD0,
    SOC_I2C_2ND_ADDR_BYTE_TX_WR_BIT_TX_NO_ACK_RX = 0xD8,
    SOC_I2C_2ND_ADDR_BYTE_TX_RD_BIT_TX_ACK_RX = 0xE0,
    SOC_I2C_2ND_ADDR_BYTE_TX_RD_BIT_TX_NO_ACK_RX = 0xE8,
    SOC_I2C_UNDEFINED = 0xF0, /* Not defined, for symmetry only */
    SOC_I2C_NO_STATUS = 0xF8,
    SOC_I2C_NUM_STATUS_CODES /* Always last please */
} soc_i2c_status_t;


/*
 * CPU/Master Initiated actions.
 */

typedef enum soc_i2c_op_e {
    SOC_I2C_IDLE,
    SOC_I2C_START,
    SOC_I2C_REP_START,
    SOC_I2C_TX,
    SOC_I2C_RX,
    SOC_I2C_STOP,
    SOC_I2C_PROBE
} soc_i2c_op_t;

/* Software Interfaces */

typedef uint8 i2c_saddr_t;     /* Device address in datasheet-speak */
typedef uint8 i2c_bus_addr_t;  /* Address byte on the I2C SDA line  */

/* Driver i/f */
extern soc_i2c_status_t soc_i2c_stat(int unit);
extern int soc_i2c_device_present(int unit, i2c_saddr_t saddr);
extern int soc_i2c_probe(int unit);
extern int soc_i2c_attach(int unit, uint32 flags, int speed);
extern void soc_i2c_reset(int unit);
extern int soc_i2c_detach(int unit);
extern int soc_i2c_ack_poll(int unit, i2c_bus_addr_t bus_addr, int maxpolls);
extern int soc_i2c_start(int unit, i2c_bus_addr_t bus_addr);
extern int soc_i2c_rep_start(int unit, i2c_bus_addr_t bus_addr);
extern int soc_i2c_write_one_byte(int unit, uint8 data);
extern int soc_i2c_read_one_byte(int unit, uint8* data, int ack);
extern int soc_i2c_read_bytes(int unit, uint8* data, int* len, int ack_last_byte);
extern int soc_i2c_read_short(int unit, uint16* val, int ack);
extern int soc_i2c_stop(int unit);
extern void soc_i2c_intr(int unit);
extern i2c_saddr_t soc_i2c_addr(int unit, int i2c_devid);
extern void soc_i2c_next_bus_phase(int unit, int tx_ack);
extern int soc_i2c_wait(int unit);
extern void soc_i2c_show(int unit);
extern int soc_i2c_device_count(int unit);
extern int soc_i2c_devtype(int unit, int devid);
extern int soc_i2c_is_attached(int unit);
extern void soc_i2c_show_log(int unit, int reverse);
extern void soc_i2c_clear_log(int unit);
extern void soc_i2c_show_speeds(int unit);
extern char *soc_i2c_saddr_to_string(int unit, i2c_saddr_t saddr);

/*
 * SMB commands
 */
extern int soc_i2c_read_byte(int unit, i2c_saddr_t saddr, uint8* data);
extern int soc_i2c_write_byte(int unit, i2c_saddr_t saddr, uint8 data);
extern int soc_i2c_read_word(int unit, i2c_saddr_t saddr, uint16* data);
extern int soc_i2c_write_word(int unit, i2c_saddr_t saddr, uint16 data);
extern int soc_i2c_read_byte_data(int unit, i2c_saddr_t saddr, 
			   uint8 com, uint8* val);
extern int soc_i2c_write_byte_data(int unit, i2c_saddr_t saddr, 
			    uint8 com, uint8 val);
extern int soc_i2c_read_word_data(int unit, i2c_saddr_t saddr, 
			   uint8 com, uint16* val);
extern int soc_i2c_write_word_data(int unit, i2c_saddr_t saddr, 
			    uint8 com, uint16 val);
extern int soc_i2c_block_read(int unit, i2c_saddr_t saddr, 
		       uint8 com, uint8* n, uint8* d);
extern int soc_i2c_block_write(int unit, i2c_saddr_t saddr, 
			uint8 com, uint8 n, uint8* d);
extern char *soc_i2c_status_message(soc_i2c_status_t code);


/* I2C Device IO defines */
#define  I2C_XADDR_MASK		0xf8
#define  I2C_XADDR		0xf0
#define  I2C_READ_MASK		0x01

/* I2C Device Attribute flags */
#define  I2C_DEV_NONE		0x0
#define  I2C_DEV_OK		0x1
#define  I2C_DEV_BUSY		0x2
#define  I2C_DEV_TESTED		0x4
#define  I2C_DEV_DRIVER		0X8
#define  I2C_REG_STATIC		0x10
#define  I2C_REG_DYNAMIC  	0x11

/*
 * I2C Driver i/f routines.
 */
typedef int (*i2c_read_func_t)(int unit, int devno,
			       uint16 addr, uint8* data, uint32* len);
typedef int (*i2c_write_func_t)(int unit, int devno,
				uint16 addr, uint8* data, uint32 len);
typedef int (*i2c_ioc_func_t)(int unit, int devno, int opcode,
			      void* data, int len);
typedef int (*i2c_dev_init_func_t)(int unit, int devno,
				   void* data, int len);
/*
 * I2C Driver structure: definitions for managed device operations
 */
typedef struct i2c_driver_s {
    uint8 flags;                  /* Device flags */
    int devno;                    /* Index into device descriptor table */
    uint32 id;                    /* Device Serial No, GUID, or identifier */
    i2c_read_func_t     read;     /* Read routine */
    i2c_write_func_t    write;    /* Write routine */
    i2c_ioc_func_t      ioctl;    /* io control routine */
    i2c_dev_init_func_t load;     /* called once at startup */
} i2c_driver_t;

/*
 * I2C Device Descriptor: One for every known device on the bus
 */
typedef struct i2c_dev_s {
    char *devname;                 /* Device name, eg. "eeprom0" */
    i2c_saddr_t saddr;             /* Slave address */
    i2c_driver_t *driver;          /* Driver routines */
    void* testdata;                /* Test data */
    int testlen;                   /* Size of test data */
    uint32 tbyte;                  /* Bytes transmitted */
    uint32 rbyte;                  /* Bytes received */
    char *desc;                    /* Description */
} i2c_device_t;

/*
 * I2C BUS configuration parameters, one per Switch on a Chip (SOC)
 * device
 */
typedef struct soc_i2c_bus_s {
    uint32 flags;               /* Bitmask of state : see below */
    uint32 frequency;           /* Frequency */
    uint8 m_val;                /* M for frequency setting */
    uint8 n_val;                /* N for frequency setting */
    uint32 master_addr;         /* SOC's slave address (reset default 0x44) */
    soc_i2c_op_t  opcode;       /* Current operation in progress */
    uint32  data;               /* Data associate with operation */
    uint32  pio_retries;        /* Max number of times to sleep for IFLG=0 */
    uint32  iflg_polls;         /* Polls of IFLG on last operation (PIO)*/
    soc_i2c_status_t stat;      /* STAT: current state of bus */
    sal_mutex_t i2cMutex;       /* I2C state mutual exclusion */
    sal_sem_t i2cIntr;          /* I2C interrupt notification */
    int i2cTimeout;             /* Operation timeout in microseconds */
    uint32 rxBytes;             /* Bytes in */
    uint32 txBytes;             /* Bytes out */
    i2c_device_t *devs[MAX_I2C_DEVICES];	/* devices on this bus */
} soc_i2c_bus_t;

/* Bus Controller flag bit values and their meanings */
#define SOC_I2C_DETACHED    0x00 /* No status */
#define SOC_I2C_MODE_PIO    0x01 /* PIO mode */
#define SOC_I2C_MODE_INTR   0x02 /* Interrupt mode */
#define SOC_I2C_ATTACHED    0x04 /* Driver attached */

/*
 * Software device object accessors.
 */
extern int soc_i2c_register(int unit, char *devname,
				 i2c_device_t *device);
extern i2c_device_t *soc_i2c_device(int unit, int devid);
extern const char *soc_i2c_devname(int unit, int devid);
extern int soc_i2c_devdesc_set(int unit, int devid, char *desc);
extern int soc_i2c_devdesc_get(int unit, int devid, char **desc);
extern i2c_device_t *soc_i2c_get_devices(int unit, int *count);
extern int soc_i2c_devopen(int unit, char *devname, uint32 flags,
				int speed);


/* I2C devices, descriptions, slave addresses, and I/O control calls
 * The only interfaces made available here are for the BCM I2C API
 * See also: bcm/bcmi2c.h
 */

/* Generic opcodes for device low level config routine */
#define I2C_LLC_DETECT 0
#define I2C_LLC_READ   1
#define I2C_LLC_RESET  2

/* LM75 Thermal Sensor chips */
#define LM75_DEVICE_TYPE  0x00000001 /* Device ID */
#define I2C_LM75_0        "temp0"
#define I2C_LM75_1        "temp1"
#define I2C_LM75_2        "temp2"
#define I2C_LM75_3        "temp3"
#define I2C_LM75_4        "temp4"
#define I2C_LM75_5        "temp5"
#define I2C_LM75_6        "temp6"
#define I2C_LM75_7        "temp7"
#define I2C_LM75_SADDR0   0x48
#define I2C_LM75_SADDR1   0x49
#define I2C_LM75_SADDR2   0x4A
#define I2C_LM75_SADDR3   0x4B
#define I2C_LM75_SADDR4   0x4C
#define I2C_LM75_SADDR5   0x4D
#define I2C_LM75_SADDR6   0x4E
#define I2C_LM75_SADDR7   0x4F

/* 24C64 EEPROM Chips */
#define LC24C64_DEVICE_TYPE 0x00000002 /* Device ID */
#define I2C_NVRAM_0       "nvram0"
#define I2C_NVRAM_1       "nvram1"
#define I2C_NVRAM_SADDR0  0x50
#define I2C_NVRAM_SADDR1  0x54

/* XFP Modules */
#define XFP_DEVICE_TYPE 0x00000002 /* Device ID */
#define I2C_XFP_0       "nvram0"
#define I2C_XFP_SADDR0  0x50

/* PCI-E config space register Debug access */
#define PCIE_DEVICE_TYPE 0x00000002 /* Device ID */
#define I2C_PCIE_0       "pcie0"
#define I2C_PCIE_SADDR0  0x44

/* MAX127 ADC chips */
#define MAX127_DEVICE_TYPE 0x00000003 /* Device ID */
#define I2C_ADC_0       "adc0"
#define I2C_ADC_1       "adc1"
#define I2C_ADC_2       "adc2"
#define I2C_ADC_3       "adc3"
#define I2C_ADC_SADDR0   0x28
#define I2C_ADC_SADDR1   0x29
#define I2C_ADC_SADDR2   0x2A
#define I2C_ADC_SADDR3   0x2B

#define I2C_ADC_CH0       0x0 /* MAX127 ADC Channel select 0 */
#define I2C_ADC_CH1       0x1 /* MAX127 ADC Channel select 1 */
#define I2C_ADC_CH2       0x2 /* MAX127 ADC Channel select 2 */
#define I2C_ADC_CH3       0x3 /* MAX127 ADC Channel select 3 */
#define I2C_ADC_CH4       0x4 /* MAX127 ADC Channel select 4 */
#define I2C_ADC_CH5       0x5 /* MAX127 ADC Channel select 5 */
#define I2C_ADC_CH6       0x6 /* MAX127 ADC Channel select 6 */
#define I2C_ADC_CH7       0x7 /* MAX127 ADC Channel select 7 */

/* MAX127 Type
 * Polarity, min/max, value, reset range and current
 * range structure. Used by ioctl() interface to query ADC
 * after last operation.
 */
typedef struct max127_s {
#ifdef	COMPILER_HAS_DOUBLE
    double val;
    double min;
    double max;
    double delta;
    uint8 cr;
    uint8 cv;
    uint8 r;
#else
    int val;
    int min;
    int max;
    int delta;
    int cr;
    int cv;
#endif
} max127_t;



/* Cypress W311/W229 clock chips */
#define W229B_DEVICE_TYPE 0x00000004 /* W229/W311 Device Driver ID */
#define I2C_PLL_0      "pll0"
#define I2C_PLL_SADDR0  0x69
#define I2C_PLL_SETW311 0x01 /* Enable W311 mode */
#define I2C_PLL_W311_SETSPEED  0x00 /* Set the speed of a clock */
#define I2C_PLL_W311_SPREAD    0x02 /* Set the spread spectrum mode */
#define I2C_PLL_W311_GETSPREAD 0x04 /* Get On/Off state */
#define I2C_PLL_W311_GETSPEED  0x08 /* Get Speed */

/* Spread spectrum settings */
#define I2C_PLL_SPREAD_NONE    0x00 /* No spread spectrum */
#define I2C_PLL_SPREAD_NEG5    0x01 /*    -0.50% */
#define I2C_PLL_SPREAD_5_5     0x02 /* +/- 0.50% */
#define I2C_PLL_SPREAD_2_5     0x03 /* +/- 0.25% */
#define I2C_PLL_SPREAD_3_8     0x04 /* +/- 0.38% */


/* PCF8574 Parallel port chips */
#define PCF8574_DEVICE_TYPE   0x00000005 /* Device ID */
#define I2C_LPT_0      "lpt0"
#define I2C_LPT_1      "lpt1"
#define I2C_LPT_2      "lpt2"
#define I2C_LPT_3      "lpt3"
#define I2C_LPT_4      "lpt4"
#define I2C_LPT_5      "lpt5"
#define I2C_LPT_6      "lpt6"
#define I2C_LPT_7      "lpt7"
#define I2C_LPT_SADDR0  0x20
#define I2C_LPT_SADDR1  0x27
#define I2C_LPT_SADDR2  0x23
#define I2C_LPT_SADDR3  0x26
#define I2C_LPT_SADDR4  0x21
#define I2C_LPT_SADDR5  0x22
#define I2C_LPT_SADDR6  0x24
#define I2C_LPT_SADDR7  0x25

/* LPT Mux: magic values written to lpt0 port to make new devices appear*/
#define I2C_LPT_P48MUX_CLOCK_A  0x00 /* Core/Turbo clock and VDD_A */
#define I2C_LPT_P48MUX_SDRAM_A  0x01 /* SDRAM clock A */
#define I2C_LPT_P48MUX_CLOCK_B  0x02 /* Core clock B */
#define I2C_LPT_P48MUX_SDRAM_B  0x03 /* SDRAM clock B */
#define I2C_LPT_P48MUX_VDD_A    0x04 /* ADC VDD_A / DAC VDD_A */
#define I2C_LPT_P48MUX_VDD_B    0x08 /* ADC VDD_B / DAC VDD_B */
#define I2C_LPT_P48MUX_VDD_T    0x0c /* ADC TRef /DAC TRef */


/* Linear Tech 1427 DAC chips */
#define LTC1427_DEVICE_TYPE 0x00000006 /* Device ID */
#define I2C_DAC_0      "dac0"
#define I2C_DAC_SADDR0   0x2C
#define I2C_DAC_IOC_SETDAC_MIN      0 /* DAC ops set output to MIN */
#define I2C_DAC_IOC_SETDAC_MAX      1 /* DAC ops set output to MAX */
#define I2C_DAC_IOC_CALIBRATE_MIN   2 /* DAC set min calibration value */
#define I2C_DAC_IOC_CALIBRATE_MAX   3 /* DAC set max calibration value */
#define I2C_DAC_IOC_CALIBRATE_STEP  4 /* DAC set step calibration value */
#define I2C_DAC_IOC_SET_VOUT        5 /* DAC set VOUT operation */
#define I2C_DAC_IOC_SET_CALTAB      6 /* Upload DAC calibration table */
#define I2C_DAC_IOC_SETDAC_MID      7 /* Reset DAC to its H/W midrange */
#define I2C_DAC_CAL_A               0 /* Calibration index:VDD Core A */
#define I2C_DAC_CAL_B               1 /* Calibration index:VDD Core B */
#define I2C_DAC_CAL_P               2 /* Calibration index:VDD PHY */
#define I2C_DAC_CAL_T               3 /* Calibration index:VDD Turbo */

/*
 * DAC Calibration table for various ADC inputs and DAC chips.
 * The ioctl call indexes this table with the len parameter
 * and store the associated DAC Volts/Step value after setting max/min
 * via reads from a MAX127 A/D device.
 */
typedef struct dac_calibrate_s {
    int idx;
    char *name;
#ifdef	COMPILER_HAS_DOUBLE
    double max;
    double min;
    double step;
#else
    int    max;
    int    min;
    int    step;
#endif
    short  dac_last_val;
    short  dac_max_hwval;
    short  dac_min_hwval;
    short  use_max;
} dac_calibrate_t;

/*
 * The LTC-1427-50 is a 10bit DAC.
 */
#define LTC1427_VALID_BITS    0x03ff

/* On HUMV/Bradley/GW slow part, max and min should be set to  960 and 64
 * to prevent the system crashes during voltage calibration.
 */
#define LTC1427_MAX_DACVAL       1023 
#define LTC1427_MIN_DACVAL       0    
#define LTC1427_MID_DACVAL       512



/*
 * LM75 Device Driver Temperature probe and display
 * See: drv/i2c/lm75.c
 */
extern void soc_i2c_lm75_temperature_show(int unit);
extern void soc_i2c_lm75_monitor(int unit, int enable, int nsecs);



/* Matrix Orbital LCD Controllers
 * See also: http://www.matrix-orbital.com
 */
#define LCD_DEVICE_TYPE 0x00000007 /* Device ID */
#define I2C_LCD_0      "lcd0"
#define I2C_LCD_SADDR0  0x2E
#define I2C_LCD_CLS        1
#define I2C_LCD_CONTRAST   2
#define I2C_LCD_CONFIG     3

/* Cypress 2239x clock chips */
#define CY2239X_DEVICE_TYPE 0x00000008
#define I2C_XPLL_0      "clk0"
#define I2C_XPLL_SADDR0     0x6A

#define I2C_XPLL_1      "clk0"
#define I2C_XPLL_SADDR1     0x69

#define I2C_XPLL_PLL1       0x01
#define I2C_XPLL_PCI        I2C_XPLL_PLL1
#define I2C_XPLL_SET_PCI    I2C_XPLL_PLL1
#define I2C_XPLL_SET_PLL1   I2C_XPLL_SET_PCI

#define I2C_XPLL_PLL2       0x02
#define I2C_XPLL_SDRAM      I2C_XPLL_PLL2
#define I2C_XPLL_SET_SDRAM  I2C_XPLL_PLL2
#define I2C_XPLL_SET_PLL2   I2C_XPLL_SET_SDRAM

#define I2C_XPLL_PLL3       0x03
#define I2C_XPLL_CORE       I2C_XPLL_PLL3
#define I2C_XPLL_SET_CORE   I2C_XPLL_PLL3
#define I2C_XPLL_SET_PLL3   I2C_XPLL_SET_CORE

#define I2C_XPLL_GET_OP(x)  ((x) + 10)

#define I2C_XPLL_GET_PCI    I2C_XPLL_GET_OP(I2C_XPLL_PLL1)
#define I2C_XPLL_GET_PLL1   I2C_XPLL_GET_PCI

#define I2C_XPLL_GET_SDRAM  I2C_XPLL_GET_OP(I2C_XPLL_PLL2)
#define I2C_XPLL_GET_PLL2   I2C_XPLL_GET_SDRAM

#define I2C_XPLL_GET_CORE   I2C_XPLL_GET_OP(I2C_XPLL_PLL3)
#define I2C_XPLL_GET_PLL3   I2C_XPLL_GET_CORE

/* Cypress 22150 clock chip */
#define CY22150_DEVICE_TYPE 0x00000009
#define I2C_XPLL1_0      "clk0"
#define I2C_XPLL1_SADDR0  0x6B


/* User IO */
#define I2C_XPLL_GET_REG    0x40
#define I2C_XPLL_SET_REG    0x80


/* PowerDsine 63000 PoE Microcontroller Unit */
#define PD63000_DEVICE_TYPE 0x0000000A /* Device ID */
#define I2C_POE_0      "poe0"
#define I2C_POE_SADDR0  0x38

/* Linear Tech 4258 Powered Ethernet (POE) chips */
#define LTC4258_DEVICE_TYPE 0x0000000B /* Device ID */
#define I2C_POE_1      "poe1"
#define I2C_POE_2      "poe2"
#define I2C_POE_3      "poe3"
#define I2C_POE_4      "poe4"
#define I2C_POE_5      "poe5"
#define I2C_POE_6      "poe6"
#define I2C_POE_SADDR1   0x21
#define I2C_POE_SADDR2   0x22
#define I2C_POE_SADDR3   0x23
#define I2C_POE_SADDR4   0x24
#define I2C_POE_SADDR5   0x25
#define I2C_POE_SADDR6   0x26
#define I2C_POE_IOC_AUTO         1
#define I2C_POE_IOC_SHUTDOWN     2
#define I2C_POE_IOC_ENABLE_PORT  3
#define I2C_POE_IOC_DISABLE_PORT 4
#define I2C_POE_IOC_REG_DUMP     5
#define I2C_POE_IOC_STATUS       6
#define I2C_POE_IOC_CLEAR        7
#define I2C_POE_IOC_RESCAN       8


/* Maxim 5478 Digital Potentiometer chips */
/*
 * ioctl request structure
 */
typedef struct max5478_s {
    uint8       wiper;
    uint8       value;
} max5478_t;

#define MAX5478_DEVICE_TYPE  0x0000000C /* Device ID */
#define I2C_POT_0            "pot0"
#define I2C_POT_SADDR0       0x2D
#define I2C_POT_IOC_SET      1 /* POT ioctl request */

#define I2C_POT_MSG_WIPER_A  0 /* Msg command */
#define I2C_POT_MSG_WIPER_B  1 /* Msg command */

/* Broadcom BCM59101 POE */
#define BCM59101_DEVICE_TYPE  0x0000000D /* Device ID */
#define I2C_POE_7        "poe7"
#define I2C_POE_8        "poe8"
#define I2C_POE_9        "poe9"
#define I2C_POE_10       "poe10"
#define I2C_POE_SADDR7   0x20
#define I2C_POE_SADDR8   0x28
#define I2C_POE_SADDR9   0x50
#define I2C_POE_SADDR10  0x58

/* PCA9548 8-channel I2C Switch */
#ifdef SHADOW_SVK
#define I2C_MUX_0         "mux2"
#else
#define I2C_MUX_0         "mux0"
#endif
#define I2C_MUX_SADDR0    0x72

#define I2C_MUX_3         "mux3"
#define I2C_MUX_SADDR3    0x73

#define I2C_MUX_4         "mux4"
#define I2C_MUX_SADDR4    0x74

#define I2C_MUX_5         "mux5"
#define I2C_MUX_SADDR5    0x75

#define I2C_MUX_6         "mux6"
#define I2C_MUX_SADDR6    0x76

#define I2C_MUX_7         "mux7"
#define I2C_MUX_SADDR7    0x77

#define PCA9548_DEVICE_TYPE  0x0000000E /* Device ID */

/* ADP4000 an integrated power control IC */
#define I2C_PWCTRL_0                    "pwctrl0"
#define I2C_PWCTRL_SADDR0               0x65
#define I2C_ADP4000_CMD_OPERATION       0x1
#define I2C_ADP4000_CMD_ON_OFF_CONFIG   0x2
#define I2C_ADP4000_CMD_CLEAR_FAULTS    0x3
#define I2C_ADP4000_CMD_WRITE_PROTECT   0x10
#define I2C_ADP4000_CMD_CAPACITY        0x19 
#define I2C_ADP4000_CMD_VOUT_MODE       0x20
#define I2C_ADP4000_CMD_VOUT_CMD        0x21
#define I2C_ADP4000_CMD_STATUS_BYTE     0x78
#define I2C_ADP4000_CMD_STATUS_WORD     0x79
#define I2C_ADP4000_CMD_RESET           0xD0
#define I2C_ADP4000_CMD_CONFIG_1A       0xD2
#define I2C_ADP4000_CMD_CONFIG_1B       0xD3

#define ADP4000_DEVICE_TYPE  0x0000000F /* Device ID */
#define ADP4000_MAX_DACVAL       0x72 
#define ADP4000_MIN_DACVAL       0x52    
#define ADP4000_MID_DACVAL       0x62

#ifdef SHADOW_SVK
/* PCA9548 8-channel i2c switch */
#define PCA9548_DEVICE_TYPE     0x0000000E  /* Device ID */
#define I2C_SW_0                "mux6"
#define I2C_SW_1                "mux7"
#define I2C_SW_SADDR0           0x76
#define I2C_SW_SADDR1           0x77



/* PCA9505 40-bit i2c bus I/O port */
#define PCA9505_DEVICE_TYPE     0x0000000F  /* Device ID */
#define I2C_IOP_0               "iop0"
#define I2C_IOP_1               "iop1"
#define I2C_IOP_SADDR0          0x24
#define I2C_IOP_SADDR1          0x22

/* Board ID is in IOP1, port 1 */
#define I2C_IOP_1_BRD_ID        1
#endif /* SHADOW_SVK */

#ifdef BCM_CMICM_SUPPORT
#define SMBUS_QUICK_CMD         0
#define SMBUS_SEND_BYTE         1
#define SMBUS_RECEIVE_BYTE      2
#define SMBUS_WRITE_BYTE        3
#define SMBUS_READ_BYTE         4
#define SMBUS_WRITE_WORD        5
#define SMBUS_READ_WORD         6
#define SMBUS_BLOCK_WRITE       7
#define SMBUS_BLOCK_READ        8
#define SMBUS_PROCESS_CALL      9
#define SMBUS_BLOCK_PROCESS_CALL 10

extern int smbus_start_wait(int unit);
extern int smbus_quick_command(int unit, uint8 addr);

#endif /* CMICM Support */

#endif	/* INCLUDE_I2C */

#endif	/* !_SOC_I2C_H */
