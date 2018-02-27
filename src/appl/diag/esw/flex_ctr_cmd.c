/*
 * $Id: flex_ctr_cmd.c 1.6.2.5 Broadcom SDK $
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
 * File:        flex_ctr_cmd.c
 * Flex Counter related commands
 *
 */


#include <appl/diag/system.h>
#include <appl/diag/dport.h>

#include <bcm/stat.h>
#include <bcm_int/esw/flex_ctr.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <sal/appl/io.h>
#ifdef BCM_KATANA_SUPPORT
extern int sscanf(const char *s, const char *format, ... ); 

#define sscanf donotuse_sscanf

int sscanf(const char *s, const char *format, ... )
{
    printk("Do not use sscanf\n");
    return 0;
}


cmd_result_t
cmd_esw_flex(int unit, args_t *a)
{
    char	     *c=NULL;
#if defined(BCM_KATANA_SUPPORT)
    cmd_result_t     cmd=CMD_NOTIMPL; 
    uint32           stat_counter_id=0;
    uint32           num_entries=0;
    uint32           object=0; 
    uint32           group=0; 
    uint32           port_number=0;
    uint32           direction=0;
    uint32           byte_flag=0;
    uint32           total_counters=0;
    uint32           count=0;
    uint32           hi_value=0; 
    uint32           lo_value=0; 
    uint32           *indexes=NULL;
    bcm_stat_value_t *values=NULL;
    bcm_port_stat_t  stat=0;
    bcm_gport_t	     gport=0;
    uint32	     vlan_index=0;
    bcm_error_t      rv=BCM_E_NONE;
#endif

    if (!(c = ARG_GET(a))) {		/* Nothing to do */
        return(CMD_USAGE);		/* Print usage line */
    }
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        /* Due to sscanf issues. Need to be corrected later */
        return (cmd);		
    } else {
       if (!sal_strcasecmp(c, "stat-enable-debug")) {
           /* bcm_esw_stat_flex_enable_debug(); */
           printk("Use debugmod command to enable flex counter messages\n");
           return CMD_OK;
       }
       if (!sal_strcasecmp(c, "stat-disable-debug")) {
           /* bcm_esw_stat_flex_disable_debug(); */
           printk("Use debugmod command to disable flex counter messages\n");
           return CMD_OK;
       }
       if (!sal_strcasecmp(c, "dump_all")) {
           bcm_esw_stat_group_dump_all(unit);
           return CMD_OK;
       }
       if (!sal_strcasecmp(c, "dump")) {
            c = ARG_GET(a);
	    if (c == NULL) {
	        printk("Required option(object) missing :%s \n",c);
                return CMD_USAGE;
	    }
            if (sscanf(c,"%u",&object) != 1) {
                 printk("object not ok :%s \n",c);
                 return CMD_FAIL;
            }
            printk("Object:%d \n",object); 
            c = ARG_GET(a);
            if (c == NULL) {
	         printk("Required option(group) missing :%s \n",c);
                 return CMD_USAGE;
	    }
            if (sscanf(c,"%u",&group) != 1) {
                printk("group not ok :%s \n",c);
                return CMD_FAIL;
            }
            bcm_esw_stat_group_dump(unit,object,group);
            return CMD_OK;
       }
       if (!sal_strcasecmp(c, "stat-group-create")) {
         printk("\n Group Creation BEGIN \n");
         c = ARG_GET(a);
	 if (c == NULL) {
	     printk("Required option(ingress or egress) missing :%s\n",c);
             return CMD_USAGE;
	 }
         if (!sal_strcasecmp(c, "ingress")) {
             printk("Ingress Mode \n");
             c = ARG_GET(a);
	     if (c == NULL) {
	         printk("Required option(object) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&object) != 1) {
                 printk("object not ok :%s \n",c);
                 return CMD_FAIL;
             }
         } else {
             printk("Egress Mode \n");
             c = ARG_GET(a);
	     if (c == NULL) {
	         printk("Required option(object) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&object) != 1) {
                 printk("object not ok :%s \n",c);
                 return CMD_FAIL;
             }
         }
         printk("Object:%d \n",object); 
         c = ARG_GET(a);
         if (c == NULL) {
	         printk("Required option(group) missing :%s \n",c);
                 return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&group) != 1) {
             printk("group not ok :%s \n",c);
             return CMD_FAIL;
         }
         printk("Group:%d \n",group); 
         rv = bcm_stat_group_create(unit,object,group,&stat_counter_id,
                            &num_entries);
         if(BCM_SUCCESS(rv)) {
           printk("Group Creation ENDED Counter_Id %d:NumOfEntries %d\n",
                 stat_counter_id,num_entries);
         } else {
           printk("Group Creation FAILED: Reason: %s\n",bcm_errmsg(rv));
         } 
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-group-destroy")) {
         c = ARG_GET(a);
         if (c == NULL) {
             printk("Required option(stat_counter_id) missing:%s\n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&stat_counter_id) != 1) {
             printk("group not ok :%s \n",c);
             return CMD_FAIL;
         }
         rv = bcm_stat_group_destroy(unit,stat_counter_id);
         if(BCM_SUCCESS(rv)) {
            printk("Group Deletion ENDED with Counter_Id %d: \n",
                         stat_counter_id);
         } else {
           printk("Group Deletion FAILED: Reason: %s\n",bcm_errmsg(rv));
         } 
         return BCM_E_NONE;
       }
       /* Port Related Calls Start over here*/
       if (!sal_strcasecmp(c, "stat-attach-port-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "gport")) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(gport value) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%d",&gport) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
         } else {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(port-name ) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"ge%u",&port_number) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
             BCM_GPORT_LOCAL_SET(gport,port_number+1);
         } 
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&stat_counter_id) != 1) {
             printk("group not ok :%s \n",c);
             return CMD_FAIL;
         }
         rv = bcm_port_stat_attach(unit,gport,stat_counter_id);
         if(BCM_SUCCESS(rv)) {
            printk("Attached gport:%d with id:%d \n",
                         gport,stat_counter_id);
         } else {
            printk("Attachment failed Reason :%s \n",bcm_errmsg(rv));
         }
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-detach-port-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "gport")) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(gport value) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%d",&gport) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
         } else {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(port-name ) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"ge%u",&port_number) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
             BCM_GPORT_LOCAL_SET(gport,port_number+1);
         } 
         rv = bcm_port_stat_detach(unit,gport);
         if(BCM_SUCCESS(rv)) {
            printk("Detached gport:%d \n",gport);
         } else {
            printk("Detachment failed Reason :%s \n",bcm_errmsg(rv));
         } 
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-get-port-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "gport")) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(gport value) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%d",&gport) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
         } else {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(port-name ) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"ge%u",&port_number) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
             BCM_GPORT_LOCAL_SET(gport,port_number+1);
         } 
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(ingress | egress) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "ingress")) {
             direction=0;
         } else {
             direction=1;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "byte")) {
             byte_flag=1;
             if(direction == 0) {
                 stat = bcmPortStatIngressBytes; 
             } else {
                 stat = bcmPortStatEgressBytes; 
             } 
         } else {
             byte_flag=0;
             if(direction == 0) {
                 stat = bcmPortStatIngressPackets; 
             } else {
                 stat = bcmPortStatEgressPackets; 
             } 
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&total_counters) != 1) {
	     printk("Not a valid counter values %s \n",c);
             return CMD_USAGE;
         }
         indexes = sal_alloc(sizeof(uint32)*total_counters,"indexes");
         values = sal_alloc(sizeof(bcm_stat_value_t)*total_counters,"values");
         for(count=0;count<total_counters;count++) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(index ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&indexes[count]) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
             }
         }  
         rv = bcm_port_stat_counter_get(
                  unit,
                  gport,
                  stat,
                  total_counters,
                  indexes,
                  values);
         if(BCM_SUCCESS(rv)) {
          printk("Got counters :%s \n",bcm_errmsg(rv));
          printk("################################\n");
          for(count=0;count<total_counters;count++) {
             printk("--------------------------------\n");
             if(byte_flag == 0) {
                printk("CounterIndex:%d Value:%d \n",
                        count,values[count].packets);
             } else {
                printk("CounterIndex:%d HiValue:%d LoValue:%d \n",
                        count,COMPILER_64_HI(values[count].bytes),
                        COMPILER_64_LO(values[count].bytes));
             } 
             printk("--------------------------------\n");
          } 
          printk("################################\n");
         } else {
           printk("Failed to get counter Reason:%s\n",bcm_errmsg(rv));
         }
         sal_free(indexes);
         sal_free(values);
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-set-port-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
             printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "gport")) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(gport value) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%d",&gport) != 1) {
               printk("gport value not ok :%s \n",c);
               return CMD_FAIL;
             }
         } else {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(port-name ) missing :%s \n",c);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"ge%u",&port_number) != 1) {
                 printk("gport value not ok :%s \n",c);
                 return CMD_FAIL;
             }
             BCM_GPORT_LOCAL_SET(gport,port_number+1);
         } 
         c = ARG_GET(a);
         if (c == NULL) {
             printk("Required option(ingress | egress) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "ingress")) {
             direction=0;
         } else {
             direction=1;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "byte")) {
             byte_flag=1;
             if(direction == 0) {
                 stat = bcmPortStatIngressBytes; 
             } else {
                 stat = bcmPortStatEgressBytes; 
             } 
         } else {
             byte_flag=0;
             if(direction == 0) {
                 stat = bcmPortStatIngressPackets; 
             } else {
                 stat = bcmPortStatEgressPackets; 
             } 
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&total_counters) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 return CMD_USAGE;
         }
         indexes = sal_alloc(sizeof(uint32)*total_counters,"indexes");
         values = sal_alloc(sizeof(bcm_stat_value_t)*total_counters,"values");
         for(count=0;count<total_counters;count++) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(index ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&indexes[count]) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
             }
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(value ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if(byte_flag == 0) { 
                if (sscanf(c,"%u",&values[count].packets) != 1) {
	            printk("Not a valid counter values %s \n",c);
                    sal_free(indexes);
                    sal_free(values);
                    return CMD_USAGE;
                }
             } else {
                if (sscanf(c,"%u:%u",&hi_value,&lo_value) != 2) {
	            printk("Not a valid counter values %s \n",c);
                    sal_free(indexes);
                    sal_free(values);
                    return CMD_USAGE;
                }
                COMPILER_64_SET(values[count].bytes,hi_value,lo_value);
             }
         }  
         rv = bcm_port_stat_counter_set(
                  unit,
                  gport,
                  stat,
                  total_counters,
                  indexes,
                  values);
         if(BCM_SUCCESS(rv)) {
            printk("Success in setting counters \n");
         } else {
            printk("Failed in setting counters Reason:%s \n",bcm_errmsg(rv));
         }
         sal_free(indexes);
         sal_free(values);
         return BCM_E_NONE;
       }
       /* Port Related Calls End over here*/

       /* Vlan Related Calls Start over here */
       if (!sal_strcasecmp(c, "stat-attach-vlan-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(vlan-index) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&vlan_index) != 1) {
               printk("vlan_index value not ok :%s \n",c);
               return CMD_FAIL;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(stat_counter_id) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&stat_counter_id) != 1) {
             printk("group not ok :%s \n",c);
             return CMD_FAIL;
         }
         rv = bcm_vlan_stat_attach(unit,vlan_index,stat_counter_id);
         if(BCM_SUCCESS(rv)) {
            printk("Attached vlan:%d with id:%d \n",
                         vlan_index,stat_counter_id);
         } else {
            printk("Attachment failed Reason :%s \n",bcm_errmsg(rv));
         }
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-detach-vlan-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(vlan-index) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&vlan_index) != 1) {
               printk("vlan_index value not ok :%s \n",c);
               return CMD_FAIL;
         }
         rv = bcm_vlan_stat_detach(unit,vlan_index);
         if(BCM_SUCCESS(rv)) {
            printk("Detached vlan:%d \n",vlan_index);
         } else {
            printk("Detachment failed Reason :%s \n",bcm_errmsg(rv));
         } 
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-get-vlan-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(vlan-index) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&vlan_index) != 1) {
               printk("vlan_index value not ok :%s \n",c);
               return CMD_FAIL;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(ingress | egress) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "ingress")) {
             direction=0;
         } else {
             direction=1;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "byte")) {
             byte_flag=1;
             if(direction == 0) {
                 stat = bcmVlanStatIngressBytes; 
             } else {
                 stat = bcmVlanStatEgressBytes; 
             } 
         } else {
             byte_flag=0;
             if(direction == 0) {
                 stat = bcmVlanStatIngressPackets; 
             } else {
                 stat = bcmVlanStatEgressPackets; 
             } 
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(total_counter) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&total_counters) != 1) {
	     printk("Not a valid counter values %s \n",c);
             return CMD_USAGE;
         }
         indexes = sal_alloc(sizeof(uint32)*total_counters,"indexes");
         values = sal_alloc(sizeof(bcm_stat_value_t)*total_counters,"values");
         for(count=0;count<total_counters;count++) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(index ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&indexes[count]) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
             }
         }  
         rv = bcm_vlan_stat_counter_get(
                  unit,
                  vlan_index,
                  stat,
                  total_counters,
                  indexes,
                  values);
         if(BCM_SUCCESS(rv)) {
          printk("Got counters :%s \n",bcm_errmsg(rv));
          printk("################################\n");
          for(count=0;count<total_counters;count++) {
             printk("--------------------------------\n");
             if(byte_flag == 0) {
                printk("CounterIndex:%d Value:%d \n",
                        count,values[count].packets);
             } else {
                printk("CounterIndex:%d HiValue:%d LoValue:%d \n",
                        count,COMPILER_64_HI(values[count].bytes),
                        COMPILER_64_LO(values[count].bytes));
             } 
             printk("--------------------------------\n");
          } 
          printk("################################\n");
         } else {
           printk("Failed to get counter Reason:%s\n",bcm_errmsg(rv));
         }
         sal_free(indexes);
         sal_free(values);
         return BCM_E_NONE;
       }
       if (!sal_strcasecmp(c, "stat-set-vlan-counter")) {
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(vlan-index) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&vlan_index) != 1) {
               printk("vlan_index value not ok :%s \n",c);
               return CMD_FAIL;
         }
         c = ARG_GET(a);
         if (c == NULL) {
             printk("Required option(ingress | egress) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "ingress")) {
             direction=0;
         } else {
             direction=1;
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(byte | packet) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if(!sal_strcasecmp(c, "byte")) {
             byte_flag=1;
             if(direction == 0) {
                 stat = bcmVlanStatIngressBytes; 
             } else {
                 stat = bcmVlanStatEgressBytes; 
             } 
         } else {
             byte_flag=0;
             if(direction == 0) {
                 stat = bcmVlanStatIngressPackets; 
             } else {
                 stat = bcmVlanStatEgressPackets; 
             } 
         }
         c = ARG_GET(a);
         if (c == NULL) {
	     printk("Required option(total_counter) missing :%s \n",c);
             return CMD_USAGE;
	 }
         if (sscanf(c,"%u",&total_counters) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 return CMD_USAGE;
         }
         indexes = sal_alloc(sizeof(uint32)*total_counters,"indexes");
         values = sal_alloc(sizeof(bcm_stat_value_t)*total_counters,"values");
         for(count=0;count<total_counters;count++) {
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(index ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if (sscanf(c,"%u",&indexes[count]) != 1) {
	         printk("Not a valid counter values %s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
             }
             c = ARG_GET(a);
             if (c == NULL) {
	         printk("Required option(value ) missing :%s \n",c);
                 sal_free(indexes);
                 sal_free(values);
                 return CMD_USAGE;
	     }
             if(byte_flag == 0) { 
                if (sscanf(c,"%u",&values[count].packets) != 1) {
	            printk("Not a valid counter values %s \n",c);
                    sal_free(indexes);
                    sal_free(values);
                    return CMD_USAGE;
                }
             } else {
                if (sscanf(c,"%u:%u",&hi_value,&lo_value) != 2) {
	            printk("Not a valid counter values %s \n",c);
                    sal_free(indexes);
                    sal_free(values);
                    return CMD_USAGE;
                }
                COMPILER_64_SET(values[count].bytes,hi_value,lo_value);
             }
         }  
         rv = bcm_vlan_stat_counter_set(
                  unit,
                  gport,
                  stat,
                  total_counters,
                  indexes,
                  values);
         if(BCM_SUCCESS(rv)) {
            printk("Success in setting counters \n");
         } else {
            printk("Failed in setting counters Reason:%s \n",bcm_errmsg(rv));
         }
         sal_free(indexes);
         sal_free(values);
         return BCM_E_NONE;
       }
       /* Vlan Related Calls End over here */
    }
#endif
    return (cmd);		
}
#endif
