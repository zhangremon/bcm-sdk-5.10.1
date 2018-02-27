/*
 * $Id: reg.c,v 1.55.2.2 2011/03/14 11:34:25 vroy Exp $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:        reg.c
 * Purpose:     SBX commands for register access
 * Requires:
 */
 
#include <sal/core/libc.h>
#include <sal/core/sync.h>
#include <sal/appl/pci.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/decode.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/dport.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/phyctrl.h>
#include <soc/l2x.h>
#include <soc/ll.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <bcm/stat.h>
#include <ibde.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/port.h>

cmd_result_t
sbx_reg_info_print(int unit, soc_sbx_reg_info_t *reg_info, uint32 flags);

cmd_result_t  
sbx_pcim_print_regs(int unit, int pci_regs);

#if defined(BCM_SIRIUS_SUPPORT)
cmd_result_t
cmd_sbx_cmic_reg_get(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_set(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_mod(int unit, args_t * a);

cmd_result_t
cmd_sbx_cmic_reg_cmp(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_list(int unit, args_t *a);

cmd_result_t
cmd_sirius_print_info(int unit, char *str, int clrok, args_t *a);

cmd_result_t
cmd_sbx_cmic_reg_edit(int unit, args_t *a);

cmd_result_t
cmd_sbx_cmic_dump(int unit, args_t *a);
#endif /* BCM_SIRIUS_SUPPORT */


/*
 * Register Types - for getreg and dump commands
 */

typedef enum sbx_regtype_t {
    sbx_pci_cfg_reg,    /* PCI configuration space register */
    sbx_cpureg,         /* AKA PCI memory */
    sbx_sw,             /* internal software state */
} sbx_regtype_t;

typedef struct sbx_regtype_entry_t {
    char *name;         /* Name of register for setreg/getreg commands */
    sbx_regtype_t type; /* Type of register (e.g. sbx_cpureg) */
    char *help;         /* Description of register for help command */
} sbx_regtype_entry_t;

STATIC sbx_regtype_entry_t sbx_regtypes[] = {
 { "PCIC",      sbx_pci_cfg_reg, "PCI Configuration space" },
 { "PCIM",      sbx_cpureg,      "PCI Memory space" },
 { "SW",        sbx_sw,          "Software state" },
};

/* Names of register types, indexed by sbx_regtype_t */
char *sbx_regtypenames[] = {
    "pci_cfg",        /* sbx_pci_cfg_reg,  */
    "cpu",            /* sbx_cpureg,       */
    "sw",             /* internal software state */
};

#define sbx_regtypes_count      COUNTOF(sbx_regtypes)

STATIC sbx_regtype_entry_t *sbx_regtype_lookup_name(char* str)
{
    int i;

    for (i = 0; i < sbx_regtypes_count; i++) {
        if (!sal_strcasecmp(str, sbx_regtypes[i].name)) {
            return &sbx_regtypes[i];
        }
    }
    
    return 0;
}

#ifdef   BCM_ICS
/* Do nothing on ICS */
static void
_pci_print_config(int dev)
{
}
#else
STATIC void
_pci_print_config(int dev)
{
    uint32              data;

    data = bde->pci_conf_read(dev, PCI_CONF_VENDOR_ID);
    printk("%04x: %08x  DeviceID=%04x  VendorID=%04x\n",
                 PCI_CONF_VENDOR_ID, data,
                 (data & 0xffff0000) >> 16,
                 (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_COMMAND);
    printk("%04x: %08x  Status=%04x  Command=%04x\n",
                 PCI_CONF_COMMAND, data,
                 (data & 0xffff0000) >> 16,
                 (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_REVISION_ID);
    printk("%04x: %08x  ClassCode=%06x  RevisionID=%02x\n",
                 PCI_CONF_REVISION_ID, data,
                 (data & 0xffffff00) >> 8,
                 (data & 0x000000ff) >> 0);

    data = bde->pci_conf_read(dev, PCI_CONF_CACHE_LINE_SIZE);
    printk("%04x: %08x  BIST=%02x  HeaderType=%02x  "
                 "LatencyTimer=%02x  CacheLineSize=%02x\n",
                 PCI_CONF_CACHE_LINE_SIZE, data,
                 (data & 0xff000000) >> 24,
                 (data & 0x00ff0000) >> 16,
                 (data & 0x0000ff00) >>  8,
                 (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR0);
    printk("%04x: %08x  BaseAddress0=%08x\n",
                 PCI_CONF_BAR0, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR1);
    printk("%04x: %08x  BaseAddress1=%08x\n",
                 PCI_CONF_BAR1, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR2);
    printk("%04x: %08x  BaseAddress2=%08x\n",
                 PCI_CONF_BAR2, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR3);
    printk("%04x: %08x  BaseAddress3=%08x\n",
                 PCI_CONF_BAR3, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR4);
    printk("%04x: %08x  BaseAddress4=%08x\n",
                 PCI_CONF_BAR4, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_BAR5);
    printk("%04x: %08x  BaseAddress5=%08x\n",
                 PCI_CONF_BAR5, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_CB_CIS_PTR);
    printk("%04x: %08x  CardbusCISPointer=%08x\n",
                 PCI_CONF_CB_CIS_PTR, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_SUBSYS_VENDOR_ID);
    printk("%04x: %08x  SubsystemID=%02x  SubsystemVendorID=%02x\n",
                 PCI_CONF_SUBSYS_VENDOR_ID, data,
                 (data & 0xffff0000) >> 16,
                 (data & 0x0000ffff) >>  0);

    data = bde->pci_conf_read(dev, PCI_CONF_EXP_ROM);
    printk("%04x: %08x  ExpansionROMBaseAddress=%08x\n",
                 PCI_CONF_EXP_ROM, data, data);

    data = bde->pci_conf_read(dev, 0x34);
    printk("%04x: %08x  Reserved=%06x  CapabilitiesPointer=%02x\n",
                 0x34, data,
                 (data & 0xffffff00) >> 8,
                 (data & 0x000000ff) >> 0);

    data = bde->pci_conf_read(dev, 0x38);
    printk("%04x: %08x  Reserved=%08x\n",
                 0x38, data, data);

    data = bde->pci_conf_read(dev, PCI_CONF_INTERRUPT_LINE);
    printk("%04x: %08x  Max_Lat=%02x  Min_Gnt=%02x  "
                 "InterruptLine=%02x  InterruptPin=%02x\n",
                 PCI_CONF_INTERRUPT_LINE, data,
                 (data & 0xff000000) >> 24,
                 (data & 0x00ff0000) >> 16,
                 (data & 0x0000ff00) >>  8,
                 (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x40);
    printk("%04x: %08x  Reserved=%02x  "
                 "RetryTimeoutValue=%02x  TRDYTimeoutValue=%02x\n",
                 0x40, data,
                 (data & 0xffff0000) >> 16,
                 (data & 0x0000ff00) >>  8,
                 (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x44);
    printk("%04x: %08x  PLLConf=%01x\n",
                 0x44, data,
                 (data & 0x000000ff) >>  0);

    data = bde->pci_conf_read(dev, 0x48);
    printk("%04x: %08x  -\n",
                 0x48, data);

#ifdef VXWORKS
    /* FIXME: Allow BDE to call pci_find_device() ...
     */
    {
#ifdef IDTRP334
        extern void sysBusErrDisable(void);
        extern void sysBusErrEnable(void);
#endif

        /* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID    0x3388
#define HINT_HB4_DEVICE_ID    0x0022

        int BusNo, DevNo, FuncNo;
        unsigned short tmp;

#ifdef IDTRP334
        sysBusErrDisable();
#endif

        /*
         * HINTCORP HB4 PCI-PCI Bridge
         */
        if (pciFindDevice(HINT_HB4_VENDOR_ID,
                          HINT_HB4_DEVICE_ID,
                          0,
                          &BusNo, &DevNo, &FuncNo) != ERROR) {

            printk("-------------------------------------\n");
            printk("HB4 PCI-PCI Bridge Status Registers  \n");
            printk("-------------------------------------\n");

            /* Dump the status registers */
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x06, &tmp);
            printk("Primary Status (%xh):   0x%x\n", 0x06, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x1e, &tmp);
            printk("Secondary Status (%xh): 0x%x\n", 0x1e, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x3e, &tmp);
            printk("Bridge Control (%xh):   0x%x\n", 0x3e, tmp);
            pciConfigInWord(BusNo,DevNo,FuncNo, 0x6a, &tmp);
            printk("P_SERR Status (%xh):    0x%x\n", 0x6a, tmp);
        }

#ifdef IDTRP334
        sysBusErrEnable();
#endif
    }
#endif
}
#endif /* ! ICS */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP

#define CMD_SW_DUMP_HELP_T(_bcm_module_) \
"  " #_bcm_module_ " -- dump internal " #_bcm_module_ " module state"

#ifdef BCM_FE2000_SUPPORT

#define FE2000_SW_DUMP_CMD_T(_bcm_module_)          \
static cmd_result_t                                                           \
_cmd_fe2000_sw_dump_##_bcm_module_(int unit, args_t *args)                    \
{                                                                             \
    extern void _bcm_fe2000_##_bcm_module_##_sw_dump(int unit);               \
    if (SOC_IS_SBX_FE2000(unit)) {                                            \
        _bcm_fe2000_##_bcm_module_##_sw_dump(unit);                           \
    } else {                                                                  \
        printk(#_bcm_module_ "module sw_dump not supported on this unit\n");  \
        return CMD_FAIL;                                                      \
    }                                                                         \
    return CMD_OK;                                                            \
}

FE2000_SW_DUMP_CMD_T(l3)
FE2000_SW_DUMP_CMD_T(mirror)
FE2000_SW_DUMP_CMD_T(oam)
FE2000_SW_DUMP_CMD_T(stg)
FE2000_SW_DUMP_CMD_T(soc_pm)

#define FE2000_SW_DUMP_OPTIONS "l3|mirror|oam|stg|soc_pm(for FE)\n\t"
#else  /* BCM_FE2000_SUPPORT */
#define FE2000_SW_DUMP_OPTIONS ""
#endif /* BCM_FE2000_SUPPORT */


#if defined(BCM_QE2000_SUPPORT) || defined(BCM_BME3200) || defined(BCM_BME3200) || defined(BCM_SIRIUS_SUPPORT)
#define CMD_SBX_FABRIC_SUPPORT
#endif


#ifdef CMD_SBX_FABRIC_SUPPORT
#define SBX_FABRIC_SW_DUMP_CMD_T(_bcm_module_)          \
static cmd_result_t                                                           \
_cmd_bcm_sbx_wb_##_bcm_module_##_sw_dump(int unit, args_t *args)              \
{                                                                             \
    extern void bcm_sbx_wb_##_bcm_module_##_sw_dump(int unit);                \
    if (SOC_IS_SBX_QE(unit) || SOC_IS_SBX_BME(unit)) {                        \
        bcm_sbx_wb_##_bcm_module_##_sw_dump(unit);                            \
    } else {                                                                  \
        printk(#_bcm_module_ " module sw_dump not supported on this unit\n"); \
        return CMD_FAIL;                                                      \
    }                                                                         \
    return CMD_OK;                                                            \
}

SBX_FABRIC_SW_DUMP_CMD_T(cosq)
SBX_FABRIC_SW_DUMP_CMD_T(fabric)
SBX_FABRIC_SW_DUMP_CMD_T(port)

#define SBX_FABRIC_SW_DUMP_OPTIONS "\tcosq|fabric|port (for QE/BME)"
#else /* CMD_SBX_FABRIC_SUPPORT */
#define SBX_FABRIC_SW_DUMP_OPTIONS ""
#endif /* CMD_SBX_FABRIC_SUPPORT */

/* cmd_sbx_sw_dump sub-command table */
static cmd_t _cmd_sw_mod_list[] = {
#ifdef BCM_FE2000_SUPPORT
    {"l3",      _cmd_fe2000_sw_dump_l3, 
                            "\n" CMD_SW_DUMP_HELP_T(l3),  NULL},
    {"mirror",  _cmd_fe2000_sw_dump_mirror, 
                            "\n" CMD_SW_DUMP_HELP_T(mirror),  NULL},
    {"oam",     _cmd_fe2000_sw_dump_oam, 
                            "\n" CMD_SW_DUMP_HELP_T(oam),     NULL},
    {"stg",     _cmd_fe2000_sw_dump_stg,
                            "\n" CMD_SW_DUMP_HELP_T(stg),  NULL},
    {"soc_pm",  _cmd_fe2000_sw_dump_soc_pm,
                            "\n" CMD_SW_DUMP_HELP_T(soc_pm),  NULL},
#endif /* BCM_FE2000_SUPPORT */

#ifdef CMD_SBX_FABRIC_SUPPORT
    {"cosq",    _cmd_bcm_sbx_wb_cosq_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(cosq), NULL},
    {"fabric",  _cmd_bcm_sbx_wb_fabric_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(fabric), NULL},
    {"port",    _cmd_bcm_sbx_wb_port_sw_dump,
                            "\n" CMD_SW_DUMP_HELP_T(port), NULL},
#endif /* CMD_SBX_FABRIC_SUPPORT */
};

#define SBX_SW_DUMP_OPTIONS FE2000_SW_DUMP_OPTIONS SBX_FABRIC_SW_DUMP_OPTIONS


/*
 * Function:
 *     cmd_sbx_sw_dump
 * Purpose:
 *     Provide interface to internal state of various bcm modules
 * Notes:
 *     Main driver to dump sw sub-commands
 */
cmd_result_t
cmd_sbx_sw_dump(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sw_mod_list, COUNTOF(_cmd_sw_mod_list));
}

#else
#define SBX_SW_DUMP_OPTIONS "Unavailable"
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

char cmd_sbx_dump_usage[] = 
    "SIRIUS device only Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "DUMP [options]\n"
#else
    "DUMP [File=<name>] [Append=true|false] [raw] [hex] [all] [chg]\n\t"
    "        <TABLE>[.<COPYNO>] [<INDEX>] [<COUNT>]\n\t"
    "        [-filter <FIELD>=<VALUE>[,...]]\n\t"
    "      If raw is specified, show raw memory words instead of fields.\n\t"
    "      If hex is specified, show hex data only (for Expect parsing).\n\t"
    "      If all is specified, show even empty or invalid entries\n\t"
    "      If chg is specified, show only fields changed from defaults\n\t"
    "      (Use \"listmem\" command to show a list of valid tables)\n\t"
    "DUMP PCIC                     (PCI config space)\n\t"
    "DUMP PCIM [<START> [<COUNT>]] (CMIC PCI registers)\n\t"
    "DUMP SOC [ADDR | RVAL | DIFF] (All SOC registers)\n\t"
    "      ADDR shows only addresses, doesn't actually load.\n\t"
    "      RVAL shows reset defaults, doesn't actually load.\n\t"
    "      DIFF shows only regs not equal to their reset defaults.\n\t"
    "DUMP SOCMEM [DIFF] (All SOC memories)\n\t"
    "      DIFF shows only memories not equal to their reset defaults.\n\t"
    "DUMP MW [<START> [<COUNT>]]   (System memory, 32 bits)\n\t"
    "DUMP MH [<START> [<COUNT>]]   (System memory, 16 bits)\n\t"
    "DUMP MB [<START> [<COUNT>]]   (System memory, 8 bits)\n\t"
    "DUMP SA                       (ARL shadow table)\n\t"
    "DUMP DV ADDR                  (DMA vector)\n\t"
    "DUMP PHY [<PHYID>]            See also, the 'PHY' command.\n"
#endif
    "Other SBX device Usage:\n"
    "\n\t"
    "DUMP PCIC                     (PCI config space)\n\t"
    "DUMP PCIM                     (Directly addressable PCI registers)\n\t"
    "DUMP SW " SBX_SW_DUMP_OPTIONS ">\n";

cmd_result_t
cmd_sbx_dump(int unit, args_t *args)
{
    char *mem;
    int rv = CMD_OK;
    sbx_regtype_entry_t *rt;

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return cmd_sbx_cmic_dump(unit, args);
    }
#endif

    if((mem = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if ((rt = sbx_regtype_lookup_name(mem)) != NULL) {
        switch(rt->type) {
        case sbx_pci_cfg_reg:
            _pci_print_config(unit);
            break;

        case sbx_cpureg:
            rv  = sbx_pcim_print_regs(unit, 1);
            rv |= sbx_pcim_print_regs(unit, 0);
            break;
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
        case sbx_sw:
            rv = cmd_sbx_sw_dump(unit, args);
            break;
#endif
        default:
            rv = CMD_USAGE;
        }
    } else {
        rv = CMD_USAGE;
    }

    return rv;
}



cmd_result_t  
sbx_pcim_print_regs(int unit, int pci_regs)
{
    int idx = 0;
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_t *reg_info;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit (%d). \n", unit);
        return CMD_FAIL;
    }
    if (!chip_info) {
        return CMD_FAIL;
    }
    
    for (idx = 0; idx < chip_info->nregs; idx++) {
        reg_info = chip_info->regs[idx];
        /* only list direct accessible registes */
        if (pci_regs) {
            if(reg_info->offset & SOC_SBX_REG_IS_INDIRECT) {
                continue;
            } 
        } else {
            if(!(reg_info->offset & SOC_SBX_REG_IS_INDIRECT)) {
                continue;
            }
        }
        sbx_reg_info_print(unit, reg_info, REG_PRINT_RAW);
    }
    
    printk("\n"); /* new line in the end */
    
    return CMD_OK;
}
/*###################################################*/

static cmd_result_t
_sbx_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t *v)
{
    sbhandle  sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    int       hi;
    uint32_t  offset, ioffset;
#ifdef BCM_FE2000_SUPPORT
    uint32_t  dhi, dlo;
    int       block_instance = 0;
    int       block_offset;
    int       xg_mac_ctrl_offset;
    int       ag_mac_ctrl_offset;
    int       num_ag_instances;
    int       num_xg_instances;
    int       ag_stride;
    int       xg_stride;
    int      is_xg_indirect = FALSE; 
    int      is_ag_indirect = FALSE; 
    int       i;
#endif /* BCM_FE2000_SUPPORT */

    offset = reg_info->offset;
    hi = !!(reg_info->ioffset & 0x08000000);
    ioffset = reg_info->ioffset & ~0x08000000;
    
    *v = 0;
#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE2000(unit)
        && offset & SOC_SBX_REG_IS_INDIRECT) {
        offset &= ~SOC_SBX_REG_IS_INDIRECT;
        xg_mac_ctrl_offset = SAND_HAL_FE2000_REG_OFFSET(sbh, XG0_MAC_ACC_CTRL);
        ag_mac_ctrl_offset = SAND_HAL_FE2000_REG_OFFSET(sbh, AG0_GPORT_MEM_ACC_CTRL);
        num_xg_instances = SAND_HAL_FE2000(sbh, XG_INSTANCE_COUNT);
        num_ag_instances = SAND_HAL_FE2000(sbh, AG_INSTANCE_COUNT);
        xg_stride = SAND_HAL_FE2000(sbh, XG_INSTANCE_ADDR_STRIDE);
        ag_stride = SAND_HAL_FE2000(sbh, AG_INSTANCE_ADDR_STRIDE);

        for(i=0; i< num_xg_instances; i++)
        {

            if( offset == (xg_mac_ctrl_offset + (i * xg_stride)))
            {
                is_xg_indirect = TRUE; 
                block_instance = i;
                break;
            }
        }

       if( is_xg_indirect != TRUE)
       {
            for(i=0; i < num_ag_instances; i++)
            {
                if( offset == (ag_mac_ctrl_offset + (i * ag_stride)))
                {
                    is_ag_indirect = TRUE;
                    block_instance = i;
                    break;
                }
            }
    
       } 

       if( is_xg_indirect == TRUE)
       {
           if (soc_sbx_fe2000_xm_read(unit, block_instance, ioffset, &dhi, &dlo) != 0) {
               printk("%s: xm_read error block=0x%x, offset=0x%x\n",SOC_CHIP_STRING(unit),
                      block_instance,
                      ioffset);
               return CMD_FAIL;
           }
           
           if (hi) {
               *v = dhi;
           } else {
               *v = dlo;
           }
       }
       else if(is_ag_indirect == TRUE)
       {
           if (ioffset & 0x00080000) {  /* GEN_REGTYPE */
                block_offset = SB_FE2000_MAX_AG_PORTS;
            } else {                     /* PORT_REGTYPE */
                block_offset = (ioffset >> 12) & 0xf;
            }
            ioffset &= 0xfff;

            if (soc_sbx_fe2000_am_read(unit, block_instance, block_offset,
                                       ioffset, v) != 0) {
              printk("%s: am_read error block=0x%x, offset=0x%x\n",SOC_CHIP_STRING(unit),
                     block_instance,
                     ioffset);
              return CMD_FAIL;
            }

       }
       else
       {
            sbFe2000UtilReadIndir(sbh, FALSE, FALSE, offset, ioffset, 1, v);
       }
    } else 

#endif /* BCM_FE2000_SUPPORT */
    {
        if (SOC_IS_SBX_FE2000(unit))
            *v = SAND_HAL_FE2000_READ_OFFS(sbh, reg_info->offset);
        else
            *v = SAND_HAL_READ_OFFS(sbh, reg_info->offset);
    }

    return CMD_OK;
}

static cmd_result_t
_sbx_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t v)
{
    sbhandle  sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    int       hi;
    uint32_t  offset, ioffset;
#ifdef BCM_FE2000_SUPPORT
    uint32_t  dhi, dlo;
    int       block_instance = 0;
    int       block_offset;
    int       xg_mac_ctrl_offset;
    int       ag_mac_ctrl_offset;
    int       num_ag_instances;
    int       num_xg_instances;
    int       ag_stride;
    int       xg_stride;
    int       is_xg_indirect = FALSE; 
    int      is_ag_indirect = FALSE; 
    int       i;
#endif /* BCM_FE2000_SUPPORT */

    offset = reg_info->offset;
    hi = !!(reg_info->ioffset & 0x08000000);
    ioffset = reg_info->ioffset & ~0x08000000;
    
#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE2000(unit)
        && offset & SOC_SBX_REG_IS_INDIRECT) {
        offset &= ~SOC_SBX_REG_IS_INDIRECT;
        xg_mac_ctrl_offset = SAND_HAL_FE2000_REG_OFFSET(sbh, XG0_MAC_ACC_CTRL);
        ag_mac_ctrl_offset = SAND_HAL_FE2000_REG_OFFSET(sbh, AG0_GPORT_MEM_ACC_CTRL);
        num_xg_instances = SAND_HAL_FE2000(sbh, XG_INSTANCE_COUNT);
        num_ag_instances = SAND_HAL_FE2000(sbh, AG_INSTANCE_COUNT);
        xg_stride = SAND_HAL_FE2000(sbh, XG_INSTANCE_ADDR_STRIDE);
        ag_stride = SAND_HAL_FE2000(sbh, AG_INSTANCE_ADDR_STRIDE);

        for(i=0; i< num_xg_instances; i++)
        {

            if( offset == (xg_mac_ctrl_offset + (i * xg_stride)))
            {
                is_xg_indirect = TRUE; 
                block_instance = i;
                break;
            }
        }

       if( is_xg_indirect != TRUE)
       {
            for(i=0; i < num_ag_instances; i++)
            {
                if( offset == (ag_mac_ctrl_offset + (i * ag_stride)))
                {
                    is_ag_indirect = TRUE;
                    block_instance = i;
                    break;
                }
            }
    
       } 

       if( is_xg_indirect == TRUE)
       {
           if (soc_sbx_fe2000_xm_read(unit, block_instance, ioffset, &dhi, &dlo) != 0) {
               printk("%s: xm_read error block=0x%x, offset=0x%x\n",SOC_CHIP_STRING(unit),
                      block_instance,
                      ioffset);
               return CMD_FAIL;
           }
           
           if (hi) {
               dhi = v;
           } else {
               dlo = v;
           }
           soc_sbx_fe2000_xm_write(unit, block_instance, ioffset, dhi, dlo);
       }
       else if(is_ag_indirect == TRUE)
       {
           if (ioffset & 0x00080000) {  /* GEN_REGTYPE */
                block_offset = SB_FE2000_MAX_AG_PORTS;
            } else {                     /* PORT_REGTYPE */
                block_offset = (ioffset >> 12) & 0xf;
            }
            ioffset &= 0xfff;

            soc_sbx_fe2000_am_write(unit, block_instance, block_offset,
                                    ioffset, v);

       }
       else
       {
         if ( sbFe2000UtilWriteIndir(sbh, FALSE, FALSE, offset, ioffset, 1, &v) ) {
           printk("sbFe2000UtilWriteIndir timed out\n");
           return CMD_FAIL;
         }
       }
    } else 

#endif /* BCM_FE2000_SUPPORT */
    {
        if (SOC_IS_SBX_FE2000(unit))
            SAND_HAL_FE2000_WRITE_OFFS(sbh, reg_info->offset, v);
        else
            SAND_HAL_WRITE_OFFS(sbh, reg_info->offset, v);
    }

    return CMD_OK;
}

/*###################################################*/
/*
 * Function:    sbx_chip_info_get
 * Purpose:     Returns a chip_info based on passed in chip_id
 *     NOTE: if not found, then create on for this chip_id
 */
cmd_result_t
sbx_chip_info_get(int chip_id, soc_sbx_chip_info_t **chip_info, int create)
{
    int idx = 0;
    int rv = CMD_FAIL;

    for (idx = 0; idx < soc_sbx_chip_count; idx++) {
        if (soc_sbx_chip_list[idx].id == chip_id) {
            *chip_info = &soc_sbx_chip_list[idx];
            return CMD_OK;
        }
    }

    if (create){
      /* create the list on the fly if not found */
      switch (chip_id){
#if defined(BCM_QE2000_SUPPORT)
      case SOC_INFO_QE2000: {
        extern int sbx_qe2000_reg_list_init(void);
        if ((rv = sbx_qe2000_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
#if defined(BCM_BME3200_SUPPORT)
      case SOC_INFO_BME3200: {
        extern int sbx_bme3200_reg_list_init(void);
        if ((rv = sbx_bme3200_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
#if defined(BCM_BM9600_SUPPORT)
      case SOC_INFO_BM9600:{
        extern int sbx_bm9600_reg_list_init(void);
        if ((rv = sbx_bm9600_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
        
#if defined(BCM_FE2000_SUPPORT)
      case SOC_INFO_FE2000:{
        extern int sbx_fe2000_reg_list_init(void);
        if ((rv = sbx_fe2000_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
      case SOC_INFO_FE2000XT:{
        extern int sbx_fe2000xt_reg_list_init(void);
        if ((rv = sbx_fe2000xt_reg_list_init()) < 0) {
          return rv;
        }
        break;
      }
#endif
      } /* switch */
      
      /* Try again to find the right info structure */
      for (idx = 0; idx < soc_sbx_chip_count; idx++) {
        if (soc_sbx_chip_list[idx].id == chip_id) {
          *chip_info = &soc_sbx_chip_list[idx];
          return CMD_OK;
        }
      }
    } /* if create */
    return rv;
}


/*
 * Function:    sbx_reg_info_list_get
 * Purpose:     Finds list of registers matching regname
 */
cmd_result_t 
sbx_reg_info_list_get(soc_sbx_chip_info_t *chip_info,
                      soc_sbx_reg_info_list_t *reginfo_l,
                      char *regname, int exactmatch)
{
    int idx = 0;
    soc_sbx_reg_info_t *reg_info = NULL;

    if ((!chip_info) || (!reginfo_l)) {
        return CMD_FAIL;
    }

    for (idx = 0; idx < chip_info->nregs; idx++) {
        reg_info = chip_info->regs[idx];
        if (exactmatch) {
            if (!sal_strcasecmp(reg_info->name, regname)) {
                reginfo_l->idx[reginfo_l->count++] = idx;
                break;
            }
        } else {
            if (strstr(reg_info->name, regname)) {
                reginfo_l->idx[reginfo_l->count++] = idx;
            }
        }
    }

    return CMD_OK;
}

void
sbx_reg_info_list_prune(soc_sbx_chip_info_t *chip_info,
                        soc_sbx_reg_info_list_t *reginfo_l,
                        char *regname)
{
    int i;
    int idx;
    soc_sbx_reg_info_t *reg_info = NULL;

    for (i = 0; i < reginfo_l->count; i++) {
        idx = reginfo_l->idx[i];
        if (idx >= 0) {
            reg_info = chip_info->regs[idx];
            if (strstr(reg_info->name, regname)) {
                reginfo_l->idx[i] = -1;
            }
        }
    }
}

/*
 * Function:    sbx_fe2000_reg_list_prune_reset
 * Purpose:     Prune blocks of chip which are in reset
 */
void
sbx_fe2000_reg_list_prune_reset(int unit,
                             soc_sbx_chip_info_t *chip_info,
                             soc_sbx_reg_info_list_t *reginfo_l)
{
    uint32_t pc_core_reset;

    /* prune blocks in reset */
    pc_core_reset = SAND_HAL_FE2000_READ(unit, PC_CORE_RESET);
    if (pc_core_reset & SAND_HAL_FE2000_FIELD_MASK(unit, PC_CORE_RESET, AG0_CORE_RESET) ){
            printk("Skipping AG0 (in reset)\n");
            sbx_reg_info_list_prune(chip_info, reginfo_l, "ag0");
    }
    if (pc_core_reset & SAND_HAL_FE2000_FIELD_MASK(unit, PC_CORE_RESET, AG1_CORE_RESET) ){
            printk("Skipping AG1 (in reset)\n");
            sbx_reg_info_list_prune(chip_info, reginfo_l, "ag1");
    }
    if (pc_core_reset & SAND_HAL_FE2000_FIELD_MASK(unit, PC_CORE_RESET, XG0_CORE_RESET) ){
            printk("Skipping XG0 (in reset)\n");
            sbx_reg_info_list_prune(chip_info, reginfo_l, "xg0");
    }
    if (pc_core_reset & SAND_HAL_FE2000_FIELD_MASK(unit, PC_CORE_RESET, XG1_CORE_RESET) ){
            printk("Skipping XG1 (in reset)\n");
            sbx_reg_info_list_prune(chip_info, reginfo_l, "xg1");
    }
    if (SOC_IS_SBX_FE2KXT(unit)){
        if (pc_core_reset & SAND_HAL_C2_PC_CORE_RESET_XG2_CORE_RESET_MASK) {
                printk("Skipping XG2 (in reset)\n");
                sbx_reg_info_list_prune(chip_info, reginfo_l, "xg2");
        }
        if (pc_core_reset & SAND_HAL_C2_PC_CORE_RESET_XG3_CORE_RESET_MASK) {
                printk("Skipping XG3 (in reset)\n");
                sbx_reg_info_list_prune(chip_info, reginfo_l, "xg3");
        }
    }
}
/* 
 * Function:    sbx_reg_info_get
 * Purpose:     Find a reg given a name or an addr
 */
cmd_result_t 
sbx_reg_info_get(soc_sbx_chip_info_t *chip_info, soc_sbx_reg_info_t **reg_info,
                 char *reg)
{
    int     idx = 0;
    uint32  regaddr = 0;
    int     isaddr = 0;
    int     found = 0;

    if ((!chip_info) || (!reg_info)) {
        return CMD_FAIL;
    }

    if (isint(reg)) {
        regaddr = parse_integer(reg);
        isaddr = 1;
    }

    for (idx = 0; idx < chip_info->nregs; idx++) {
        if (isaddr) {
            if (chip_info->regs[idx]->offset == regaddr) {
                found = 1;
                break;
            }
        } else {
            if (!sal_strcasecmp(chip_info->regs[idx]->name, reg)) {
                found = 1;
                break;
            }
        }
    }

    if (found) {
        *reg_info = chip_info->regs[idx];
        return CMD_OK;
    }

    return CMD_FAIL;
}

/*
 * Function:    sbx_reg_fields_list
 * Purpose:     List all fields in the reg_info
 */
cmd_result_t
sbx_reg_fields_list(soc_sbx_reg_info_t *reg_info)
{
    int idx = 0;
    soc_sbx_field_info_t *fld_info = NULL;
    
    if (!reg_info) {
        return CMD_FAIL;
    }

    printk("%s: \n", reg_info->name);
    for (idx = 0; idx < reg_info->nfields; idx++) {
        fld_info = reg_info->fields[idx];
        if (fld_info) {
            printk("    FIELD:%s,  MASK:0x%x,  MSB:%d,  LSB:%d,  DEFAULT:%x \n", 
                   fld_info->name, fld_info->mask, fld_info->msb, 
                   fld_info->lsb, fld_info->default_val);
        }
    }
    return CMD_OK;
}


/*
 * Function:    sbx_reg_list_all
 * Purpose:     List all SBX registers for passed in chip_info
 */
cmd_result_t
sbx_reg_list_all(soc_sbx_chip_info_t *chip_info)
{
    int idx = 0;
    
    if (!chip_info) {
        return CMD_FAIL;
    }

    printk("List of all %s Registers: \n", chip_info->name);
    for (idx = 0; idx < chip_info->nregs; idx++) {
        if ((idx % 3) == 2) {
            printk("\n"); /* new line every 4 regs */
        }
        printk("%s  ", chip_info->regs[idx]->name);
    }

    printk("\n"); /* new line in the end */

    return CMD_OK;
}


/* 
 * Function:    sbx_reg_list
 * Purpose:     prints list of regs
 */
cmd_result_t 
sbx_reg_list(soc_sbx_chip_info_t *chip_info, char *regname)
{
    int idx = 0;
    int rv = CMD_OK;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;

    if (!chip_info) {
        return CMD_FAIL;
    }

    if (!regname) {
        return CMD_USAGE;
    }

    if (*regname == '*') {
        return sbx_reg_list_all(chip_info);
    }

    /* Now search for exact or partial matches */
    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        printk("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    /* Initialize to empty */
    reginfo_l->count = 0;

    /* first find exact match */
    if (sbx_reg_info_list_get(chip_info,reginfo_l ,regname , 1) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (reginfo_l->count == 1) {
        /* found exact match */
        rv = sbx_reg_fields_list(chip_info->regs[reginfo_l->idx[0]]);
        sal_free(reginfo_l);
        return rv;
    }

    reginfo_l->count = 0; /* zero out the count */
    /* Now find partial matches */
    if (sbx_reg_info_list_get(chip_info,reginfo_l ,regname , 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    /* now list all regs */
    for (idx = 0; idx < reginfo_l->count; idx++) {
        printk("%s  ", chip_info->regs[reginfo_l->idx[idx]]->name);
        if ((idx % 3) == 2) {
            printk("\n"); /* new line every 4 regs */
        }
    }

    if (idx % 3) {
        printk("\n"); /* new line in the end */
    }

    sal_free(reginfo_l);
    return CMD_OK;
}

STATIC void
_sbx_reg_print(uint32 regval, soc_sbx_reg_info_t *reg_info, uint32 flags)
{
    int         idx = 0;
    int         def_chg = 0;
    uint32      fld_val;
    soc_sbx_field_info_t    *fld_info = NULL;

    if (flags & REG_PRINT_HEX) {
        printk("0x%08x", regval);
    } else if (flags & REG_PRINT_RAW) {
        printk("%-30s: 0x%08x", reg_info->name, regval);
    } else {
      printk("%s: 0x%08x\n", reg_info->name, regval);
        for (idx = 0; idx < reg_info->nfields; idx++) {
            fld_info = reg_info->fields[idx];
            fld_val = ((regval & fld_info->mask) >> fld_info->shift);
            if ((!(flags & REG_PRINT_CHG)) 
                || (fld_val != fld_info->default_val)) {
                printk("\t%30s:  0x%08x\n", fld_info->name, fld_val);
                def_chg = 1;
            }
        }
        if ((flags & REG_PRINT_CHG) && (!def_chg)) {
            printk("ALL FIELDS SET TO DEFAULT VALUES");
        }
        printk("\n");
    }
}

/*
 * Function:    sbx_reg_print
 * Purpose:     Print the values of the register
 */
cmd_result_t
sbx_reg_info_print(int unit, soc_sbx_reg_info_t *reg_info, uint32 flags)
{
    uint32       regval;
    cmd_result_t rv;

    if (!reg_info) {
        return CMD_FAIL;
    }

    rv = _sbx_read_reg(unit, reg_info, &regval);
    if (rv != CMD_OK) {
        return rv;
    }
    
    _sbx_reg_print(regval, reg_info, flags);

#if (defined(VENDOR_BROADCOM) && defined(BCM_SBX_SUPPORT))
    /* allow polaris emulator team getreg to return value instead of command status */
    if ( SOC_IS_SBX(unit) && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        return regval;
    } 
    else 
#endif
    {
        return CMD_OK;
    }
}

/*
 * Function:    sbx_reg_addr_print
 * Purpose:     Check if a register address is valid and print it
 */
cmd_result_t
sbx_reg_print(int unit, char *reg, uint32 flags)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_t *reg_info = NULL;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info, 1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }
    
    if (sbx_reg_info_get(chip_info, &reg_info, reg) != CMD_OK) {
        soc_sbx_reg_info_list_t *reginfo_l = NULL;
        int idx;

        /* Now search for exact or partial matches */
        reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");

        if (!reginfo_l) {
            printk("ERROR: sbx_reg_list failed. Out of Memory \n");
            return CMD_FAIL;
        }
        /* Zero out the struct. */
        sal_memset(reginfo_l, 0, sizeof(soc_sbx_reg_info_list_t));
   
        reginfo_l->count = 0; /* zero out the count */
        /* Now find partial matches */
        if (sbx_reg_info_list_get(chip_info,reginfo_l ,reg , 0) != CMD_OK) {
            sal_free(reginfo_l);
            printk("ERROR: Invalid Register: %s on %s \n", reg, chip_info->name);
            return CMD_FAIL;
        }

        /* now dump values for all partial match regs */
        for (idx = 0; idx < reginfo_l->count; idx++) {
             if (sbx_reg_info_get(chip_info, 
                                  &reg_info, 
                                  chip_info->regs[reginfo_l->idx[idx]]->name) == CMD_OK) {
                 sbx_reg_info_print(unit, reg_info, flags);
             }
             printk("\n"); /* new line in the end */
        }
        sal_free(reginfo_l);

        return CMD_OK;
    }

    return sbx_reg_info_print(unit, reg_info, flags);
}


/*
 * Function:    sbx_reg_name_set
 * Purpose:     Check if a register name is valid and set it
 */
cmd_result_t
sbx_reg_set(int unit, char *reg, args_t *a, int mod)
{
    int         idx = 0;
    uint32      regval = 0;
    uint32      oldregval = 0;
    uint32      value = 0;
    char        *valstr = NULL;
    int         valstrlen = 0;
    uint32      regmask = 0;
    char        fldname [SBX_FIELD_NAME_LEN_MAX];
    char        *name = NULL;
    soc_sbx_chip_info_t     *chip_info = NULL;
    soc_sbx_reg_info_t      *reg_info = NULL;
    soc_sbx_field_info_t    *fld_info = NULL;
    cmd_result_t rv;
    sbhandle sbh;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }

    if (sbx_reg_info_get (chip_info, &reg_info, reg) != CMD_OK) {
        printk("ERROR: Invalid Register: %s \n", reg);
        return CMD_FAIL;
    }

    name = ARG_GET(a);
    if (!name) {
        return CMD_USAGE;
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    if (isint(name)) {
        /* complete register value given */
        regval = parse_integer(name);
        return _sbx_write_reg(unit, reg_info, regval);
    }

    /* individual fields specified */
    for (;;) {
        if (!name) {
            break;
        }
        valstr = strchr(name, '=');
        if ((valstr == NULL) || (valstr >= (name + (sal_strlen(name) - 1)))) {
            return CMD_USAGE;
        }
        valstr++; /* value starts after = sign */
        if (!isint(valstr)) {
            return CMD_USAGE;
        }
        
        valstrlen = (sal_strlen(name) - sal_strlen(valstr) - 1);
        if (valstrlen > (SBX_FIELD_NAME_LEN_MAX - 1)) {
            return CMD_USAGE;
        }
        value =  parse_integer(valstr);
        strncpy(fldname, name, (sal_strlen(name) - sal_strlen(valstr) - 1));
        *(fldname + valstrlen) = '\0';

        /* check for valid field name */
        fld_info = NULL;
        for (idx = 0; idx < reg_info->nfields; idx++) {
            if (!sal_strcasecmp(reg_info->fields[idx]->name, fldname)) {
                fld_info = reg_info->fields[idx];
                break;
            }
        }
        if (!fld_info) {
            printk("ERROR: Invalid field: %s specified for register: %s  \n",
                   fldname, reg_info->name);
            return CMD_FAIL;
        }

        regval |= (value << fld_info->shift);
        regmask |= fld_info->mask;

        name = ARG_GET(a);
    }

    if (!regmask) {
        return CMD_USAGE;
    }

    rv = _sbx_read_reg(unit, reg_info, &oldregval);
    if (rv != CMD_OK) {
        return rv;
    }
    oldregval &= (~regmask); /* zero out the fields being modified */
    rv = _sbx_write_reg(unit, reg_info, ((mod?oldregval:0) | regval));
    return CMD_OK;
}

STATIC int
_sbx_reg_get_by_type(int unit, uint32 regaddr, sbx_regtype_t regtype,
                     uint64 *outval, uint32 flags)
{
    int           rv = CMD_OK;
    int             is64 = FALSE;

    switch (regtype) {
    case sbx_pci_cfg_reg:
        if (regaddr & 3) {
            printk("ERROR: PCI config addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
        }
        break;

    case sbx_cpureg:
        if (regaddr & 3) {
            printk("ERROR: PCI memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
#ifdef BCM_FE2000_SUPPORT
            if (SOC_IS_SBX_FE2000(unit)
                && regaddr & SOC_SBX_REG_IS_INDIRECT) {
                return CMD_OK;
            }
#endif /* BCM_FE2000_SUPPORT */
    
            COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));           
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }
    
    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
        if (flags & REG_PRINT_HEX) {
            if (is64) {
                printk("%08x%08x\n",
                       COMPILER_64_HI(*outval),
                       COMPILER_64_LO(*outval));
            } else {
                printk("%08x\n",
                       COMPILER_64_LO(*outval));
            }
        } else {
            char buf[80];
            
            format_uint64(buf, *outval);
            
            printk("%s[0x%x] = %s\n",
                   sbx_regtypenames[regtype], regaddr, buf);
        }
    }

    return rv;
}

char cmd_sbx_reg_get_usage[] =
  "SIRIUS only Usage:\n"
  "Parameters: [hex|raw|chg] [<REGTYPE>] <REGISTER>\n\t"
  "If <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
  "<REGISTER> designates offset, address, or symbolic name.\n\t"
  "If hex is specified, dumps only a hex value (for Expect parsing).\n\t"
  "If raw is specified, no field decoding is done.\n\t"
  "If chg is specified, show only fields/regs changed from defaults.\n\t"
  "For a list of register types, use \"dump\".\n"
  "\n"
  "\n"
  "Other SBX device Usage:\n"
  "Parameters: [hex|raw|chg] <REGISTER> \n \t"
  "If hex is specified, dumps only a hex value (for Expect parsing).\n\t"
  "If raw is specified, no field decoding is done.\n\t"
  "If chg is specified, show only fields/regs changed from defaults.\n\t"
  "For a list of register types, use \"listreg\".\n";

/*
 * Function:    cmd_sbx_reg_get
 * Purpose:         Register Read command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_get(int unit, args_t *a)
{
    char                *name;
    uint32              flags = REG_PRINT_DO_PRINT;
    uint32              regaddr = 0;
    sbx_regtype_entry_t *rt;
    int                 rv = CMD_OK;
    uint64              rval;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return cmd_sbx_cmic_reg_get(unit, a);
    }
#endif

    name = ARG_GET(a);

    /* 
     * If first arg specifies print options, take it and use the next argument
     * as the name or address
     */

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "chg")) {
            flags |= REG_PRINT_CHG;
            name = ARG_GET(a);
        } else {
            break;
        }
    }
    
    if (name == NULL) {
        return CMD_USAGE;
    }

    /* parse the type of register, default to PCIM */
    if ((rt = sbx_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_regtype_lookup_name("PCIM"); /* i.e. sbx_cpureg */
    }

    assert(rt);

    if (isint(name)) {
        /* Numerical address given */
        regaddr = parse_integer(name);
    } else {
        if (*name == '$') {
            name++;
        }
    }

    switch(rt->type) {
    case sbx_cpureg:
        rv = sbx_reg_print(unit, name, flags);
        break;
        
    default:
        if (!isint(name)) {
            printk("ERROR: Numeric register address expected\n");
            rv = CMD_FAIL;
        } else {
            rv = _sbx_reg_get_by_type(unit, regaddr, rt->type, &rval, flags);
        }
    }
    
    return rv;
}

char cmd_sbx_reg_set_usage[] = 
  "SIRIUS only Usage:\n"
  "1. Parameters: [<REGTYPE>] <REGISTER> <VALUE>\n\t"
  "If <REGTYPE> is not specified, it defaults to \"soc\".\n\t"
  "<REGISTER> is offset, address, or symbolic name.\n"
  "2. Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
  "<REGISTER> is SOC register offset or symbolic name.\n\t"
  "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
  "for example: L3_ENA=0,CPU_PRI=1.\n\t"
  "Fields not specified in the list are set to zero.\n\t"
  "For a list of register types, use \"help dump\".\n"
  "\n"
  "\n"
  "Other SBX device Usage:\n"
  "1. Parameters: <REGISTER> <VALUE>\n\t"
  "2. Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
  "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
  "Fields not specified in the list are set to zero.\n";

/*
 * Function:    cmd_sbx_reg_set
 * Purpose:         Register Write command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_set(int unit, args_t *a)
{
    char    *name;
    uint32  regaddr;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return cmd_sbx_cmic_reg_set(unit, a);
    }
#endif

    name = ARG_GET(a);

    if (name == NULL) {
    return CMD_USAGE;
    }

    if (isint(name)) {                 
        /* Numerical address given */
        regaddr = parse_integer(name);
        if (regaddr & 3) {
            printk("ERROR: Address must be multiple of 4 \n");
            return CMD_FAIL;
        }
    } else {
        if (*name == '$') {
            name++;
        }
    }

    return sbx_reg_set(unit, name, a, 0);
}

char cmd_sbx_reg_modify_usage[] = 
    "Parameters: <REGISTER> <FIELD>=<VALUE>[,...]\n\t"
    "<FIELD>=<VALUE>[,...] is a list of fields to affect,\n\t"
    "Fields not specified in the list are left unchanged.\n";

/*
 * Function:    cmd_sbx_reg_modify
 * Purpose:         Register Read/Modify/Write command on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_modify(int unit, args_t *a)
{
    char    *name;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return cmd_sbx_cmic_reg_mod(unit, a);
    }
#endif

    name = ARG_GET(a);

    if (name == NULL) {
    return CMD_USAGE;
    }

    if (*name == '$') {
        name++;
    }

    return sbx_reg_set(unit, name, a, 1);
}


char cmd_sbx_reg_verify_usage[] =
    "Parameters: <REGISTER> <VALUE>\n \t"
    "Write a register and read it back to verify the value written.\n";
/*
 * Function:    cmd_sbx_reg_verify
 * Purpose:         Write device register and read it back on SBX device
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_verify(int unit, args_t *a)
{
    char                *name,*value;
    int                 rv = CMD_OK;
    uint32      regval = 0;
    soc_sbx_chip_info_t     *chip_info = NULL;
    soc_sbx_reg_info_t      *reg_info = NULL;
    uint32                  set_val;
    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (SOC_IS_SBX_SIRIUS(unit)) {
        printk("ERROR: Command not supported on Sirius devie \n");
        return CMD_FAIL;
    }

    /* remember register name */
    name = ARG_GET(a);
    if (name == NULL) {
        return CMD_USAGE;
    }
    value = ARG_GET(a);
    if (value == NULL) {
        return CMD_USAGE;
    }

    ARG_PREV(a);
    ARG_PREV(a);

    /* write register */
    rv = cmd_sbx_reg_set(unit, a);
    if (rv < 0 ) {
        return rv;
    }

    set_val = parse_integer(value);

    /* read it back */
    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit %d \n", unit);
        return CMD_FAIL;
    }
    
    if (sbx_reg_info_get (chip_info, &reg_info, name) != CMD_OK) {
        printk("ERROR: Invalid Register: %s \n", name);
        return CMD_FAIL;
    }
    
    _sbx_read_reg(unit, reg_info, &regval);
    if(set_val != regval) {
        printk("Error. Written Value 0x%08X != Read Value 0x%08X\n",
               set_val, regval);
    }
    return CMD_OK;
}


char cmd_sbx_reg_list_usage[] =
  "SIRIUS only Usage: listreg [options] regname [value]\n"
  "Options:\n"
  "     -alias/-a       display aliases\n"
  "     -summary/-s     display single line summary for each reg\n"
  "     -counters/-c    display counters\n"
  "     -ed/-e          display error/discard counters\n"
  "     -type/-t        display registers grouped by block type\n"
  "If regname is '*' or does not match a register name, a substring\n"
  "match is performed.  [] indicates register array.\n"
  "If regname is a numeric address, the register that resides at that\n"
  "address is displayed.\n"
  "\n"
  "\n"
  "Other SBX device Usage: Listreg regname \n"
  "If regname is '*' or does not match a register name, a substring\n"
  "match is performed.  [] indicates register array.\n"
  "If regname is a numeric address, the register that resides at that\n"
  "address is displayed.\n";

/*
 * Function:    cmd_sbx_reg_list
 * Purpose:         List of Registers
 * Parameters:  unit - unit number of device
 *                      a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sbx_reg_list(int unit, args_t *a)
{
    char *name;

    soc_sbx_chip_info_t *chip_info = NULL;

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return cmd_sbx_cmic_reg_list(unit, a);
    }
#endif

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit (%d). \n", unit);
        return CMD_FAIL;
    }

    name = ARG_GET(a);
    sbx_str_tolower(name);
    return sbx_reg_list(chip_info, name);
}

static cmd_result_t
_cmd_sbx_print_reg_class(int unit, args_t *a, soc_sbx_chip_info_t *chip_info,
                         soc_sbx_reg_info_list_t *reginfo_l, int flags)
{
    sbhandle sbh;
    int raw = 0;
    int hex = 0;
    int clear = 0;
    soc_sbx_reg_info_t *reg_info;
    uint32 regval;
    int i;
    int idx;
    pbmp_t pbmp;
    bcm_port_t port;
    cmd_result_t rv;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "raw", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &raw, NULL);
        parse_table_add(&pt, "hex", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &hex, NULL);
        parse_table_add(&pt, "clear", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &clear, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    for (i = 0; i < reginfo_l->count; i++) {
        idx = reginfo_l->idx[i];
        if (idx >= 0) {
            reg_info = chip_info->regs[idx];
            rv = _sbx_read_reg(unit, reg_info, &regval);
            if (rv != CMD_OK) {
                return rv;
            }
            if (regval != 0) {
                _sbx_reg_print(regval, reg_info,
                               (flags
                                | (raw ? REG_PRINT_RAW : 0)
                                | (hex ? REG_PRINT_HEX : 0)));
		printk("\n");
                if (clear) {
                    /* Clear W1TC fields */
                    rv = _sbx_write_reg(unit, reg_info, regval);
                    if (rv != CMD_OK) {
                        return rv;
                    }
                    /* Clear other fields (e.g. counters) */
                    rv = _sbx_write_reg(unit, reg_info, 0);
                    if (rv != CMD_OK) {
                        return rv;
                    }
                }
            }
        }
    }

    /* Clear software counter statistics */
    if (clear) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        PBMP_ITER(pbmp, port) {
            bcm_stat_clear(unit, port);
        }
    }

    return CMD_OK;
}


char cmd_sbx_print_errors_usage[] =
    "Usage: PrintErrors [args]\n"
    "    hex    - prints in hex format\n"
    "    raw    - prints in raw format\n"
    "    clear  - writes 1s to clear\n";

cmd_result_t
cmd_sbx_print_errors(int unit, args_t *a)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;
    int rv;
#if defined(BCM_SIRIUS_SUPPORT)
    char *str = "error";
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)){
        rv = cmd_sirius_print_info(unit, str, 1, a);
        return (rv);
    }
#endif

    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        printk("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    reginfo_l->count = 0;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit %d \n", unit);
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (sbx_reg_info_list_get(chip_info, reginfo_l, "error", 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    sbx_reg_info_list_prune(chip_info, reginfo_l, "_mask");

    if (SOC_IS_SBX_FE2000(unit)) {
        sbx_fe2000_reg_list_prune_reset(unit, chip_info, reginfo_l);
    }

    rv = _cmd_sbx_print_reg_class(unit, a, chip_info, reginfo_l, 0);

    sal_free(reginfo_l);
    return rv;
}

char cmd_sbx_print_counts_usage[] =
    "Usage: PrintCounts [args]\n"
    "    hex    - prints in hex format\n"
    "    raw    - prints in raw format\n"
    "    clear  - clears any counts that are not normally clear-on-read\n";
cmd_result_t
cmd_sbx_print_counts(int unit, args_t *a)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_list_t *reginfo_l = NULL;
    int rv;
#if defined(BCM_SIRIUS_SUPPORT)
    char *str1 = "cnt";
    char *str2 = "count";
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)){
        rv = cmd_sirius_print_info(unit,str1,1, a);
        rv = cmd_sirius_print_info(unit,str2,1, a);
        return (rv);
    }
#endif

    reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!reginfo_l) {
        printk("ERROR: sbx_reg_list failed. Out of Memory \n");
        return CMD_FAIL;
    }
    reginfo_l->count = 0;

    if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
        printk("ERROR: Register info unknown for unit %d \n", unit);
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (sbx_reg_info_list_get(chip_info, reginfo_l, "cnt", 0) != CMD_OK) {
        sal_free(reginfo_l);
        return CMD_FAIL;
    }

    if (SOC_IS_SBX_FE2000(unit)) {
        sbx_fe2000_reg_list_prune_reset(unit, chip_info, reginfo_l);
    }

    rv = _cmd_sbx_print_reg_class(unit, a, chip_info, reginfo_l,
                                  REG_PRINT_RAW);

    sal_free(reginfo_l);
    return rv;
}

char cmd_sbx_print_info_usage[] =
    "Usage: PrintInfo searchstr [args]\n"
    "    hex     - prints in hex format\n"
    "    raw     - prints in raw format\n"
    "    all     - prints all registers\n"
    "    pattern - find only pattern in searchstr\n"
    "   ~pattern - ignore pattern in searchstr\n";

cmd_result_t
cmd_sbx_print_info(int unit, args_t *a)
{
    int rv = CMD_NOTIMPL;
    char *str = ARG_GET(a);

    if (str == 0)
        return CMD_FAIL;

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)){
        rv = cmd_sirius_print_info(unit,str, 0, a);
        return (rv);
    }
#endif
    return rv;
}

/* routines used only for diags */
cmd_result_t
sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t *v)
{
  return _sbx_read_reg(unit,reg_info,v);
}

cmd_result_t
sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t v)
{
  return _sbx_write_reg(unit,reg_info,v);
}

cmd_result_t
sbx_clear_all(int unit, uint8_t bErrorsOnly)
{
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_list_t *reginfo_l = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  int rv = CMD_OK;
  sbhandle sbh;
  uint32_t i;
  uint32_t regval = 0;
  int idx;
  pbmp_t pbmp;
  bcm_port_t port;

  if (!soc_attached(unit)) {
    printk("Error Unit %d not attached\n",unit);
    return FALSE;
  }

  reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
  if (!reginfo_l) {
    printk("ERROR: sbx_reg_list failed. Out of Memory \n");
    return CMD_FAIL;
  }
  reginfo_l->count = 0;

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(reginfo_l);
    return CMD_FAIL;
  }

  if (sbx_reg_info_list_get(chip_info, reginfo_l, "error", 0) != CMD_OK) {
    sal_free(reginfo_l);
    return CMD_FAIL;
  }

  if (!bErrorsOnly) {
    if (sbx_reg_info_list_get(chip_info, reginfo_l, "cnt", 0) != CMD_OK) {
      sal_free(reginfo_l);
      return CMD_FAIL;
    }
  }

  sbx_fe2000_reg_list_prune_reset(unit, chip_info, reginfo_l);

  sbh = SOC_SBX_CONTROL(unit)->sbhdl;
  
  for (i = 0; i < reginfo_l->count; i++) {
    idx = reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = _sbx_read_reg(unit, reg_info, &regval);
      if (rv != CMD_OK) {
        return rv;
      }
      if (regval != 0) {
          /* Clear W1TC fields */
          rv = _sbx_write_reg(unit, reg_info, regval);
          if (rv != CMD_OK) {
            return rv;
          }
          /* Clear other fields (e.g. counters) */
          rv = _sbx_write_reg(unit, reg_info, 0);
          if (rv != CMD_OK) {
            return rv;
          }
      }
    }
  }

  /* Clear software counter statistics */
  BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
  PBMP_ITER(pbmp, port) {
    bcm_stat_clear(unit, port);
  }

  sal_free(reginfo_l);
  return rv;

}


#if defined(BCM_SIRIUS_SUPPORT)

/* 
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */

static void
collect_comma_args(args_t *a, char *valstr, char *first)
{
    char           *s;

    sal_strcpy(valstr, first);

    while ((s = ARG_GET(a)) != 0) {
        strcat(valstr, ",");
        strcat(valstr, s);
    }
}

/* 
 * modify_reg_fields
 *
 *   Takes a soc_reg_t 'regno', pointer to current value 'val',
 *   and a string 'mod' containing a field replacement spec of the form
 *   "FIELD=value".   The string may contain more than one spec separated
 *   by commas.  Updates the field in the register value accordingly,
 *   leaving all other unmodified fields intact.  If the string CLEAR is
 *   present in the list, the current register value is zeroed.
 *   If mask is non-NULL, it receives a mask of all fields modified.
 *
 *   Examples with modreg:
 *        modreg fe_mac1 lback=1        (modifies only lback field)
 *        modreg config ip_cfg=2        (modifies only ip_cfg field)
 *        modreg config clear,ip_cfg=2,cpu_pri=4  (zeroes all other fields)
 *
 *   Note that if "clear" appears in the middle of the list, the
 *   values in the list before "clear" are ignored.
 *
 *   Returns -1 on failure, 0 on success.
 */

static int
modify_reg_fields(int unit, soc_reg_t regno,
                  uint64 *val, uint64 *mask /* out param */, char *mod)
{
    soc_field_info_t *fld;
    char           *fmod, *fval;
    char           *modstr;
    soc_reg_info_t *reg = &SOC_REG_INFO(unit, regno);
    uint64          fvalue;
    uint64          fldmask;
    uint64          tmask;

    if ((modstr = sal_alloc(ARGS_BUFFER, "modify_reg")) == NULL) {
        printk("modify_reg_fields: Out of memory\n");
        return CMD_FAIL;
    }

    strncpy(modstr, mod, ARGS_BUFFER);/* Don't destroy input string */
    modstr[ARGS_BUFFER - 1] = 0;
    mod = modstr;

    if (mask) {
        COMPILER_64_ZERO(*mask);
    }

    while ((fmod = strtok(mod, ",")) != 0) {
        mod = NULL;                    /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval) {                    /* Point fval to arg, NULL if none */
            *fval++ = 0;               /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0) {
            printk("Null field name\n");
            sal_free(modstr);
            return -1;
        }
        if (!sal_strcasecmp(fmod, "clear")) {
            COMPILER_64_ZERO(*val);
            if (mask) {
                COMPILER_64_ALLONES(*mask);
            }
            continue;
        }
        for (fld = &reg->fields[0]; fld < &reg->fields[reg->nFields]; fld++) {
            if (!sal_strcasecmp(fmod, SOC_FIELD_NAME(unit, fld->field))) {
                break;
            }
        }
        if (fld == &reg->fields[reg->nFields]) {
            printk("No such field \"%s\" in register \"%s\".\n",
                   fmod, SOC_REG_NAME(unit, regno));
            sal_free(modstr);
            return -1;
        }
        if (!fval) {
            printk("Missing %d-bit value to assign to \"%s\" "
                   "field \"%s\".\n",
                   fld->len, SOC_REG_NAME(unit, regno), SOC_FIELD_NAME(unit, fld->field));
            sal_free(modstr);
            return -1;
        }
        fvalue = parse_uint64(fval);

        /* Check that field value fits in field */
        COMPILER_64_MASK_CREATE(tmask, fld->len, 0);
        COMPILER_64_NOT(tmask);
        COMPILER_64_AND(tmask, fvalue);

        if (!COMPILER_64_IS_ZERO(tmask)) {
            printk("Value \"%s\" too large for %d-bit field \"%s\".\n",
                   fval, fld->len, SOC_FIELD_NAME(unit, fld->field));
            sal_free(modstr);
            return -1;
        }

        if (reg->flags & SOC_REG_FLAG_64_BITS) {
            soc_reg64_field_set(unit, regno, val, fld->field, fvalue);
        } else {
            uint32          tmp;
            uint32 ftmp;
            COMPILER_64_TO_32_LO(tmp, *val);
            COMPILER_64_TO_32_LO(ftmp, fvalue);
            soc_reg_field_set(unit, regno, &tmp, fld->field, ftmp);
            COMPILER_64_SET(*val, 0, tmp);
            COMPILER_64_SET(fvalue, 0, ftmp);
        }

        COMPILER_64_MASK_CREATE(fldmask, fld->len, fld->bp);
        if (mask) {
            COMPILER_64_OR(*mask, fldmask);
        }
    }

    sal_free(modstr);
    return 0;
}

#define PRINT_COUNT(str, len, wrap, prefix) \
    if ((wrap > 0) && (len > wrap)) { \
        printk("\n%s", prefix); \
        len = sal_strlen(prefix); \
    } \
    printk("%s", str); \
    len += strlen(str)


/* 
 * Print a SOC internal register with fields broken out.
 */
void
sbx_cmic_reg_print(int unit, soc_regaddrinfo_t *ainfo, uint64 val, uint32 flags,
          char *fld_sep, int wrap)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, ainfo->reg);
    int             f;
    uint64          val64, resval, resfld;
    char            buf[80];
    char            line_buf[256];
    int             linelen = 0;
    int             nprint;

    if (flags & REG_PRINT_HEX) {
        if (SOC_REG_IS_64(unit, ainfo->reg)) {
            printk("%08x%08x\n",
                   COMPILER_64_HI(val),
                   COMPILER_64_LO(val));
        } else {
            printk("%08x\n",
                   COMPILER_64_LO(val));
        }
        return;
    }

    if (flags & REG_PRINT_CHG) {
        SOC_REG_RST_VAL_GET(unit, ainfo->reg, resval);
        if (COMPILER_64_EQ(val, resval)) {      /* no changed fields */
            return;
        }
    } else {
        COMPILER_64_ZERO(resval);
    }

    soc_reg_sprint_addr(unit, buf, ainfo);

    sal_sprintf(line_buf, "%s[0x%x]=", buf, ainfo->addr);
    PRINT_COUNT(line_buf, linelen, wrap, "   ");

    format_uint64(line_buf, val);
    PRINT_COUNT(line_buf, linelen, -1, "");

    if (flags & REG_PRINT_RAW) {
        printk("\n");
        return;
    }

    PRINT_COUNT(": <", linelen, wrap, "   ");

    nprint = 0;
    for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
        val64 = soc_reg64_field_get(unit, ainfo->reg, val, fld->field);
        if (flags & REG_PRINT_CHG) {
            resfld = soc_reg64_field_get(unit, ainfo->reg, resval, fld->field);
            if (COMPILER_64_EQ(val64, resfld)) {
                continue;
            }
        }

        if (nprint > 0) {
            sal_sprintf(line_buf, "%s", fld_sep);
            PRINT_COUNT(line_buf, linelen, -1, "");
        }
        sal_sprintf(line_buf, "%s=", SOC_FIELD_NAME(unit, fld->field));
        PRINT_COUNT(line_buf, linelen, wrap, "   ");
        format_uint64(line_buf, val64);
        PRINT_COUNT(line_buf, linelen, -1, "");
        nprint += 1;
    }

    printk(">\n");
}

/* 
 * Reads and displays all SOC registers specified by alist structure.
 */
int
sbx_cmic_reg_print_all(int unit, soc_regaddrlist_t *alist, uint32 flags)
{
    int             j;
    uint64          value;
    int             r, rv = 0;
    soc_regaddrinfo_t *ainfo;

    assert(alist);

    for (j = 0; j < alist->count; j++) {
        ainfo = &alist->ainfo[j];
        if ((r = soc_anyreg_read(unit, ainfo, &value)) < 0) {
            char            buf[80];
            soc_reg_sprint_addr(unit, buf, ainfo);
            printk("ERROR: read from register %s failed: %s\n",
                   buf, soc_errmsg(r));
            rv = -1;
        } else {
            sbx_cmic_reg_print(unit, ainfo, value, flags, ",", 62);
        }
    }

    return rv;
}

/*
 * Register Types - for getreg and dump commands
 */

static regtype_entry_t regtypes[] = {
 { "PCIC",      soc_pci_cfg_reg,"PCI Configuration space" },
 { "PCIM",      soc_cpureg,     "PCI Memory space (CMIC)" },
 { "SOC",       soc_schan_reg,  "SOC internal registers" },
 { "SCHAN",     soc_schan_reg,  "SOC internal registers" },
 { "PHY",       soc_phy_reg,    "PHY registers via MII (phyID<<8|phyADDR)" },
 { "MW",        soc_hostmem_w,  "Host Memory 32-bit" },
 { "MH",        soc_hostmem_h,  "Host Memory 16-bit" },
 { "MB",        soc_hostmem_b,  "Host Memory 8-bit" },
 { "MEM",       soc_hostmem_w,  "Host Memory 32-bit" }, /* Backward compat */
};

#define regtypes_count  COUNTOF(regtypes)

regtype_entry_t *sbx_cmic_regtype_lookup_name(char* str)
{
    int i;

    for (i = 0; i < regtypes_count; i++) {
        if (!sal_strcasecmp(str,regtypes[i].name)) {
            return &regtypes[i];
        }
    }

    return 0;
}

void sirius_regtype_print_all(void)
{
    int i;

    printk("Register types supported by setreg, getreg, and dump:\n");

    for (i = 0; i < regtypes_count; i++)
        printk("\t%-10s -%s\n", regtypes[i].name, regtypes[i].help);
}

/* 
 * Get a register by type.
 *
 * doprint:  Boolean.  If set, display data.
 */
int
sbx_cmic_reg_get_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 *outval, uint32 flags)
{
    int             rv = CMD_OK;
    int             r;
    uint16          phy_rd_data;
    pci_dev_t      *pci_dev = NULL;
    soc_regaddrinfo_t ainfo;
    int             is64 = FALSE;

    switch (regtype) {
    case soc_pci_cfg_reg:
        if (regaddr & 3) {
            printk("ERROR: PCI config addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
        }
        break;

    case soc_cpureg:
        if (regaddr & 3) {
            printk("ERROR: PCI memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));
        }
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_get(unit, &ainfo, regaddr);

        if (ainfo.reg >= 0) {
            is64 = SOC_REG_IS_64(unit, ainfo.reg);
        }

        r = soc_anyreg_read(unit, &ainfo, outval);
        if (r < 0) {
            printk("ERROR: soc_reg32_read failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }

        break;

    case soc_hostmem_w:
        COMPILER_64_SET(*outval, 0, pci_dma_getw(pci_dev, regaddr));
        break;
    case soc_hostmem_h:
        COMPILER_64_SET(*outval, 0, pci_dma_geth(pci_dev, regaddr));
        break;
    case soc_hostmem_b:
        COMPILER_64_SET(*outval, 0, pci_dma_getb(pci_dev, regaddr));
        break;

    case soc_phy_reg:
        /* Leave for MII debug reads */
        if ((r = soc_miim_read(unit,
                               (uint8) (regaddr >> 8 & 0xff),   /* Phy ID */
                               (uint8) (regaddr & 0xff),        /* Phy addr */
                               &phy_rd_data)) < 0) {
            printk("ERROR: soc_miim_read failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, (uint32) phy_rd_data);
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }

    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
        if (flags & REG_PRINT_HEX) {
            if (is64) {
                printk("%08x%08x\n",
                       COMPILER_64_HI(*outval),
                       COMPILER_64_LO(*outval));
            } else {
                printk("%08x\n",
                       COMPILER_64_LO(*outval));
            }
        } else {
            char buf[80];

            format_uint64(buf, *outval);

            printk("%s[0x%x] = %s\n",
                   soc_regtypenames[regtype], regaddr, buf);
        }
    }

    return rv;
}


/* 
 * Get a register by type.
 *
 * doprint:  Boolean.  If set, display data.
 */
int
sbx_cmic_reg_get_extended_by_type(int unit, int port, int block, uint32 regaddr, 
				  soc_regtype_t regtype,
				  uint64 *outval, uint32 flags)
{
    int             rv = CMD_OK;
    int             r;
    uint16          phy_rd_data;
    pci_dev_t      *pci_dev = NULL;
    soc_regaddrinfo_t ainfo;
    int		    is64 = FALSE;

    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        return sbx_cmic_reg_get_by_type(unit, regaddr, regtype, outval, flags);
    }

    switch (regtype) {
    case soc_pci_cfg_reg:
	if (regaddr & 3) {
	    printk("ERROR: PCI config addr must be multiple of 4\n");
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
	}
	break;

    case soc_cpureg:
	if (regaddr & 3) {
	    printk("ERROR: PCI memory addr must be multiple of 4\n");
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));
	}
	break;
#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        if (regaddr & 3) {
            printk("ERROR: MCS memory addr must be multiple of 4\n");
            rv = CMD_FAIL;
        } else {
            COMPILER_64_SET(*outval, 0, soc_pci_mcs_read(unit, regaddr));
        }
        break;
#endif
    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
	soc_regaddrinfo_extended_get(unit, &ainfo, 
                                     SOC_BLOCK_INFO(unit, block).cmic, 
                                     regaddr);

        if (ainfo.reg >= 0) {
	    is64 = SOC_REG_IS_64(unit, ainfo.reg);
	}
        /* Set port value to handle TR3 AXP regs */
        if ((port >= 0) && (ainfo.port < 0)) {
            ainfo.port = port;
        }
	r = soc_anyreg_read(unit, &ainfo, outval);
	if (r < 0) {
	    printk("ERROR: soc_reg32_read failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}

	break;

    case soc_hostmem_w:
	COMPILER_64_SET(*outval, 0, pci_dma_getw(pci_dev, regaddr));
	break;
    case soc_hostmem_h:
	COMPILER_64_SET(*outval, 0, pci_dma_geth(pci_dev, regaddr));
	break;
    case soc_hostmem_b:
	COMPILER_64_SET(*outval, 0, pci_dma_getb(pci_dev, regaddr));
	break;

    case soc_phy_reg:
	/* Leave for MII debug reads */
	if ((r = soc_miim_read(unit,
			       (uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
			       (uint8) (regaddr & 0xff),	/* Phy addr */
			       &phy_rd_data)) < 0) {
	    printk("ERROR: soc_miim_read failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	} else {
	    COMPILER_64_SET(*outval, 0, (uint32) phy_rd_data);
	}
	break;

    default:
	assert(0);
	rv = CMD_FAIL;
	break;
    }

    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
	if (flags & REG_PRINT_HEX) {
	    if (is64) {
		printk("%08x%08x\n",
		       COMPILER_64_HI(*outval),
		       COMPILER_64_LO(*outval));
	    } else {
		printk("%08x\n",
		       COMPILER_64_LO(*outval));
	    }
	} else {
	    char buf[80];

	    format_uint64(buf, *outval);

	    printk("%s[0x%x] = %s\n",
		   soc_regtypenames[regtype], regaddr, buf);
	}
    }

    return rv;
}

/* 
 * Set a register by type.  For SOC registers, is64 is used to
 * indicate if this is a 64 bit register.  Otherwise, is64 is
 * ignored.
 *
 */
int
sbx_cmic_reg_set_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 regval)
{
    int             rv = CMD_OK, r;
    uint32          val32;
    pci_dev_t      *pci_dev = NULL;
    soc_regaddrinfo_t ainfo;

    COMPILER_64_TO_32_LO(val32, regval);

    switch (regtype) {
    case soc_pci_cfg_reg:
        bde->pci_conf_write(unit, regaddr, val32);
        break;

    case soc_cpureg:
        soc_pci_write(unit, regaddr, val32);
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
        soc_regaddrinfo_get(unit, &ainfo, regaddr);

        r = soc_anyreg_write(unit, &ainfo, regval);
        if (r < 0) {
            printk("ERROR: write reg failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }

        break;

    case soc_hostmem_w:
        pci_dma_putw(pci_dev, regaddr, val32);
        break;

    case soc_hostmem_h:
        pci_dma_puth(pci_dev, regaddr, val32);
        break;

    case soc_hostmem_b:
        pci_dma_putb(pci_dev, regaddr, val32);
        break;

    case soc_phy_reg:
        /* Leave for MII debug writes */
        if ((r = soc_miim_write(unit,
                                (uint8) (regaddr >> 8 & 0xff),  /* Phy ID */
                                (uint8) (regaddr & 0xff),       /* Phy addr */
                                (uint16) val32)) < 0) {
            printk("ERROR: write miim failed: %s\n", soc_errmsg(r));
            rv = CMD_FAIL;
        }
        break;

    default:
        assert(0);
        rv = CMD_FAIL;
        break;
    }

    return rv;
}


/* 
 * Set a register by type.  For SOC registers, is64 is used to
 * indicate if this is a 64 bit register.  Otherwise, is64 is
 * ignored.
 *
 */
int
sbx_cmic_reg_set_extended_by_type(int unit, int port, int block, uint32 regaddr, 
				  soc_regtype_t regtype, uint64 regval)
{
    int             rv = CMD_OK, r;
    uint32          val32;
    pci_dev_t      *pci_dev = NULL;
    soc_regaddrinfo_t ainfo;
    
    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        return sbx_cmic_reg_set_by_type(unit, regaddr, regtype, regval);
    }
    COMPILER_64_TO_32_LO(val32, regval);

    switch (regtype) {
    case soc_pci_cfg_reg:
	bde->pci_conf_write(unit, regaddr, val32);
	break;

    case soc_cpureg:
	soc_pci_write(unit, regaddr, val32);
	break;

#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        soc_pci_mcs_write(unit, regaddr, val32);
        break;
#endif

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:
	soc_regaddrinfo_extended_get(unit, &ainfo, 
	                             SOC_BLOCK_INFO(unit, block).cmic, 
                                     regaddr);
        ainfo.port = port; /* FIXME: Check if this can be avoided */
	r = soc_anyreg_write(unit, &ainfo, regval);
	if (r < 0) {
	    printk("ERROR: write reg failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}

	break;

    case soc_hostmem_w:
	pci_dma_putw(pci_dev, regaddr, val32);
	break;

    case soc_hostmem_h:
	pci_dma_puth(pci_dev, regaddr, val32);
	break;

    case soc_hostmem_b:
	pci_dma_putb(pci_dev, regaddr, val32);
	break;

    case soc_phy_reg:
	/* Leave for MII debug writes */
	if ((r = soc_miim_write(unit,
				(uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
				(uint8) (regaddr & 0xff),	/* Phy addr */
				(uint16) val32)) < 0) {
	    printk("ERROR: write miim failed: %s\n", soc_errmsg(r));
	    rv = CMD_FAIL;
	}
	break;

    default:
	assert(0);
	rv = CMD_FAIL;
	break;
    }

    return rv;
}

/* 
 * Gets a memory value or register from the SOC.
 * Syntax: getreg [<regtype>] <offset|symbol>
 */

cmd_result_t
cmd_sbx_cmic_reg_get(int unit, args_t *a)
{
    uint64          regval;
    uint32          regaddr = 0;
    int             rv = CMD_OK;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist;
    char           *name;
    uint32          flags = REG_PRINT_DO_PRINT;

    if (0 == sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    /* 
     * If first arg is a register type, take it and use the next argument
     * as the name or address, otherwise default to register type "soc."
     */
    name = ARG_GET(a);

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "chg")) {
            flags |= REG_PRINT_CHG;
            name = ARG_GET(a);
        } else {
            break;
        }
    }

    if (name == NULL) {
        return CMD_USAGE;
    }

    if ((rt = sbx_cmic_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_cmic_regtype_lookup_name("schan");
    }
    if (0 == rt) {
        printk("Unknown register.\n");
        return (CMD_FAIL);
    }

    if (soc_regaddrlist_alloc(&alist) < 0) {
        printk("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (isint(name)) {                 /* Numerical address given */
        regaddr = parse_integer(name);
        rv = sbx_cmic_reg_get_by_type(unit, regaddr, rt->type, &regval, flags);
    } else {
        if (*name == '$') {
            name++;
        }

        /* Symbolic name given, print all or some values ... */
        if (rt->type == soc_cpureg) {
            if (parse_cmic_regname(unit, name, &regaddr) < 0) {
                printk("ERROR: bad argument to GETREG PCIM: %s\n", name);
                rv = CMD_FAIL;
            } else {
                rv = sbx_cmic_reg_get_by_type(unit, regaddr, rt->type,
                                      &regval, flags);
            }
        } else if (parse_symbolic_reference(unit, &alist, name) < 0) {
            printk("Syntax error parsing \"%s\"\n", name);
            rv = CMD_FAIL;
        } else if (sbx_cmic_reg_print_all(unit, &alist, flags) < 0) {
            rv = CMD_FAIL;
        }
    }
    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Auxilliary routine to handle setreg and modreg.
 *      mod should be 0 for setreg, which takes either a value or a
 *              list of <field>=<value>, and in the latter case, sets all
 *              non-specified fields to zero.
 *      mod should be 1 for modreg, which does a read-modify-write of
 *              the register and permits value to be specified by a list
 *              of <field>=<value>[,...] only.
 */

STATIC cmd_result_t
sbx_cmic_do_reg_set(int unit, args_t *a, int mod)
{
    uint64          regval;
    uint32          regaddr = 0;
    int             rv = CMD_OK, i;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist = {0, NULL};
    soc_regaddrinfo_t *ainfo;
    char           *name;
    char           *s, *valstr = NULL;

    COMPILER_64_ALLONES(regval);

    if (0 == sh_check_attached(ARG_CMD(a), unit)) {
        return  CMD_FAIL;
    }

    if ((name = ARG_GET(a)) == 0) {
        return  CMD_USAGE;
    }

    /* 
     * If first arg is an access type, take it and use the next argument
     * as the name, otherwise use default access type.
     * modreg command does not allow this and assumes soc.
     */

    if ((0 == mod) && (rt = sbx_cmic_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = sbx_cmic_regtype_lookup_name("schan");
        if (0 == rt) {
            return CMD_FAIL;
        }
    }

    /* 
     * Parse the value field.  If there are more than one, string them
     * together with commas separating them (to make field-based setreg
     * inputs more convenient).
     */

    if ((s = ARG_GET(a)) == 0) {
        printk("Syntax error: missing value\n");
        return  CMD_USAGE;
    }

    if ((valstr = sal_alloc(ARGS_BUFFER, "reg_set")) == NULL) {
        printk("sbx_cmic_do_reg_set: Out of memory\n");
        return CMD_FAIL;
    }

    collect_comma_args(a, valstr, s);

    if (mod && isint(valstr)) {
        sal_free(valstr);
        return CMD_USAGE;
    }


    if (soc_regaddrlist_alloc(&alist) < 0) {
        printk("Could not allocate address list.  Memory error.\n");
        sal_free(valstr);
        return CMD_FAIL;
    }

    if (!mod && isint(name)) {
        /* Numerical address given */
        regaddr = parse_integer(name);
        regval = parse_uint64(valstr);
        rv = sbx_cmic_reg_set_by_type(unit, regaddr, rt->type, regval);
    } else {                           /* Symbolic name given, set all or some 
                                * values ... */
        if (*name == '$') {
            name++;
        }
        if (rt->type == soc_cpureg) {
            if (parse_cmic_regname(unit, name, &regaddr) < 0) {
                printk("ERROR: bad argument to SETREG PCIM: %s\n", name);
                sal_free(valstr);
                soc_regaddrlist_free(&alist);
                return CMD_FAIL;
            }
            regval = parse_uint64(valstr);
            rv = sbx_cmic_reg_set_by_type(unit, regaddr, rt->type, regval);
        } else if (parse_symbolic_reference(unit, &alist, name) < 0) {
            printk("Syntax error parsing \"%s\"\n", name);
            sal_free(valstr);
            soc_regaddrlist_free(&alist);
            return CMD_FAIL;
        } else {
            if (isint(valstr)) {       /* valstr is numeric */
                regval = parse_uint64(valstr);
            }

            for (i = 0; i < alist.count; i++) {
                ainfo = &alist.ainfo[i];

                /* alist now holds list of registers to change */
                if (!isint(valstr)) {  /* Must modify registers */
                    /* 
                     * valstr must be a field replacement spec.
                     * In modreg mode, read the current register value,
                     * and modify it.  In setreg mode,
                     * assume a starting value of zero and modify it.
                     */
                    if (mod) {
                        rv = sbx_cmic_reg_get_extended_by_type(unit, ainfo->port, 
							       ainfo->block, 
							       ainfo->addr,
							       SOC_REG_INFO(unit, ainfo->reg).regtype,
							       &regval, 0);
                    } else {
                        COMPILER_64_ZERO(regval);
                    }

                    if (rv == CMD_OK) {
                        if ((rv = modify_reg_fields(unit, ainfo->reg, &regval,
                                                    (uint64 *) 0,
                                                    valstr)) < 0) {
                            printk("Syntax error, aborted\n");
                        }
                    }
                }

                if (rv == CMD_OK) {
                    rv = sbx_cmic_reg_set_extended_by_type(unit, ainfo->port, ainfo->block, ainfo->addr,
                                          SOC_REG_INFO(unit, ainfo->reg).regtype, regval);
                }
                if (rv != CMD_OK) {
                    break;
                }
            }
        }
    }
    sal_free(valstr);
    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Sets a memory value or register on the SOC.
 * Syntax 1: setreg [<regtype>] <offset|symbol> <value>
 * Syntax 2: setreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_sbx_cmic_reg_set(int unit, args_t *a)
{
    return sbx_cmic_do_reg_set(unit, a, 0);
}

/* 
 * Read/modify/write a memory value or register on the SOC.
 * Syntax: modreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_sbx_cmic_reg_mod(int unit, args_t * a)
{
    return sbx_cmic_do_reg_set(unit, a, 1);
}

char cmd_sirius_regcmp_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "Usage: regcmp [args...]\n"
#else
    "Parameters: [-q] [<LOOPS>] <REG> [<VALUE>] [==|!=] <VALUE>\n\t"
    "If the optional <VALUE> on the left is given, starts by writing\n\t"
    "<VALUE> to <REG>.   Then loops up to <LOOPS> times reading <REG>,\n\t"
    "comparing if it is equal (==) or not equal (!=) to the <VALUE> on\n\t"
    "the right, and stopping if the compare fails.  If -q is specified, no\n\t"
    "output is displayed.  <LOOPS> defaults to 1 and may be * for\n\t"
    "indefinite.  If the compare fails, the command result is 1.  If the\n\t"
    "loops expire (compares succeed), the result is 0.  The result may be\n\t"
    "tested in an IF statement.  Also, each <VALUE> can consist of\n\t"
    "<FIELD>=<VALUE>[,...] to compare individual fields.  Examples:\n\t"
    "    if \"regcmp -q 1 rpkt.fe0 == 0\" \"echo RPKT IS ZERO\"\n\t"
    "    if \"regcmp -q config.e0 == fil_en=1\" \"echo FILTERING ENABLED\"\n"
#endif
    ;

cmd_result_t
cmd_sbx_cmic_reg_cmp(int unit, args_t *a)
{
    soc_reg_t       reg;
    soc_regaddrlist_t alist;
    soc_regaddrinfo_t *ainfo;
    char           *name = NULL, *count_str;
    char           *read_str, *write_str, *op_str;
    uint64          read_val, read_mask, write_val, reg_val, tmp_val;
    int             equal, i, quiet, loop;
    int             are_equal;
    int rv = CMD_OK;

    if (!(count_str = ARG_GET(a))) {
        return (CMD_USAGE);
    }

    if (!strcmp(count_str, "-q")) {
        quiet = TRUE;
        if (!(count_str = ARG_GET(a))) {
            return (CMD_USAGE);
        }
    } else {
        quiet = FALSE;
    }

    if (!strcmp(count_str, "*")) {
        loop = -1;
    } else if (isint(count_str)) {
        if ((loop = parse_integer(count_str)) < 0) {
            printk("%s: Invalid loop count: %s\n", ARG_CMD(a), count_str);
            return (CMD_FAIL);
        }
    } else {
        name = count_str;
        loop = 1;
    }

    if (!name && !(name = ARG_GET(a))) {
        return (CMD_USAGE);
    }

    write_str = ARG_GET(a);
    op_str = ARG_GET(a);
    read_str = ARG_GET(a);

    /* Must have WRITE ==|!= READ or ==|!= READ */

    if (!read_str) {
        read_str = op_str;
        op_str = write_str;
        write_str = NULL;
    } else if (ARG_CNT(a)) {
        return (CMD_USAGE);
    }

    if (!read_str || !op_str) {
        return (CMD_USAGE);
    }

    if (!strcmp(op_str, "==")) {
        equal = TRUE;
    } else if (!strcmp(op_str, "!=")) {
        equal = FALSE;
    } else {
        return (CMD_USAGE);
    }

    if (*name == '$') {
        name++;
    }

    /* Symbolic name given, set all or some values ... */

    if (soc_regaddrlist_alloc(&alist) < 0) {
        printk("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (parse_symbolic_reference(unit, &alist, name) < 0) {
        printk("%s: Syntax error parsing \"%s\"\n", ARG_CMD(a), name);
        soc_regaddrlist_free(&alist);
        return (CMD_FAIL);
    }

    ainfo = &alist.ainfo[0];
    reg = ainfo->reg;

    COMPILER_64_ALLONES(read_mask);

    if (isint(read_str)) {
        read_val = parse_uint64(read_str);
    } else {
        COMPILER_64_ZERO(read_val);
        if (modify_reg_fields(unit, ainfo->reg, &read_val,
                              &read_mask, read_str) < 0) {
            printk("%s: Syntax error: %s\n", ARG_CMD(a), read_str);
            soc_regaddrlist_free(&alist);
            return (CMD_USAGE);
        }
    }

    if (write_str) {
        if (isint(write_str)) {
            write_val = parse_uint64(write_str);
        } else {
            COMPILER_64_ZERO(write_val);
            if (modify_reg_fields(unit, ainfo->reg, &write_val,
                                  (uint64 *) 0, write_str) < 0) {
                printk("%s: Syntax error: %s\n", ARG_CMD(a), write_str);
                soc_regaddrlist_free(&alist);
                return (CMD_USAGE);
            }
        }
    }

    do {
        for (i = 0; i < alist.count; i++) {
            int             r;

            ainfo = &alist.ainfo[i];
            if (write_str) {
                if ((r = soc_anyreg_write(unit, ainfo, write_val)) < 0) {
                    printk("%s: ERROR: Write register %s.%d failed: %s\n",
                           ARG_CMD(a), SOC_REG_NAME(unit, reg), i, soc_errmsg(r));
                    soc_regaddrlist_free(&alist);
                    return (CMD_FAIL);
                }
            }

            if ((r = soc_anyreg_read(unit, ainfo, &reg_val)) < 0) {
                printk("%s: ERROR: Read register %s.%d failed: %s\n",
                       ARG_CMD(a), SOC_REG_NAME(unit, reg), i, soc_errmsg(r));
                soc_regaddrlist_free(&alist);
                return (CMD_FAIL);
            }

            tmp_val = read_val;
            COMPILER_64_AND(tmp_val, read_mask);
            COMPILER_64_XOR(tmp_val, reg_val);
            are_equal = COMPILER_64_IS_ZERO(tmp_val);
            if ((!are_equal && equal) || (are_equal && !equal)) {
                if (!quiet) {
                    char buf1[80], buf2[80];
                    printk("%s: %s.%d ", ARG_CMD(a), SOC_REG_NAME(unit, reg), i);
                    format_uint64(buf1, read_val);
                    format_uint64(buf2, reg_val);
                    printk("%s %s %s\n", buf1, equal ? "!=" : "==", buf2);
                }
                soc_regaddrlist_free(&alist);
                return (1);
            }
        }

        if (loop > 0) {
            loop--;
        }
    } while (loop);

    soc_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Lists registers containing a specific pattern
 *
 * If use_reset is true, ignores val and uses reset default value.
 */

static void
do_reg_list(int unit, soc_regaddrinfo_t *ainfo, int use_reset, uint64 regval)
{
    soc_reg_t       reg = ainfo->reg;
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);
    soc_field_info_t *fld;
    int             f;
    uint32          flags;
    uint64          mask, fldval, rval, rmsk;
    char            buf[80];
    char            rval_str[20], rmsk_str[20], dval_str[20];
    int             i, copies, disabled;

    if (!SOC_REG_IS_VALID(unit, reg)) {
        printk("Register %s is not valid for chip %s\n",
               SOC_REG_NAME(unit, reg), SOC_UNIT_NAME(unit));
        return;
    }

    flags = reginfo->flags;

    COMPILER_64_ALLONES(mask);

    SOC_REG_RST_VAL_GET(unit, reg, rval);
    SOC_REG_RST_MSK_GET(unit, reg, rmsk);
    format_uint64(rval_str, rval);
    format_uint64(rmsk_str, rmsk);
    if (use_reset) {
        regval = rval;
        mask = rmsk;
    } else {
        format_uint64(dval_str, regval);
    }

    soc_reg_sprint_addr(unit, buf, ainfo);

    printk("Register: %s", buf);
#if !defined(SOC_NO_ALIAS)
    if (soc_reg_alias[reg] && *soc_reg_alias[reg]) {
        printk(" alias %s", soc_reg_alias[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    printk(" %s register", soc_regtypenames[reginfo->regtype]);
    printk(" address 0x%08x\n", ainfo->addr);

    printk("Flags:");
    if (flags & SOC_REG_FLAG_64_BITS) {
        printk(" 64-bits");
    } else {
        printk(" 32-bits");
    }
    if (flags & SOC_REG_FLAG_COUNTER) {
        printk(" counter");
    }
    if (flags & SOC_REG_FLAG_ARRAY) {
        printk(" array[%d-%d]", 0, reginfo->numels-1);
    }
    if (flags & SOC_REG_FLAG_NO_DGNL) {
        printk(" no-diagonals");
    }
    if (flags & SOC_REG_FLAG_RO) {
        printk(" read-only");
    }
    if (flags & SOC_REG_FLAG_WO) {
        printk(" write-only");
    }
    if (flags & SOC_REG_FLAG_ED_CNTR) {
        printk(" error/discard-counter");
    }
    if (flags & SOC_REG_FLAG_SPECIAL) {
        printk(" special");
    }
    if (flags & SOC_REG_FLAG_EMULATION) {
        printk(" emulation");
    }
    if (flags & SOC_REG_FLAG_VARIANT1) {
        printk(" variant1");
    }
    if (flags & SOC_REG_FLAG_VARIANT2) {
        printk(" variant2");
    }
    if (flags & SOC_REG_FLAG_VARIANT3) {
        printk(" variant3");
    }
    if (flags & SOC_REG_FLAG_VARIANT4) {
        printk(" variant4");
    }
    printk("\n");

    printk("Blocks:");
    copies = disabled = 0;
    for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
        /*if (SOC_BLOCK_INFO(unit, i).type & reginfo->block) {*/
        if (SOC_BLOCK_IS_TYPE(unit, i, reginfo->block)) {
            if (SOC_INFO(unit).block_valid[i]) {
                printk(" %s", SOC_BLOCK_NAME(unit, i));
            } else {
                printk(" [%s]", SOC_BLOCK_NAME(unit, i));
                disabled += 1;
            }
            copies += 1;
        }
    }
    printk(" (%d cop%s", copies, copies == 1 ? "y" : "ies");
    if (disabled) {
        printk(", %d disabled", disabled);
    }
    printk(")\n");

#if !defined(SOC_NO_DESC)
    if (soc_reg_desc[reg] && *soc_reg_desc[reg]) {
        printk("Description: %s\n", soc_reg_desc[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    printk("Displaying:");
    if (use_reset) {
        printk(" reset defaults");
    } else {
        printk(" value %s", dval_str);
    }
    printk(", reset value %s mask %s\n", rval_str, rmsk_str);

    for (f = reginfo->nFields - 1; f >= 0; f--) {
        fld = &reginfo->fields[f];
        printk("  %s<%d", SOC_FIELD_NAME(unit, fld->field),
               fld->bp + fld->len - 1);
        if (fld->len > 1) {
            printk(":%d", fld->bp);
        }
        fldval = soc_reg64_field_get(unit, reg, mask, fld->field);
        if (use_reset && COMPILER_64_IS_ZERO(fldval)) {
            printk("> = x");
        } else {
            fldval = soc_reg64_field_get(unit, reg, regval, fld->field);
            format_uint64(buf, fldval);
            printk("> = %s", buf);
        }
        if (fld->flags & (SOCF_RO|SOCF_WO)) {
            printk(" [");
            i = 0;
            if (fld->flags & SOCF_RO) {
                printk("%sRO", i++ ? "," : "");
            }
            if (fld->flags & SOCF_WO) {
                printk("%sWO", i++ ? "," : "");
            }
            printk("]");
        }
        printk("\n");
    }
}

#define PFLAG_ALIAS     0x01
#define PFLAG_SUMMARY   0x02

static void
_print_regname(int unit, soc_reg_t reg, int *col, int pflags)
{
    int             len;
    soc_reg_info_t *reginfo;

    reginfo = &SOC_REG_INFO(unit, reg);
    len = strlen(SOC_REG_NAME(unit, reg)) + 1;

    if (pflags & PFLAG_SUMMARY) {
        char    tname, *dstr1, *dstr2, *bname;
        int     dlen, copies, i;
        char    nstr[128], bstr[64];

        switch (reginfo->regtype) {
        case soc_schan_reg:     tname = 's'; break;
        case soc_cpureg:        tname = 'c'; break;
        case soc_genreg:        tname = 'g'; break;
        case soc_portreg:       tname = 'p'; break;
        case soc_cosreg:        tname = 'o'; break;
        case soc_hostmem_w:
        case soc_hostmem_h:
        case soc_hostmem_b:     tname = 'm'; break;
        case soc_phy_reg:       tname = 'f'; break;
        case soc_pci_cfg_reg:   tname = 'P'; break;
        default:                tname = '?'; break;
        }
#if !defined(SOC_NO_DESC)
        dstr2 = strchr(soc_reg_desc[reg], '\n');
        if (dstr2 == NULL) {
            dlen = strlen(soc_reg_desc[reg]);
        } else {
            dlen = dstr2 - soc_reg_desc[reg];
        }
        if (dlen > 30) {
            dlen = 30;
            dstr2 = "...";
        } else {
            dstr2 = "";
        }
        dstr1 = soc_reg_desc[reg];
#else /* defined(SOC_NO_DESC) */
        dlen = 1;
        dstr1 = "";
        dstr2 = "";
#endif /* defined(SOC_NO_DESC) */
        if (reginfo->flags & SOC_REG_FLAG_ARRAY) {
            sal_sprintf(nstr, "%s[%d]", SOC_REG_NAME(unit, reg), reginfo->numels);
        } else {
            sal_sprintf(nstr, "%s", SOC_REG_NAME(unit, reg));
        }

        copies = 0;
        bname = NULL;
        for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
            /*if (SOC_BLOCK_INFO(unit, i).type & reginfo->block) {*/
            if (SOC_BLOCK_IS_TYPE(unit, i, reginfo->block)) {
                if (bname == NULL) {
                    bname = SOC_BLOCK_NAME(unit, i);
                }
                copies += 1;
            }
        }
        if (copies > 1) {
            sal_sprintf(bstr, "%d/%s", copies, bname);
        } else if (copies == 1) {
            sal_sprintf(bstr, "%s", bname);
        } else {
            sal_sprintf(bstr, "none");
        }
        printk(" %c%c%c%c%c  %-26s %-8.8s  %*.*s%s\n",
               tname,
               (reginfo->flags & SOC_REG_FLAG_64_BITS) ? '6' : '3',
               (reginfo->flags & SOC_REG_FLAG_COUNTER) ? 'c' : '-',
               (reginfo->flags & SOC_REG_FLAG_ED_CNTR) ? 'e' : '-',
               (reginfo->flags & SOC_REG_FLAG_RO) ? 'r' :
               (reginfo->flags & SOC_REG_FLAG_WO) ? 'w' : '-',
               nstr,
               bstr,
               dlen, dlen, dstr1, dstr2);
        return;
    }
    if (*col < 0) {
        printk("  ");
        *col = 2;
    }
    if (*col + len > ((pflags & PFLAG_ALIAS) ? 65 : 72)) {
        printk("\n  ");
        *col = 2;
    }
    printk("%s%s ", SOC_REG_NAME(unit, reg), SOC_REG_ARRAY(unit, reg) ? "[]" : "");
#if !defined(SOC_NO_ALIAS)
    if ((pflags & PFLAG_ALIAS) && soc_reg_alias[reg]) {
        len += strlen(soc_reg_alias[reg]) + 8;
        printk("(aka %s) ", soc_reg_alias[reg]);
    }
#endif /* !defined(SOC_NO_ALIAS) */
    *col += len;
}

static void
_list_regs_by_type(int unit, soc_block_t blk, int *col, int pflag)
{
    soc_reg_t       reg;

    *col = -1;
    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        /*if (SOC_REG_INFO(unit, reg).block & blk) {*/
        if (SOC_BLOCK_IN_LIST(SOC_REG_INFO(unit, reg).block, blk)) {
            _print_regname(unit, reg, col, pflag);
        }
    }
    printk("\n");
}

cmd_result_t
cmd_sbx_cmic_reg_list(int unit, args_t *a)
{
    char           *str;
    char           *val;
    uint64          value;
    soc_regaddrinfo_t ainfo;
    int             found;
    int             rv = CMD_OK;
    int             all_regs;
    soc_reg_t       reg;
    int             col;
    int             pflag;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    ainfo.reg = INVALIDr;
    pflag = 0;
    col = -1;

    /* Parse options */
    while (((str = ARG_GET(a)) != NULL) && (str[0] == '-')) {
        while (str[0] && str[0] == '-') {
            str += 1;
        }
        if (sal_strcasecmp(str, "alias") == 0 ||
            sal_strcasecmp(str, "a") == 0) {    /* list w/ alias */
            pflag |= PFLAG_ALIAS;
            continue;
        }
        if (sal_strcasecmp(str, "summary") == 0 ||
            sal_strcasecmp(str, "s") == 0) {    /* list w/ summary */
            pflag |= PFLAG_SUMMARY;
            continue;
        }
        if (sal_strcasecmp(str, "counters") == 0 ||
            sal_strcasecmp(str, "c") == 0) {    /* list counters */
            printk("unit %d counters\n", unit);
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg))
                    continue;
                if (!SOC_REG_IS_COUNTER(unit, reg))
                    continue;
                _print_regname(unit, reg, &col, pflag);
            }
            printk("\n\n");
            return CMD_OK;
        }
        if (sal_strcasecmp(str, "ed") == 0 ||
            sal_strcasecmp(str, "e") == 0) {    /* error/discard */
            printk("unit %d error/discard counters\n", unit);
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg)) {
                    continue;
                }
                if (!(SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_ED_CNTR)) {
                    continue;
                }
                _print_regname(unit, reg, &col, pflag);
            }
            printk("\n\n");
            return CMD_OK;
        }
        if (sal_strcasecmp(str, "type") == 0 ||
            sal_strcasecmp(str, "t") == 0) {    /* list by type */
            int         i;
            soc_info_t *si = &SOC_INFO(unit);

            for (i = 0; i < COUNTOF(si->has_block); i++) {
                if (!(si->has_block[i])) {
                    continue;
                }
                printk("unit %d %s registers\n",
                       unit,
                       soc_block_name_lookup_ext(si->has_block[i], unit));
                col = -1;
                _list_regs_by_type(unit, si->has_block[i], &col, pflag);
            }
            printk("\n");
            return CMD_OK;
        }
        printk("ERROR: unrecognized option: %s\n", str);
        return CMD_FAIL;
    }

    if (!str) {
        return CMD_USAGE;
    }

    if ((val = ARG_GET(a)) != NULL) {
        value = parse_uint64(val);
    } else {
        COMPILER_64_ZERO(value);
    }


    if (isint(str)) {
        /* 
         * Address given, look up SOC register.
         */
        char            buf[80];
        uint32          addr;
        addr = parse_integer(str);
        soc_regaddrinfo_get(unit, &ainfo, addr);
        if (!ainfo.valid || (int)ainfo.reg < 0) {
            printk("Unknown register address: 0x%x\n", addr);
            rv = CMD_FAIL;
        } else {
            soc_reg_sprint_addr(unit, buf, &ainfo);
            printk("Address %s\n", buf);
        }
    } else {
        soc_regaddrlist_t alist;

        if (soc_regaddrlist_alloc(&alist) < 0) {
            printk("Could not allocate address list.  Memory error.\n");
            return CMD_FAIL;
        }

        /* 
         * Symbolic name.
         * First check if the register is there as exact match.
         * If not, list all substring matches.
         */

        all_regs = 0;
        if (*str == '$') {
            str++;
        } else if (*str == '*') {
            str++;
            all_regs = 1;
        }

        if (all_regs || parse_symbolic_reference(unit, &alist, str) < 0) {
            found = 0;
            for (reg = 0; reg < NUM_SOC_REG; reg++) {
                if (!SOC_REG_IS_VALID(unit, reg)) {
                    continue;
                }
                if (strcaseindex(SOC_REG_NAME(unit, reg), str) != 0) {
                    if (!found && !all_regs) {
                        printk("Unknown register; possible matches are:\n");
                    }
                    _print_regname(unit, reg, &col, pflag);
                    found = 1;
                }
            }
            if (!found) {
                printk("No matching register found");
            }
            printk("\n");
            rv = CMD_FAIL;
        } else {
            ainfo = alist.ainfo[0];
        }

        soc_regaddrlist_free(&alist);
    }

    /* 
     * Now have ainfo -- if reg is no longer INVALIDr
     */

    if (ainfo.reg != INVALIDr) {
        if (val) {
            do_reg_list(unit, &ainfo, 0, value);
        } else {
            COMPILER_64_ZERO(value);
            do_reg_list(unit, &ainfo, 1, value);
        }
    }

    return rv;
}

/* 
 * Editreg allows modifying register fields.
 * Works on fully qualified SOC registers only.
 */

cmd_result_t
cmd_sbx_cmic_reg_edit(int unit, args_t *a)
{
    soc_reg_info_t *reginfo;
    soc_field_info_t *fld;
    soc_regaddrlist_t alist;
    soc_reg_t       reg;
    uint64          v64;
    uint32          val, dfl, fv;
    char            ans[64], dfl_str[64];
    char           *name = ARG_GET(a);
    int             r, rv = CMD_FAIL;
    int             i, f;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return rv;
    }

    if (!name) {
        return CMD_USAGE;
    }

    if (*name == '$') {
        name++;
    }

    if (soc_regaddrlist_alloc(&alist) < 0) {
        printk("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (parse_symbolic_reference(unit, &alist, name) < 0) {
        printk("Syntax error parsing \"%s\"\n", name);
        soc_regaddrlist_free(&alist);
        return (rv);
    }

    reg = alist.ainfo[0].reg;
    reginfo = &SOC_REG_INFO(unit, reg);

    /* 
     * If more than one register was specified, read the first one
     * and write the edited value to all of them.
     */

    if (soc_anyreg_read(unit, &alist.ainfo[0], &v64) < 0) {
        printk("ERROR: read reg failed\n");
        soc_regaddrlist_free(&alist);
        return (rv);
    }

    COMPILER_64_TO_32_LO(val, v64);

    printk("Current value: 0x%x\n", val);

    for (f = 0; f < (int)reginfo->nFields; f++) {
        fld = &reginfo->fields[f];
        dfl = soc_reg_field_get(unit, reg, val, fld->field);
        sal_sprintf(dfl_str, "0x%x", dfl);
        sal_sprintf(ans,                       /* Also use ans[] for prompt */
                "  %s<%d", SOC_FIELD_NAME(unit, fld->field), fld->bp + fld->len - 1);
        if (fld->len > 1) {
            sal_sprintf(ans + strlen(ans), ":%d", fld->bp);
        }
        strcat(ans, ">? ");
        if (sal_readline(ans, ans, sizeof(ans), dfl_str) == 0 || ans[0] == 0) {
            printk("Aborted\n");
        soc_regaddrlist_free(&alist);
        return (rv);
        }
        fv = parse_integer(ans);
        if (fv & ~((1 << (fld->len - 1) << 1) - 1)) {
            printk("Value too big for %d-bit field, try again.\n",
                   fld->len);
            f--;
        } else {
            soc_reg_field_set(unit, reg, &val, fld->field, fv);
        }
    }

    printk("Writing new value: 0x%x\n", val);

    for (i = 0; i < alist.count; i++) {
        COMPILER_64_SET(v64, 0, val);

        if ((r = soc_anyreg_write(unit, &alist.ainfo[i], v64)) < 0) {
            printk("ERROR: write reg 0x%x failed: %s\n",
                   alist.ainfo[i].addr, soc_errmsg(r));
        soc_regaddrlist_free(&alist);
        return (rv);
        }
    }

    rv = CMD_OK;

    soc_regaddrlist_free(&alist);
    return rv;
}


#define DUMP_PHY_COLS   4
#define DUMP_MW_COLS    4
#define DUMP_MH_COLS    8
#define DUMP_MB_COLS    16

/*
 * Dump all of the CMIC registers.
 */

static void
do_dump_pcim(int unit, uint32 off_start, uint32 count)
{
    uint32 off, val;

    if ((off_start & 3) != 0) {
        printk("dump_pcim ERROR: offset must be a multiple of 4\n");
        return;
    }

    for (off = off_start; count--; off += 4) {
        val = soc_pci_read(unit, off);
        printk("0x%04x %s: 0x%x\n", off, soc_pci_off2name(unit, off), val);
    }
}

/*
 * Dump all of the SOC register addresses, and if do_values is true,
 * read and dump their values along with the addresses.
 */

#define DREG_ADR_SEL_MASK     0xf       /* Low order 4 bits */
#define DREG_ADDR               0       /* Address only */
#define DREG_RVAL               1       /* Address and reset default value */
#define DREG_REGS               2       /* Address and real value */
#define DREG_DIFF               3       /* Addr & real value if != default */
#define DREG_CNTR               4       /* Address and real value if counter */

#define DREG_PORT_ALL -1
#define DREG_BLOCK_ALL -1

struct dreg_data {
    int unit;
    int dreg_select;
    int only_port;    /* Select which port/block.  -1 ==> all */
    int only_block;
};

static int
dreg(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct dreg_data *dd = data;
    uint32 value;
    uint64 val64, resetVal;
    char name[80];
    int is64, is_default, rv;
    int no_match = FALSE;  /* If specific port/block requested, turns true */
    char rval_str[20];

    /* Filter (set no_match) on ports and blocks if selected. */
    if (dd->only_port != DREG_PORT_ALL) {
        /* Only print ports that match */
        if (ainfo->port != dd->only_port) {
            no_match = TRUE;
        }
    }

    if (dd->only_block != DREG_BLOCK_ALL) {
        /* Only print blocks that match */
        if (ainfo->block != dd->only_block) {
            no_match = TRUE;
        } else { /* Match found; undo no_match */
            no_match = FALSE;
        }
    }

    if (no_match) {
        return 0;
    }

    soc_reg_sprint_addr(unit, name, ainfo);

    if (dd->dreg_select == DREG_ADDR) {
        printk("0x%08x %s\n", ainfo->addr, name);
        return 0;
    }

    SOC_REG_RST_VAL_GET(unit, ainfo->reg, resetVal);
    format_uint64(rval_str, resetVal);

    if (dd->dreg_select == DREG_RVAL) {
        printk("0x%08x %s = 0x%s\n", ainfo->addr, name, rval_str);
        return 0;
    }

    if (SOC_REG_INFO(unit, ainfo->reg).flags & SOC_REG_FLAG_WO) {
        printk("0x%08x %s = Write Only\n", ainfo->addr, name);
        return 0;
    }

    if (SOC_REG_IS_SPECIAL(unit, ainfo->reg)) {
        printk("0x%08x %s = Requires special processing\n",
               ainfo->addr, name);
        return 0;
    }

    if (reg_mask_subset(unit, ainfo, NULL)) {
        /* Register does not exist on this port/cos */
        return 0;
    }

    is64 = SOC_REG_IS_64(unit, ainfo->reg);

    if (is64) {
        rv = soc_reg64_read(dd->unit, ainfo->addr, &val64);
        is_default = COMPILER_64_EQ(val64, resetVal);
    } else {
        rv = soc_reg32_read(dd->unit, ainfo->addr, &value);
        is_default = (value == COMPILER_64_LO(resetVal));
    }

    if (rv < 0) {
        printk("0x%08x %s = ERROR\n", ainfo->addr, name);
        return 0;
    }

    if (dd->dreg_select == DREG_DIFF && is_default) {
        return 0;
    }

    if ((dd->dreg_select == DREG_CNTR)  &&
        (!(SOC_REG_INFO(unit, ainfo->reg).flags & SOC_REG_FLAG_COUNTER))) {
        return 0;
    }

    if (is64) {
        printk("0x%08x %s = 0x%08x%08x\n",
               ainfo->addr, name,
               COMPILER_64_HI(val64), COMPILER_64_LO(val64));
    } else {
        printk("0x%08x %s = 0x%08x\n", ainfo->addr, name, value);
    }

    return 0;
}

static cmd_result_t
do_dump_soc(int unit, int dreg_select, int only_port, int only_block)
{
    struct dreg_data dd;

    dd.unit = unit;
    dd.dreg_select = dreg_select;
    dd.only_port = only_port;
    dd.only_block = only_block;

    (void) soc_reg_iterate(unit, dreg, &dd);

    return CMD_OK;
}

/*
 * Dump registers, tables, or an address space.
 */

static cmd_result_t
sbx_cmic_do_dump_registers(int unit, regtype_entry_t *rt, args_t *a)
{
    int         i;
    uint32      t1, t2;
    pbmp_t      pbmp;
    soc_port_t  port, dport;
    int         rv = CMD_OK;
    pci_dev_t   *pci_dev = NULL;
    uint32      flags = DREG_REGS;
    int         dump_port = DREG_PORT_ALL;
    int         dump_block = DREG_BLOCK_ALL;
    char        *an_arg;
    char        *count;

    an_arg = ARG_GET(a);
    count = ARG_GET(a);

    /* PCI config space does not take an offset */
    switch (rt->type) {
    case soc_pci_cfg_reg:
        _pci_print_config(unit);
        break;
    case soc_cpureg:
        if (an_arg) {
            if (parse_cmic_regname(unit, an_arg, &t1) < 0) {
                printk("ERROR: unknown CMIC register name: %s\n", an_arg);
                rv = CMD_FAIL;
                goto done;
            }
            t2 = count ? parse_integer(count) : 1;
        } else {
            t1 = CMIC_OFFSET_MIN;
            t2 = (CMIC_OFFSET_MAX - CMIC_OFFSET_MIN) / 4 + 1;
        }
        do_dump_pcim(unit, t1, t2);
        break;

    case soc_schan_reg:
    case soc_genreg:
    case soc_portreg:
    case soc_cosreg:

        while (an_arg) {
            if (sal_strcasecmp(an_arg, "addr") == 0) {
                flags = DREG_ADDR;
            } else if (sal_strcasecmp(an_arg, "rval") == 0) {
                flags = DREG_RVAL;
            } else if (sal_strcasecmp(an_arg, "diff") == 0) {
                flags = DREG_DIFF;
            } else if (sal_strcasecmp(an_arg, "counter") == 0) {
                flags = DREG_CNTR;
            } else if (sal_strcasecmp(an_arg, "port") == 0) {
                an_arg = ARG_GET(a);
                dump_port = an_arg ? parse_integer(an_arg) : 0;
            } else if (sal_strcasecmp(an_arg, "block") == 0) {
                an_arg = ARG_GET(a);
                dump_block = an_arg ? parse_integer(an_arg) : 0;
            } else {
                printk("ERROR: unrecognized argument to DUMP SOC: %s\n",
                       an_arg);
                return CMD_FAIL;
            }
            if (count != NULL) {
                an_arg = count;
                count = NULL;
            } else {
                an_arg = ARG_GET(a);
            }
        }
        rv = do_dump_soc(unit, flags, dump_port, dump_block);
        break;

    case soc_phy_reg:
        if (an_arg) {
            if (parse_pbmp(unit, an_arg, &pbmp)) {
                printk("Error: Invalid port identifier: %s\n", an_arg);
                rv = CMD_FAIL;
                break;
            }
        } else {
            pbmp = PBMP_PORT_ALL(unit);
        }
        SOC_PBMP_AND(pbmp, PBMP_PORT_ALL(unit));
        DPORT_SOC_PBMP_ITER(unit, pbmp, dport, port) {
            uint8       phy_id = PORT_TO_PHY_ADDR(unit, port);
            uint16      phy_data, phy_reg;
            printk("\nPort %d (Phy ID %d)", port + 1, phy_id);
            for (phy_reg = PHY_MIN_REG; phy_reg <= PHY_MAX_REG; phy_reg++) {
                rv = soc_miim_read(unit, phy_id, phy_reg, &phy_data);
                if (rv < 0) {
                    printk("Error: Port %d: cmic_read_miim failed: %s\n",
                           port + 1, soc_errmsg(rv));
                    rv = CMD_FAIL;
                    goto done;
                }
                printk("%s\t0x%02x: 0x%04x",
                       ((phy_reg % DUMP_PHY_COLS) == 0) ? "\n" : "",
                       phy_reg, phy_data);
            }
            printk("\n");
        }
        break;

    case soc_hostmem_w:
        if (!an_arg) {
            printk("Dumping memory requires address and optional count\n");
            rv = CMD_FAIL;
            goto done;
        }
        t1 = parse_integer(an_arg) & ~3;
        t2 = count ? parse_integer(count) : 1;
        for (i = 0; i < (int)t2; i++, t1 += 4) {
            if ((i % DUMP_MW_COLS) == 0)
                printk("%08x: ", t1);
            printk("%08x%c",
                   pci_dma_getw(pci_dev, t1),
                   ((i + 1) % DUMP_MW_COLS) == 0 ? '\n' : ' ');
        }
        if (i % DUMP_MW_COLS)
            printk("\n");
        break;
    case soc_hostmem_h:
        if (!an_arg) {
            printk("Dumping memory requires address and optional count\n");
            rv = CMD_FAIL;
            goto done;
        }
        t1 = parse_integer(an_arg) & ~1;
        t2 = count ? parse_integer(count) : 1;
        for (i = 0; i < (int)t2; i++, t1 += 2) {
            if ((i % DUMP_MH_COLS) == 0)
                printk("%08x: ", t1);
            printk("%04x%c",
                   pci_dma_geth(pci_dev, t1),
                   ((i + 1) % DUMP_MH_COLS) == 0 ? '\n' : ' ');
        }
        if (i % DUMP_MH_COLS)
            printk("\n");
        break;
    case soc_hostmem_b:
        if (!an_arg) {
            printk("Dumping memory requires address and optional count\n");
            rv = CMD_FAIL;
            goto done;
        }
        t1 = parse_integer(an_arg);
        t2 = count ? parse_integer(count) : 1;
        for (i = 0; i < (int)t2; i++, t1 += 1) {
            if ((i % DUMP_MB_COLS) == 0)
                printk("%08x: ", t1);
            printk("%02x%c",
                   pci_dma_getb(pci_dev, t1),
                   ((i + 1) % DUMP_MB_COLS) == 0 ? '\n' : ' ');
        }
        if (i % DUMP_MB_COLS)
            printk("\n");
        break;
    default:
        printk("Dumping register type %s is not yet implemented.\n",
               rt->name);
        rv = CMD_FAIL;
        break;
    }

 done:
    return rv;
}

#define DUMP_TABLE_RAW          0x01
#define DUMP_TABLE_HEX          0x02
#define DUMP_TABLE_ALL          0x04
#define DUMP_TABLE_CHANGED      0x08

cmd_result_t
cmd_sbx_cmic_do_dump_table(int unit, soc_mem_t mem,
              int copyno, int index, int count, int flags)
{
    int                 k, i;
    uint32              entry[SOC_MAX_MEM_WORDS];
    char                lineprefix[256];
    int                 entry_dw;
    int                 rv = CMD_FAIL;
    uint8_t             bWredStateModFormat = FALSE;
    uint32              uRegVal = 0;
    int32               voq_threshold = 0;
    

    assert(copyno >= 0);

    entry_dw = soc_mem_entry_words(unit, mem);

    /* special case when dumping WRED_STATE in tme or hybrid mode sdk-29998 */
    if (sal_strcasecmp(SOC_MEM_NAME(unit, mem),"WRED_STATE") == 0 
	&& ((SOC_SBX_CFG(unit)->bTmeMode) || SOC_SBX_CFG(unit)->bHybridMode)) {
      bWredStateModFormat = TRUE;
    }

    for (k = index; k < index + count; k++) {
        {
            i = soc_mem_read(unit, mem, copyno, k, entry);
        }
        if (i < 0) {
            printk("Read ERROR: table %s.%s[%d]: %s\n",
                   SOC_MEM_UFNAME(unit, mem),
                   SOC_BLOCK_NAME(unit, copyno), k, soc_errmsg(i));
            goto done;
        }

        if (!(flags & DUMP_TABLE_ALL)) {
            int         validf;

            validf = -1;

            if (soc_mem_field_length(unit, mem, ACTIVEf) > 0) {
                validf = ACTIVEf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_BITf) > 0) {
                validf = VALID_BITf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALIDf) > 0) {
                validf = VALIDf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, L3_VALIDf) > 0) {
                validf = L3_VALIDf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, BUCKET_BITMAPf) > 0) {
                validf = BUCKET_BITMAPf;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID0f) > 0) {
                validf = VALID0f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    if (soc_mem_field_length(unit, mem, VALID1f) > 0) {
                        validf = VALID1f;
                        if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                            continue;
                        }
                    }
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_0f) > 0) {
                validf = VALID_0f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_1f) > 0) {
                validf = VALID_1f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_2f) > 0) {
                validf = VALID_2f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (soc_mem_field_length(unit, mem, VALID_3f) > 0) {
                validf = VALID_3f;
                if (soc_mem_field32_get(unit, mem, entry, validf) == 0) {
                    continue;
                }
            }

            if (mem == GFILTER_IMASKm || mem == FILTER_IMASKm) {
                uint32 mask_val[SOC_MAX_MEM_WORDS];
                soc_mem_field_get(unit, mem, entry, FMASKf, mask_val);
                if (sal_memcmp(mask_val,
                               soc_mem_entry_zeroes(unit, mem),
                               soc_mem_field_length(unit, mem,
                                                    FMASKf) / 8) == 0) {
                    continue;
                }
            }
        }

	if (bWredStateModFormat) {
	  /* the data was read from WRED_STATE, valid for q < 32k,
	   * but when printing use the wred_avg_queue_length format if 
	   * this is a local queue */
	  if (SOC_SBX_CFG(unit)->bHybridMode) {
	    SOC_IF_ERROR_RETURN(READ_QMA_CONFIG1r(unit, &uRegVal));
	    voq_threshold = soc_reg_field_get(unit, QMA_CONFIG1r, uRegVal, VOQ_THRESHOLDf);
	    if (k >= voq_threshold) {
	      mem = WRED_AVG_QUEUE_LENGTHm;
	    }
	  } else {
	    mem = WRED_AVG_QUEUE_LENGTHm;
	  }
	}

        if (flags & DUMP_TABLE_HEX) {
            for (i = 0; i < entry_dw; i++) {
                printk("%08x\n", entry[i]);
            }
        } else if (flags & DUMP_TABLE_CHANGED) {
            sal_sprintf(lineprefix, "%s.%s[%d]: ",
                    SOC_MEM_UFNAME(unit, mem),
                    SOC_BLOCK_NAME(unit, copyno),
                    k);
            soc_mem_entry_dump_if_changed(unit, mem, entry, lineprefix);
        } else {
            printk("%s.%s[%d]: ",
                   SOC_MEM_UFNAME(unit, mem),
                   SOC_BLOCK_NAME(unit, copyno),
                   k);

            if (flags & DUMP_TABLE_RAW) {
                for (i = 0; i < entry_dw; i++) {
                    printk("0x%08x ", entry[i]);
                }
            } else {
                soc_mem_entry_dump(unit, mem, entry);
            }

            printk("\n");
        }
    }

    rv = CMD_OK;

 done:
    return rv;
}

int
sbx_cmic_mem_dump_iter_callback(int unit, soc_mem_t mem, void *data)
{
    uint32     index_min, count, copyno;
    int        rv = SOC_E_NONE;
    int        flags = PTR_TO_INT(data);

    if (!SOC_MEM_IS_VALID(unit, mem) ||
        (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_DEBUG)) {
        return rv;
    }

    index_min = soc_mem_index_min(unit, mem);
    count = soc_mem_index_count(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
        /*
         * Bypass dumping MMU memories.
         */
        if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU) {
            continue;
        }

        if ((cmd_sbx_cmic_do_dump_table(unit, mem, copyno, index_min, 
                           count, flags)) != CMD_OK) {
            rv = SOC_E_INTERNAL;
            break;
        }
    }

    return rv;
}

static cmd_result_t
sbx_cmic_do_dump_memories(int unit, args_t *a)
{
    char        *an_arg;
    int          flags = DUMP_TABLE_ALL, rv = CMD_OK;

    an_arg = ARG_GET(a);

    while (an_arg) {
        if (sal_strcasecmp(an_arg, "diff") == 0) {
            flags = DUMP_TABLE_CHANGED;
        } else {
            printk("ERROR: unrecognized argument to DUMP SOC: %s\n",
                   an_arg);
            return CMD_FAIL;
        }
        an_arg = ARG_GET(a);
    }

    if ((soc_mem_iterate(unit, 
                         sbx_cmic_mem_dump_iter_callback, INT_TO_PTR(flags))) < 0) {
        rv = CMD_FAIL;
    }

    return rv;
}


cmd_result_t
cmd_sbx_cmic_dump(int unit, args_t *a)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    regtype_entry_t *rt;
    soc_mem_t mem;
    char *arg1, *arg2, *arg3;
    volatile int flags = 0;
    int copyno;
    volatile int rv = CMD_FAIL;
    parse_table_t pt;
    volatile char *fname = "";
    int append = FALSE;
#ifndef NO_SAL_APPL
    volatile int console_was_on = 0, console_disabled = 0, pushed_ctrl_c = 0;
    jmp_buf     ctrl_c;
#endif

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "File", PQ_STRING, 0, &fname, 0);
    parse_table_add(&pt, "Append", PQ_BOOL, 0, &append, FALSE);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        goto done;
    }

    if (parse_arg_eq(a, &pt) < 0) {
        rv = CMD_USAGE;
        goto done;
    }

#ifndef NO_SAL_APPL

    console_was_on = printk_cons_is_enabled();

    if (fname[0] != 0) {
        /*
         * Catch control-C in case if using file output option.
         */

#ifndef NO_CTRL_C
        if (setjmp(ctrl_c)) {
            rv = CMD_INTR;
            goto done;
        }
#endif

        sh_push_ctrl_c(&ctrl_c);

        pushed_ctrl_c = TRUE;

        if (printk_file_is_enabled()) {
            printk("%s: Can't dump to file while logging is enabled\n",
                   ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        if (printk_file_open((char *)fname, append) < 0) {
            printk("%s: Could not start log file\n", ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        printk_cons_enable(FALSE);

        console_disabled = 1;
    }

#endif /* NO_SAL_APPL */

    arg1 = ARG_GET(a);

    for (;;) {
        if (arg1 != NULL && !sal_strcasecmp(arg1, "raw")) {
            flags |= DUMP_TABLE_RAW;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "hex")) {
            flags |= DUMP_TABLE_HEX;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "all")) {
            flags |= DUMP_TABLE_ALL;
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, "chg")) {
            flags |= DUMP_TABLE_CHANGED;
            arg1 = ARG_GET(a);
        } else {
            break;
        }
    }

    if (arg1 == NULL) {
        rv = CMD_USAGE;
        goto done;
    }

    /* See if dumping internal ARL/L2 shadow copy */

    if (!sal_strcasecmp(arg1, "sarl") || !sal_strcasecmp(arg1, "sa")) {
        if (soc->arlShadow == NULL) {
            printk("No software ARL shadow table\n");
            rv = CMD_FAIL;
            goto done;
        }

        sal_mutex_take(soc->arlShadowMutex, sal_mutex_FOREVER);

        sal_mutex_give(soc->arlShadowMutex);

        rv = CMD_OK;
        goto done;
    }

    /* See if dumping a DV */
    if (!sal_strcasecmp(arg1, "dv")) {
        arg2 = ARG_GET(a);
        if (!arg2 || !isint(arg2)) {
            rv = CMD_USAGE;
            goto done;
        }
        soc_dma_dump_dv(unit, " ", (void *)parse_address(arg2));
        rv = CMD_OK;
        goto done;
    }

    /* See if dumping a packet */
    if (!sal_strcasecmp(arg1, "pkt")) {
        arg2 = ARG_GET(a);
        if (!arg2 || !isint(arg2)) {
            rv = CMD_USAGE;
            goto done;
        }
#ifdef  BROADCOM_DEBUG
        arg3 = ARG_GET(a);
        arg3 = arg3 ? arg3 : "0000";
        bcm_pkt_dump(unit,
                     INT_TO_PTR(parse_integer(arg2)),
                     parse_integer(arg3));
        rv = CMD_OK;
#else
        printk("cannot dump pkt in non-BROADCOM_DEBUG compilation\n");
        rv = CMD_FAIL;
#endif  /* BROADCOM_DEBUG */
        goto done;
    }

    /* See if dumping a memory table */

    if (parse_memory_name(unit, &mem, arg1, &copyno) >= 0) {
        int index, count;

        arg2 = ARG_GET(a);
        arg3 = ARG_GET(a);
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            debugk(DK_ERR, "Error: Memory %s not valid for chip %s.\n",
                   SOC_MEM_UFNAME(unit, mem), SOC_UNIT_NAME(unit));
            goto done;
        }
        if (copyno == COPYNO_ALL) {
            copyno = SOC_MEM_BLOCK_ANY(unit, mem);
        }
        if (arg2) {
            index = parse_memory_index(unit, mem, arg2);
            count = (arg3 ? parse_integer(arg3) : 1);
        } else {
            index = soc_mem_index_min(unit, mem);
            if (soc_mem_is_sorted(unit, mem) &&
                !(flags & DUMP_TABLE_ALL)) {
                count = soc_mem_entries(unit, mem, copyno);
            } else {
                count = soc_mem_index_max(unit, mem) - index + 1;
            }
        }
        rv = cmd_sbx_cmic_do_dump_table(unit, mem, copyno, index, count, flags);
        goto done;
    }

    if (!sal_strcasecmp(arg1, "socmem")) {
        rv = sbx_cmic_do_dump_memories(unit, a);
        goto done;
    }

    /*
     * See if dumping a register type
     */
    if ((rt = sbx_cmic_regtype_lookup_name(arg1)) != NULL) {
        rv = sbx_cmic_do_dump_registers(unit, rt, a);
        goto done;
    }

    printk("Unknown option or memory to dump "
           "(use 'help dump' for more info)\n");

    rv = CMD_FAIL;

 done:

#ifndef NO_SAL_APPL
    if (fname[0] != 0) {
        printk_file_close();
    }

    if (console_disabled && console_was_on) {
        printk_cons_enable(TRUE);
    }

    if (pushed_ctrl_c) {
        sh_pop_ctrl_c();
    }
#endif

    parse_arg_eq_done(&pt);
    return rv;
}


/*
 * Function:    cmd_sirius_print_info
 * Purpose:         List of Registers
 * Parameters:  unit - unit number of device
 *              str  - Search string to key in on such as "error" or "count"
 *              a    - command to be processed in args_t format
 * Returns:     CMD_FAIL, CMD_USAGE, CMD_OK
 */
cmd_result_t
cmd_sirius_print_info(int unit, char *str, int clrok,  args_t *a)
{
    int             clear = 0, ignore = 0;
    soc_regaddrinfo_t *ainfo;
    soc_regaddrlist_t alist;
    int             i = 0, rv = CMD_OK;
    char           *name = ARG_GET(a);
    soc_reg_t       reg;
    int             flags = DUMP_TABLE_CHANGED;
    uint64          val;

    COMPILER_64_ALLONES(val);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "clear")) {
            if (clrok) clear = 1;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "all")) {
            flags &= ~DUMP_TABLE_CHANGED;
            name = ARG_GET(a);
        } else {
            break;
        }
    }

    if ((name) && (*name == '~')) {
        name++;
        ignore = 1;
    }
   
    if (soc_regaddrlist_alloc(&alist) < 0) {
        printk("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }
    
    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        if (strcaseindex(SOC_REG_NAME(unit, reg), str) != 0) {
            if (parse_symbolic_reference(unit, &alist, SOC_REG_NAME(unit, reg)) < 0) {
                rv = CMD_FAIL;
            } else {
                /*
                 * Causes the hardware to hang
                 */
                if (strcaseindex(SOC_REG_NAME(unit, reg), "mmu_coslc_count_data") != 0) {
                    continue;
                }
                if (SAL_BOOT_BCMSIM && SOC_REG_IS_64(unit, reg)) {
                    continue;
                }
                if (sal_strcasecmp(str, "error") == 0) {
                    if ((strcaseindex(SOC_REG_NAME(unit, reg), "mask") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "force") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "error_status") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "error_inject") != 0)) {
                        continue;
                    }
                } else if (sal_strcasecmp(str, "cnt") == 0) {
                    if ((strcaseindex(SOC_REG_NAME(unit, reg), "maxsize") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cntr") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cntl") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt6") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt7") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "ecc_status") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "sfi_rx_sot") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "xp_xbode_cell_req") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "maxsz") != 0)) {
                        continue;
                    }
                    if (SAL_BOOT_BCMSIM && 
                        (strcaseindex(SOC_REG_NAME(unit, reg), "mac") != 0)) {
                        continue;
                    }
                } else if (sal_strcasecmp(str, "count") == 0) {
                    if ((strcaseindex(SOC_REG_NAME(unit, reg), "addr") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cmic") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "rb_debug_edc_line") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "ecc") != 0)) {
                        continue;
                    }
                }

                if (name != NULL) {
                    if (ignore) {
                        if (strcaseindex(SOC_REG_NAME(unit, reg), name) != 0) {
                            continue;
                        }
                    } else {
                        if (strcaseindex(SOC_REG_NAME(unit, reg), name) == 0) {
                            continue;
                        }
                    }
                }
                if (sbx_cmic_reg_print_all(unit, &alist, flags) < 0) {
                    rv = CMD_FAIL;
                }
                if (clear) {
                    if ((strcaseindex(SOC_REG_NAME(unit, reg), "cmic_ser_fail") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "eb_aging_dft") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "fr_flow_ctl_global") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "fr_flow_ctl_unicast") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "mac_pfc") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "mac_rxllfcmsg") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt6") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "qmb_debug_cnt7") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_pkt_count") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_slam_dma_entry") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_table_dma_entry") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "cmic_sw_pio_ack_data_beat") != 0) ||
                        (strcaseindex(SOC_REG_NAME(unit, reg), "txllfcmsg") != 0)) {
                        continue;
                    }

                    for (i = 0; i < alist.count; i++) {
                        ainfo = &alist.ainfo[i];
                        rv = sbx_cmic_reg_set_by_type(unit, ainfo->addr,
                                                        SOC_REG_INFO(unit, ainfo->reg).regtype, val);
                    }
                }
            }
        }
    }
    
    soc_regaddrlist_free(&alist);
    
    return rv;
}

#endif /* BCM_SIRIUS_SUPPORT */
