/*
 * $Id: enet.c 1.27 Broadcom SDK $
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
 * FE2000 Ethernet OAM functions
 */

#if defined(INCLUDE_L3)

#include <bcm_int/sbx/fe2000/oam/oam.h>
#include <bcm_int/sbx/fe2000/oam/pm.h>
#include <bcm_int/sbx/fe2000/vswitch.h>
extern oam_state_t* _state[SOC_MAX_NUM_DEVICES];

STATIC int
_oam_find_msb(uint32_t bitfield, int32_t *msb)
{
    int found = BCM_E_NOT_FOUND;
    for (*msb = 31; *msb >= 0; (*msb)--) {
        if (bitfield & (1 << *msb)) {
            found = BCM_E_NONE;
            break;
        }
    }
    return found;
}

STATIC int
_oam_find_vid(int unit, bcm_port_t port, bcm_oam_endpoint_info_t *ep_info, bcm_vlan_t *vid)
{
  int rv = BCM_E_NONE;
  int dir;
  soc_sbx_g2p3_evp2e_t              evp2e_rec;
  soc_sbx_g2p3_eteencap_t           eteencap_rec;
  soc_sbx_g2p3_etel2_t              etel2_rec;

  dir = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);

  /* up vid translation */
  if (dir) {
    /* walk the egress path to see if there is a vid translation */
    rv = soc_sbx_g2p3_evp2e_get (unit, ep_info->vlan, port, &evp2e_rec);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get evp2e: %d %s\n"),
                 rv, bcm_errmsg(rv)));
        return rv;
    }
    rv = soc_sbx_g2p3_eteencap_get (unit, evp2e_rec.eteptr, &eteencap_rec);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get eteencap 0x%x: %d %s\n"),
                 evp2e_rec.eteptr, rv, bcm_errmsg(rv)));
        return rv;
    }
    if (!(eteencap_rec.l2ete)) {
      eteencap_rec.l2ete = evp2e_rec.eteptr;
    }
    rv = soc_sbx_g2p3_etel2_get (unit, eteencap_rec.l2ete, &etel2_rec);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get etel2 0x%x: %d %s\n"),
                 eteencap_rec.l2ete, rv, bcm_errmsg(rv)));
        return rv;
    }
    if (etel2_rec.usevid) {
      *vid = etel2_rec.vid;
      OAM_VERB((_SBX_D(unit, "oam up vid %x on vlan %x \n"), 
                vid, ep_info->vlan));
    } else {
      *vid = ep_info->vlan;
      OAM_VERB((_SBX_D(unit, "oam up vid %x \n"), *vid));
    }
  } else {
    *vid = ep_info->vlan;
  }
  return rv;
}

STATIC int
_oam_enet_config_portmd_passive_sap (int unit, bcm_port_t port,
                                     int level, int active_dir,
                                     int delete)
{
    int      rv = BCM_E_NONE;
    int      port_mdl;
    int      passive_dir;
    int      mdlvl;

    soc_sbx_g2p3_oamportmdlevel2etc_t pmd;

    /* configuration of passive sap for port, MdLevel lookup  */

    passive_dir = !active_dir;
    port_mdl = (passive_dir << 9 ) + (port << 3) + level;

    rv = soc_sbx_g2p3_oamportmdlevel2etc_get(unit, port_mdl, &pmd);
    if (rv != BCM_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) entry get\n"),
                 passive_dir, port, level));
    }

     /* Check to see if there is a valid Endpoint configured already */
    if (pmd.valid) {
        /* do not change existing endpoint */
    } else {
        /* if delete, then enable forwading */
        pmd.fwd = delete;
        rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdl, &pmd);
        if (BCM_FAILURE(rv)) {
          OAM_ERR((_SBX_D(unit, "Failed port/lvl=(%d/%d) entry add: %d %s\n"),
                   port, level, rv, bcm_errmsg(rv)));
        } else {
          OAM_DEBUG((_SBX_D(unit, "Untagged peer endpoint. Added "
                            "Port/MdLvl %d/%d entry\n"), port, level));
        }
    }

    if (delete) {
        /* For mdLevels lower than current mdLevel, enable forwarding */
        if (level > 0) {
            for (mdlvl = level - 1; mdlvl >= 0; mdlvl--) {
                port_mdl = (passive_dir << 9) + (port << 3) + mdlvl;
                rv = soc_sbx_g2p3_oamportmdlevel2etc_get(unit, port_mdl, &pmd);
                if (rv != BCM_E_NONE) {
                     OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) "
                                     "entry get\n"),passive_dir, port, mdlvl));
                 }
                /* if a valid endpoint if found, stop enable forwarding at this mdLevel */
                if (pmd.valid) {
                    break;
                }
                pmd.fwd = 1;
                rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdl, &pmd);
                if (rv != BCM_E_NONE) {
                   OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d)"
                                   " entry set\n"),passive_dir, port, mdlvl));
                }
            }
        }
    } else {
        /* For unconfigured mdLevels lower than current mdLevel, clear forwarding. */
        for (mdlvl=0; mdlvl < level; mdlvl++) {
            port_mdl = (passive_dir << 9 ) + (port << 3) + mdlvl;
            rv = soc_sbx_g2p3_oamportmdlevel2etc_get(unit, port_mdl, &pmd);
            if (rv != BCM_E_NONE) {
                  OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d)"
                                  " entry get\n"),passive_dir, port, mdlvl));
            }
            if (!(pmd.valid)) {
                pmd.fwd = 0;
                rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdl, &pmd);
                if (rv != BCM_E_NONE) {
                    OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) "
                                    "entry set\n"),passive_dir, port, mdlvl));
                }
            }
        } /* end for mdlvl */
    }

    return rv;
}

STATIC int
_oam_enet_config_passive_sap (int unit, bcm_port_t port,
                              bcm_vlan_t vid, int active_dir,
                              int tag, int level, int delete)
{
    int      rv = BCM_E_NONE;
    int      passive_dir;
    int      port_mode;
    int32_t  highest_mdlevel=0;

    soc_sbx_g2p3_oampvd2e_t    passive_rec;

    /* passive direction is the opposite of the active direction */
    passive_dir = !active_dir;

    tag = 0;

    /* need to determine the port mode to configure tag setting */
    /* - set tag if passive_direction=up and port=customer */
    rv = bcm_port_dtag_mode_get (unit, port, &port_mode);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed dtag port mode get port=(%d)\n"),
                 port));
    }
    if ((port_mode == BCM_PORT_DTAG_MODE_EXTERNAL) && (passive_dir)) {
        tag = 1;
    }

    rv = soc_sbx_g2p3_oampvd2e_get (unit, port, vid,
                                    passive_dir, tag, &passive_rec );

    if (rv == BCM_E_NOT_FOUND) {

        if (delete) {
            OAM_ERR((_SBX_D(unit, "delete passive SAP "
                            "port/vid/dir/tag=(%d/0x%04x %d %d) not found\n"),
                     port, vid, passive_dir, tag));
        }

        soc_sbx_g2p3_oampvd2e_t_init (&passive_rec);

        /*
         * If no Endpoints on Passive SAP:
         * - allow messages at higher mdLevels to pass through.
         * - set all bits in the mdLevelFwd mask starting at level + 1 and higher
         * - set mdLevelEp to 0 (no active endpoint)
         */
        passive_rec.mdLevelFwd  = ~((1 << (level + 1)) - 1);
        passive_rec.mdLevelFwd &= 0xFF;
        passive_rec.mdLevelEp   = 0;

        rv = soc_sbx_g2p3_oampvd2e_set (unit, port, vid, passive_dir, tag, &passive_rec);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed port/vid=(%d/0x%04x) lookup add\n"),
                     port, vid));
        }

    } else {

       if (delete) {
            if (passive_rec.mdLevelEp != 0) {
                if (level != OAM_NO_LEVEL) {
                     /* find the highest endpoint */
                    rv = _oam_find_msb(passive_rec.mdLevelEp, &highest_mdlevel);
                     /* use the higher of active or passive SAP endpoint */
                    if (highest_mdlevel > level) {
                        passive_rec.mdLevelFwd &= ~((1 << (highest_mdlevel + 1)) - 1);
                    } else {
                        passive_rec.mdLevelFwd &= ~((1 << (level + 1)) - 1);
                    }
                    rv = soc_sbx_g2p3_oampvd2e_update (unit, port, vid,
                                                       passive_dir, tag, &passive_rec);
                }
            } else {
                if (level != OAM_NO_LEVEL) {
                     /* set the the mdLevelFwd mask to the highest level +1 and higher */
                    passive_rec.mdLevelFwd = ~((1 << (level + 1)) - 1);
                    rv = soc_sbx_g2p3_oampvd2e_update (unit, port, vid,
                                                       passive_dir, tag, &passive_rec);
                } else {
                     /* if this is the last passive sap (level = 0), then remove the entry */
                    rv = soc_sbx_g2p3_oampvd2e_delete (unit, port,
                                                       vid, passive_dir, tag);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed to remove port/vid "
                                        "lookup port=%d  vid=%d err=%d %s\n"),
                                 port, vid, rv, bcm_errmsg(rv)));
                    }
                }
            }
        } else {

            /*
             * If Endpoint(s) on Passive SAP:
             * - allow messages at higher mdLevels to pass through.
             * - AND all bits in the mdLevelFwd mask starting at level + 1 and higher
             */
            passive_rec.mdLevelFwd &= ~((1 << (level + 1)) - 1);
            rv = soc_sbx_g2p3_oampvd2e_update (unit, port, vid, passive_dir, tag, &passive_rec);
        }
    }

    return rv;
}

STATIC int
_oam_enet_config_rx_local(int unit, bcm_oam_endpoint_info_t *ep_info,
                           bcm_trunk_add_info_t *port_info,
                           uint32_t ep_rec_index)
{
    int rv = BCM_E_NONE;
    int i=0;
    bcm_port_t   port;
    int          dir;
    int          tag;
    bcm_vlan_t   vid;
    bcm_module_t mod;
    soc_sbx_g2p3_oampvd2e_t           etc_rec;
    uint32_t                          intermediate_ep;
    uint32_t                          remap_type;
    soc_sbx_g2p3_oamepremap_t         epremap;

    if (!port_info || !ep_info) {
        OAM_ERR((_SBX_D(unit, "invalid pointer\n")));
        return BCM_E_INTERNAL;
    }

    remap_type = _state[unit]->ep_subtypes.cfm;

    /* Set up the key */
    dir = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);

    /* Set up the tag location */
    tag = 0;

    /* set the RX table entries.  if this is a LAG, each port in
       the LAG will point to the same Endpoint entry.  */
    for (i=0; i<port_info->num_ports; i++) {
        port = port_info->tp[i];
        mod  = port_info->tm[i];

        /* BCM_VLAN_VALID is:  0 < vid < 0x1000
         * G2P3 reserves 0xFFF as an ETE flag specifying "untagged-vid".
         */
        if ((BCM_VLAN_VALID(ep_info->vlan)) && 
            (ep_info->vlan != _BCM_VLAN_G2P3_UNTAGGED_VID)) {
            /*  PVD table */

            /* up vid translation */
            rv = _oam_find_vid(unit, port, ep_info, &vid);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to resolve vid "
                                "translation: %d %s\n"),
                         rv, bcm_errmsg(rv)));
                return rv;
            }

            rv = soc_sbx_g2p3_oampvd2e_get(unit, port, vid, 
                                           dir, tag, &etc_rec );
            if (rv == BCM_E_NOT_FOUND) { /* if there is not a pvd2e entry, add one */
                /* else (below) updates Fwd and Ep masks */
                /* Add port,vid,dir hash lookup */
                soc_sbx_g2p3_oampvd2e_t_init(&etc_rec);
  
                if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
                  /* for MIPs, allow OAM messages at upper and lower MdLevels 
                   * to pass through.  Set all bits in the 
                   * fwd mask, other than that at the MIP's mdlevel. */
                  etc_rec.mdLevelFwd = ~(1 << ep_info->level);
                  /* uP MIP use the tag_selection flag on CEP ports */
                  if (dir && (ep_info->flags & BCM_OAM_ENDPOINT_MATCH_INNER_VLAN)) {
                      tag = 1;
                  }

                } else {
                  /* for MEPs, only allow messages at higher MdLevels to pass through.
                   * set all bits in the MDLevelFwd mask starting at
                   * MDLevel + 1 and higher
                   */
                  etc_rec.mdLevelFwd  = ~((1 << (ep_info->level + 1)) - 1);
                }
                etc_rec.mdLevelFwd &= 0xFF;
                etc_rec.mdLevelEp   = 1 << ep_info->level;
                etc_rec.endpointPtr = ep_rec_index;  /* this is the "local" entry's ID */
                etc_rec.ftIdx       = vid;

                intermediate_ep = etc_rec.endpointPtr;
                epremap.epIdx   = ep_rec_index;
                /* write the remap table entry prior to updating cuckoo */
                rv = soc_sbx_g2p3_oamepremap_set(unit, intermediate_ep, ep_info->level, 
                                                 remap_type, &epremap);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed oamEpRemap set for "
                                    "endpoint 0x%04x, InEp/mdLevel/type/="
                                    "%d %d %d\n"),
                             ep_rec_index, intermediate_ep, 
                             ep_info->level, remap_type));
                }

                rv = soc_sbx_g2p3_oampvd2e_set(unit, port, vid,
                                               dir, tag, &etc_rec);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed port/vid=(%d/0x%04x) "
                                    "lookup add\n"),
                             port, vid));
                }

                /* Calculate Passive Sap information and add reverse direction entry */
                if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
                    /* no passive sap configuration on MIP */
                } else {
                    rv = _oam_enet_config_passive_sap (unit, port, vid, 
                                                       dir, tag, ep_info->level, 0);
                }

                if (BCM_SUCCESS(rv)) {
                    OAM_DEBUG((_SBX_D(unit, 
                                      "Tagged peer endpoint. Added "
                                      "Port/Vid/Dir "
                                      "%d/0x%04x/%s entry\n"), 
                               port, vid, 
                               ((ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING) ?
                                "UP" : "DOWN")));
                    OAM_DEBUG((_SBX_D(unit, "oamEpRemap entry with same Endpoint"
                                      " InEp/mdLevel/type/=%d %d %d\n"),
                               intermediate_ep, ep_info->level, remap_type));
                }
            } else if (rv == BCM_E_NONE) { 
                /* if there was a pvd2e entry, update the cuckoo payload 
                 * masks to reflect multiple mdlevels and/or types 
                 * after writing the oamEpRemap entry for the new mdlevel/type 
                 */
                /* If Caladan (C1), warning message regarding possible nested entries */
                if ((!SOC_IS_SBX_FE2KXT(unit)) && 
                    (etc_rec.mdLevelEp != (1 << ep_info->level))) {
                    OAM_WARN((_SBX_D(unit, "Multiple MD Levels for Endpoint 0x%04x"
                                     " not redirected on BCM88020 device\n"),
                              ep_rec_index));
                } 
                intermediate_ep = etc_rec.endpointPtr;
                epremap.epIdx   = ep_rec_index;
                /* write the remap table entry prior to updating cuckoo */
                rv = soc_sbx_g2p3_oamepremap_set(unit, intermediate_ep,
                                                 ep_info->level, 
                                                 remap_type, &epremap);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed oamEpRemap set for endpoint "
                                    "0x%04x, InEp/mdLevel/type/=%d %d %d\n"),
                             ep_rec_index, intermediate_ep, ep_info->level,
                             remap_type));
                }
                      
                /* entry may exist only as passive sap.  if so, add active endpoint information */
                if (etc_rec.endpointPtr == 0) {
                    etc_rec.endpointPtr = ep_rec_index;
                    etc_rec.ftIdx       = vid;
                }

                if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
                    /* for MIPs, allow OAM messages at lower MdLevels to pass through. */
                    etc_rec.mdLevelFwd &= ~(1 << ep_info->level);
                } else {
                    etc_rec.mdLevelFwd &= ~((1 << (ep_info->level + 1)) - 1);
                }
                etc_rec.mdLevelEp   |= 1 << ep_info->level;
                rv = soc_sbx_g2p3_oampvd2e_update(unit, port, vid, 
                                                  dir, tag, &etc_rec);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed port/vid/dir=(%d/0x%04x) pvd2e update\n"),
                             port, vid, dir));
                }

                /* Calculate Passive Sap information and add reverse direction entry */
                if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
                    /* no passive sap configuration on MIP */
                } else {
                    rv = _oam_enet_config_passive_sap (unit, port, vid, 
                                                       dir, tag, ep_info->level, 0);
                }

            } else {
                /* an error code other than NOT_FOUND or NONE was returned. */
                OAM_ERR((_SBX_D(unit, "Failed to get port/vid/dir=(%d/0x%04x).  Error = %d\n"),
                         port, vid, dir, rv));
            }

            /* PMD table */
        } else {  /* if (!BCM_VLAN_VALID(ep_info->vlan)) */
            uint32_t port_mdl;
            int mdlvl;
            soc_sbx_g2p3_oamportmdlevel2etc_t pmd;

            /* add remap entry which is a 1:1 remapping for endpoint id */
            epremap.epIdx   = ep_rec_index;
            /* write the remap table entry prior to updating cuckoo */
            rv = soc_sbx_g2p3_oamepremap_set(unit, ep_rec_index, ep_info->level,
                                             remap_type, &epremap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed oamEpRemap set for endpoint "
                                "0x%04x, InEp/mdLevel/type/=%d %d %d\n"),
                         ep_info->id, ep_rec_index, ep_info->level, remap_type));
            }

            /* Add port, MdLevel lookup  */
            soc_sbx_g2p3_oamportmdlevel2etc_t_init(&pmd);
            pmd.valid = 1;
            pmd.fwd = 0;
            pmd.epIdx = ep_rec_index; /* this is the "local" entry's ID. */

            port_mdl = (dir << 9 ) + (port << 3) + ep_info->level;

            rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdl, &pmd);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed port/lvl=(%d/%d) entry add: %d %s\n"),
                         port, ep_info->level, rv, bcm_errmsg(rv)));
            } else {
                OAM_DEBUG((_SBX_D(unit, "Untagged peer endpoint. Added Port/MdLvl %d/%d "
                                  "entry\n"), port, ep_info->level));
            }
            /* For unconfigured mdLevels lower than current mdLevel, clear forwarding.
             * only if we're configuring a MEP.  
             * if this is a MIP, do not block lower levels. */
            if (!(ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) {
                for (mdlvl=0; mdlvl < ep_info->level; mdlvl++) {
                    port_mdl = (dir << 9 ) + (port << 3) + mdlvl;
                    rv = soc_sbx_g2p3_oamportmdlevel2etc_get(unit, port_mdl, &pmd);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) entry get\n"),dir, port, mdlvl));
                    }
                    if (!(pmd.valid)) {
                        pmd.fwd = 0;
                        rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdl, &pmd);
                        if (rv != BCM_E_NONE) {
                            OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) entry set\n"),dir, port, mdlvl));
                        }
                    }
                } /* end for mdlvl */
            } /* end if MIP */

            /* configuration of passive sap for port, MdLevel lookup  */
            if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
              /* no passive sap configuration on MIP */
            } else {
              _oam_enet_config_portmd_passive_sap (unit, port, ep_info->level, dir, 0);
            }

        } /* end PMD */
    } /* end for-loop (port) */

    return rv;
}

STATIC int
_oam_enet_config_rx_peer(int unit, bcm_oam_endpoint_info_t *ep_info,
                         uint32_t ep_rec_index)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_oammaidmep2e_t       mamep_ptr;

    if (!ep_info) {
        OAM_ERR((_SBX_D(unit, "Internal error, bogus pointer in _oam_enet_config_rx\n")));
        return BCM_E_FAIL;
    }

    /* add lookups only on new entries */
    soc_sbx_g2p3_oammaidmep2e_t_init(&mamep_ptr);

    /* The peer endpoint's id. */
    mamep_ptr.endpointPtr = ep_rec_index;
    mamep_ptr.epValid = 1;

    OAM_VERB((_SBX_D(unit, "_oam_enet_config_rx_peer id=0x%x: adding (cookie, mepid)=(0x%x, 0x%x)\n"),
              ep_rec_index, ep_info->local_id, ep_info->name));

    /* the keys to this cuckoo are MAID-Cookie and MEP-ID.
     * the MAID-Cookie is the local endpoint's id stashed in the ep_info->local_id.
     */
    rv = soc_sbx_g2p3_oammaidmep2e_set(unit, ep_info->local_id,
                                       ep_info->name,
                                       &mamep_ptr);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to set oam maidmep 0x%x 0x%x: %d %s\n"),
                 ep_info->local_id, ep_info->name, rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}


STATIC int
_oam_enet_remove_rx_local(int unit, bcm_oam_endpoint_info_t *ep_info,
                          bcm_trunk_add_info_t *port_info)
{
    int rv = BCM_E_NONE;
    int i=0;
    int dir;
    int tag;
    int mdlvl;
    int32_t      highest_mdlevel=0;
    bcm_vlan_t   vid;
    uint32_t     port_mdlvl;
    bcm_port_t   port;
    bcm_module_t mod;
    soc_sbx_g2p3_oampvd2e_t           etc_rec;
    uint32_t                          remap_type;
    soc_sbx_g2p3_oamportmdlevel2etc_t pmd;

    if (!port_info || !ep_info) {
        OAM_ERR((_SBX_D(unit, "Internal error, bogus pointer in _oam_enet_remove_rx\n")));
        return BCM_E_FAIL;
    }

    remap_type = _state[unit]->ep_subtypes.cfm;

    dir = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);
    tag = 0;

    /* uP MIP use the tag_selection flag on CEP ports */
    if (dir && (ep_info->flags & BCM_OAM_ENDPOINT_MATCH_INNER_VLAN)) {
        tag = 1;
    }

    /* remove the RX table entries.  */
    for (i=0; i<port_info->num_ports; i++) {

        port = port_info->tp[i];
        mod  = port_info->tm[i];

        /* BCM_VLAN_VALID is:  0 < vid < 0x1000
         * G2P3 reserves 0xFFF as an ETE flag specifying "untagged-vid".
         */
        if ((BCM_VLAN_VALID(ep_info->vlan)) && (ep_info->vlan != _BCM_VLAN_G2P3_UNTAGGED_VID)) {

            /* up vid translation */
            rv = _oam_find_vid(unit, port, ep_info, &vid);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to resolve vid translation: %d %s\n"),
                         rv, bcm_errmsg(rv)));
                return rv;
            }

            rv = soc_sbx_g2p3_oampvd2e_get(unit, port, vid,
                                           dir, tag, &etc_rec);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to find the Port/Vid/Dir entry (mod/port)=%d/%d "
                                "vid=%d err=%d %s\n"), mod, port, vid,
                         rv, bcm_errmsg(rv)));
            } else { 

                /* clear level in ep mdLevel mask */
                etc_rec.mdLevelEp &= ~(1 << ep_info->level);

                /* after removing this endpoint's mdlevel from the EP-Mask,
                 * check to see if there are other (nested) endpoints still
                 * at this port/vid.  if so, recompute the Forward-Mask by
                 * searching for the highest remaining MdLevel in the EP-Mask,
                 * and then constructing the Forward-Mask from that.
                 */
                rv = _oam_find_msb(etc_rec.mdLevelEp, &highest_mdlevel);
                if (BCM_E_NONE == rv) {

                    /* recomputing the Forward-Mask here will have no information
                     * about whether the remaining MdLevels are MEPs or MIPs.
                     * the Fwd-Mask is calculated differently for MIPs.  this
                     * code cant determine MEP or MIP so it will assume MEP.
                     * the application is responcible for removing nested MIPs
                     * and re-creating them if it removes any nested MEP or MIP.
                     */
                    etc_rec.mdLevelFwd  = ~((1 << (highest_mdlevel + 1)) - 1);
                    etc_rec.mdLevelFwd &= 0xFF;

                    rv = soc_sbx_g2p3_oampvd2e_update(unit, port,
                                                      vid,
                                                      dir, tag, &etc_rec);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed to update Port/Vid lookup mod/port=%d/%d "
                                        "vid=%d err=%d %s\n"), mod, port,
                                 vid, rv, bcm_errmsg(rv)));
                    } else {
                        OAM_DEBUG((_SBX_D(unit, "Tagged peer endpoint.  Updated the Port/Vid/Dir entry\n")));
                        OAM_DEBUG((_SBX_D(unit, "Tagged peer endpoint.  level=0x%x, etc_rec.mdLevelEp=0x%x\n"), 
                                   ep_info->level, etc_rec.mdLevelEp));
                    }
                } else {
                    highest_mdlevel = OAM_NO_LEVEL;
                    rv = soc_sbx_g2p3_oampvd2e_delete(unit, port,
                                                      vid, dir, tag);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed to remove Port/Vid lookup mod/port=%d/%d "
                                        "vid=%d err=%d %s\n"), mod, port,
                                 vid, rv, bcm_errmsg(rv)));
                    } else {
                        OAM_DEBUG((_SBX_D(unit, "Tagged peer endpoint.  Removed the Port/Vid/Dir entry\n")));
                    }
                }

                /* Calculate Passive Sap information and add reverse direction entry */
                if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
                    /* no passive sap configuration on MIP */
                } else {
                    rv = _oam_enet_config_passive_sap (unit, port, vid, dir, tag, highest_mdlevel, 1);

                    /* Check to restore passive sap information on deleted endpoint */
                    rv = soc_sbx_g2p3_oampvd2e_get(unit, port, vid, !dir, tag, &etc_rec);
                    if (rv == BCM_E_NOT_FOUND) {
                        /* no action needed */
                    } else if (rv == BCM_E_NONE) {
                        rv = _oam_find_msb(etc_rec.mdLevelEp, &highest_mdlevel);
                        if (BCM_E_NONE == rv) {
                            rv = _oam_enet_config_passive_sap (unit, port, vid, !dir,
                                                               tag, highest_mdlevel, 0);
                        }
                    } else {
                        /* an error code other than NOT_FOUND or NONE was returned. */
                        OAM_ERR((_SBX_D(unit, "Failed to get port/vid/dir=(%d/0x%04x).  Error = %d\n"),
                                         port, vid, !dir, rv));
                    }
                }
            }
        } else {
            /* clear (disable) the PMD entry */
            soc_sbx_g2p3_oamportmdlevel2etc_t_init(&pmd);
            pmd.valid = 0;
            /* defaulting fwd=1 to flood the packet rather than no-endpoint exception */
            pmd.fwd = 1;
            pmd.epIdx = 0;

            port_mdlvl = (dir << 9) + (port << 3) + ep_info->level;

            rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdlvl, &pmd);
            if (rv != BCM_E_NONE) {
                OAM_ERR((_SBX_D(unit, "Failed port/mdlvl=(%d/%d) entry set\n"),
                         port, ep_info->level));
            }

            OAM_DEBUG((_SBX_D(unit, "Untagged peer endpoint. Cleared Port/MdLvl entry\n")));

            /* For mdLevels lower than current mdLevel, enable forwarding */
            if (ep_info->level > 0) {
                for (mdlvl = ep_info->level - 1; mdlvl >= 0; mdlvl--) {
                    port_mdlvl = (dir << 9) + (port << 3) + mdlvl;
                    rv = soc_sbx_g2p3_oamportmdlevel2etc_get(unit, port_mdlvl, &pmd);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) entry get\n"),dir, port, mdlvl));
                    }
                    /* if a valid endpoint if found, stop enable forwarding at this mdLevel */
                    if (pmd.valid) break;
                    pmd.fwd = 1;
                    rv = soc_sbx_g2p3_oamportmdlevel2etc_set(unit, port_mdlvl, &pmd);
                    if (rv != BCM_E_NONE) {
                        OAM_ERR((_SBX_D(unit, "Failed dir/port/mdlvl=(%d/%d/%d) entry set\n"),dir, port, mdlvl));
                    }
                }
            } 

            /* configuration of passive sap for port, MdLevel lookup  */
            if (ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
              /* no passive sap configuration on MIP */
            } else {
              _oam_enet_config_portmd_passive_sap (unit, port, ep_info->level, dir, 1);
            }

            /* port, MdLevel is active in down direction and passive in up direction */
            /* do not need to restore passive sap information on deleted endpoint    */

        }
    } /* end for */

    return rv;
}

STATIC int
_oam_enet_remove_rx_peer(int unit, uint16 maid_cookie, uint16 ep_name)
{
    int rv = BCM_E_NONE;

    rv = soc_sbx_g2p3_oammaidmep2e_delete(unit, maid_cookie, ep_name);
    if (rv != BCM_E_NONE) {
        OAM_ERR((_SBX_D(unit, "Failed delete maid/mepid=(%d/%d) entry set\n"),
                 maid_cookie, ep_name));
    }

    return rv;
}

int _oam_enet_endpoint_set(int unit, bcm_oam_endpoint_info_t *ep_info, 
                           uint32_t ep_rec_index, egr_path_desc_t *egrPath, 
                           tcal_id_t *tcal_id, bcm_trunk_add_info_t *trunk_info)
{
    int                               rv = BCM_E_NONE;
    uint8_t                           mamep_committed     = 0;
    uint8_t                           oampv2e_committed   = 0;
    uint32_t                          watchdog_id         = INVALID_POLICER_ID;
    bcm_port_t                        port;
    bcm_module_t                      mod;
    bcm_gport_t                       gport;
    int                               gportId; 
    soc_sbx_g2p3_oamep_t              oamep, oamep2;
    uint32_t                          ep_rec_index2 = 0;
    soc_sbx_g2p3_ep2e_t               ep2e;
    uint8_t                           peer_record_written = 0;
    bcm_oam_group_info_t             *group_info = NULL;
    uint32_t                          lsmIdx;
    soc_sbx_g2p3_lsmac_t              lsm;
    soc_sbx_g2p3_oamupmac_t           oamupmac;
    soc_sbx_g2p3_eteencap_t           eteEncap;
    uint32_t                          maid1, maid2, hashReg1, hashReg2;
    oam_sw_hash_data_t               *hash_data = NULL;
    int                               byte, is_smacAux=0;
    uint32_t                          smac_hi=0, smac_lo=0;
    uint32_t                          pushdown_hi=0, pushdown_lo=0, pushdown_aux_hi=0, pushdown_aux_lo=0;
    uint32_t                          smac_lo_6b=0, pushdown_lo_6b=0, pushdown_aux_lo_6b=0;
    uint32_t                          smac_lo_26b=0, pushdown_lo_26b=0, pushdown_aux_lo_26b=0;

    /* sanity check pointers */
    if ((ep_info==NULL) || (egrPath==NULL) || 
        (tcal_id==NULL) || (trunk_info==NULL)) {
        return BCM_E_INTERNAL;
    }

    /* group */
    group_info = OAM_GROUP_INFO(unit, ep_info->group);

    hash_data = &_state[unit]->hash_data_store[ep_rec_index];

    soc_sbx_g2p3_oamep_t_init(&oamep);
    soc_sbx_g2p3_oamep_t_init(&oamep2);
    soc_sbx_g2p3_ep2e_t_init(&ep2e);

    if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
        rv = soc_sbx_g2p3_oamep_get(unit, ep_rec_index, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get oamep 0x%x: %d %s\n"),
                     ep_rec_index, rv, bcm_errmsg(rv)));
            return rv;
        }

        /* the soc layer doesn't handle overlays very well.
         * Must clear all fields not related to this type to ensure
         * bits are set as expected
         */
        _oam_oamep_prepare(unit, &oamep);
    }

    /* check if we're configuring this endpoint on a LAG. */
    if (BCM_GPORT_IS_TRUNK(ep_info->gport)) {
        /* select the "designate" port/mod */
        mod  = trunk_info->tm[0];
        port = trunk_info->tp[0];

    } else if (BCM_GPORT_IS_MODPORT(ep_info->gport)) {
        /* not a trunk, but fill in the trunk_add_info structure
           anyways, as a common representation for the port(s). */
        mod  = BCM_GPORT_MODPORT_MODID_GET(ep_info->gport);
        port = BCM_GPORT_MODPORT_PORT_GET(ep_info->gport);
        trunk_info->num_ports = 1;
        trunk_info->tp[0] = port;
        trunk_info->tm[0] = mod;
    } else {
        return BCM_E_PARAM;
    }

    /************* PEER entry ****************/
    if (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {

        if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {

            /* free the timer and reallocate to simulate a reset */
            if (oamep.policerid != INVALID_POLICER_ID) {
                rv = _oam_timer_free(unit, &oamep.policerid);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to free watchdog timer 0x%x: %d %s\n"),
                             oamep.policerid, rv, bcm_errmsg(rv)));
                    return rv;
                }
            }
            mamep_committed = 1;

        } else {

            /* add the MAID-MEPID cuckoo entry for this peer. */
            rv = _oam_enet_config_rx_peer(unit, ep_info, ep_rec_index);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to configure receive-side table entries: %d %s\n"),
                         rv, bcm_errmsg(rv)));
            } else {
                mamep_committed = 1;
            }
        } /*  if(BCM_OAM_ENDPOINT_REPLACE) */

        oamep.function = _state[unit]->ep_type_ids.ccm_peer;
        oamep.type     = 1; /* Ethernet */

        /* calculate CRC over MAID and set into the peer endpoint. */
        maid2 = MAID_ID_TO_ZF(&group_info->name[0]);
        maid1 = MAID_ID_TO_ZF(&group_info->name[4]);
        hashReg1 = sbCrc32(maid1 ^ maid2);
        for (byte=8; byte<48; byte=byte+8) {
            maid2 = MAID_ID_TO_ZF(&group_info->name[byte]);
            maid1 = MAID_ID_TO_ZF(&group_info->name[byte+4]);
            hashReg2 = sbCrc32(maid1 ^ hashReg1);
            hashReg1 = sbCrc32(hashReg2 ^ maid2);
        }
        OAM_VERB((_SBX_D(unit, "Calculated CRC32 = 0x%8x \n"),hashReg1));

        /* stuff the maid's CRC into the first maid word in the peer endpoint entry */
        oamep.maidcrc = hashReg1;

        /* only peer/remote endpoints require watchdog timers */
        if (BCM_SUCCESS(rv) && ep_info->ccm_period) {
            /* use the "peer" entry's ID. */
            rv = _oam_timer_allocate(unit, ep_rec_index, ep_info->ccm_period,
                                     OAM_DEFAULT_WINDOW_MULTIPLIER,
                                     &watchdog_id);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocated watchdog timer of period %d"
                                " for epId 0x%x: %d %s\n"),
                         ep_info->ccm_period, ep_rec_index, 
                         rv, bcm_errmsg(rv)));
            }
        }
        oamep.policerid = watchdog_id; /* watchdog_id is initialized to INVALID for local EP */

    } else {  /************* LOCAL entry ****************/

        oamep.function      = _state[unit]->ep_type_ids.ccm_first;
        oamep.type          = 1; /* Ethernet */
        oamep.mdlevel       = ep_info->level;
        oamep.mepid         = ep_info->name;
        oamep.dir           = !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING);
        oamep.mip           = !!(ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE);

        /* when p2e.customer = 0, (provider port) the endpoint entry's
         * int_pri will be written into the erh.rcos/rdp field.
         */
        oamep.intpri = (((int)(ep_info->int_pri)) & 0x1f);

        /* per endpoint RDI TX Defect */
        oamep.rdi = !!(ep_info->flags & BCM_OAM_ENDPOINT_REMOTE_DEFECT_TX); 

        /* for Up MEPs:   sid = gport 
         * for Down MEPs: sid = 0xffffffff
         */
        if (!!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING)) {
            rv = _bcm_fe2000_vswitch_port_gport_get (unit, port, ep_info->vlan, &gport); 
            if (rv == BCM_E_NOT_FOUND) {
                rv = soc_sbx_g2p3_ep2e_get(unit, port, &ep2e);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to get EP2E for port %d \n"), port));
                }
                oamep2.sid = ep2e.pid;
                OAM_VERB((_SBX_D(unit, "Got sid %d from ep2e. \n"), oamep2.sid));
            } else if (rv == BCM_E_NONE) {
                if (BCM_GPORT_IS_VLAN_PORT (gport)) {
                    gportId = BCM_GPORT_VLAN_PORT_ID_GET (gport);
                    oamep2.sid = VLAN_VGPORT_ID_TO_FT_INDEX (unit, gportId);
                    OAM_VERB((_SBX_D(unit, "Got sid %d from vlan gport. \n"), oamep2.sid));
                } else {
                    OAM_ERR((_SBX_D(unit, "Failed to get pid for gport %x. Not a vlan gport. \n"), gport));
                }
            } else {
                OAM_ERR((_SBX_D(unit, "Failed to get pid, couldnt find vlan gport. Err %d %s \n"),
                          gport, rv, bcm_errmsg(rv)));
            }
        } else {
            oamep2.sid = 0x3fff;
        }
        OAM_DEBUG((_SBX_D(unit, "Set SID 0x%08x \n"),oamep2.sid));
    
        oamep2.ftidx   = egrPath->ftIdx;

        if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
            oampv2e_committed = 1;

            lsmIdx = ~0;
            rv = _sbx_gu2_ismac_idx_free(unit, hash_data->mac_address,
                                         _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
            OAM_VERB((_SBX_D(unit, "freed mac " L2_6B_MAC_FMT " at %d on replace: "
                             "%s\n"),
                      L2_6B_MAC_PFMT(hash_data->mac_address), lsmIdx, 
                      bcm_errmsg(rv)));
            if (rv == BCM_E_EMPTY) {
                rv = BCM_E_NONE;
            }
        } else {

            /* if we're not just updating the entry, but rather
             * creating a new entry, then the PVD->remap->EP or
             * PMD->EP tables need to be setup for each of the
             * RX ports.  if this gport is not a trunk-id, then
             * there is only one RX port.  however, if is this
             * endpoint is being created on a LAG, then we need
             * to configure the RX table entries on each port in
             * the LAG. */
            rv = _oam_enet_config_rx_local(unit, ep_info, 
                                           trunk_info, ep_rec_index);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to configure local receive-side table entries: %d %s\n"),
                         rv, bcm_errmsg(rv)));
            } else { 
                oampv2e_committed = 1;
            }
        }

        if (BCM_SUCCESS(rv)) {
            /* Add an LSM entry (for both MEPs and MIPs) */
            soc_sbx_g2p3_lsmac_t_init (&lsm);
            rv = _sbx_gu2_ismac_idx_alloc(unit, 0, ep_info->src_mac_address, 
                                          _SBX_GU2_RES_UNUSED_PORT, &lsmIdx);
            
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to allocate local station MAC idx\n")));
            } else {
                OAM_VERB((_SBX_D(unit, "Stored mac " L2_6B_MAC_FMT " at idx %d\n"),
                          L2_6B_MAC_PFMT(ep_info->src_mac_address), lsmIdx));

                hash_data->lsm_idx = lsmIdx;
            }
        }

        if (BCM_SUCCESS(rv)) {
            sal_memcpy (lsm.mac, ep_info->src_mac_address, sizeof(bcm_mac_t));
            lsm.useport = 1;
            lsm.port = port;
            rv = soc_sbx_g2p3_lsmac_set (unit, lsmIdx, &lsm);

            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to write local station MAC\n")));
            }
        }
            
        if (BCM_SUCCESS(rv)) {
            OAM_VERB((_SBX_D(unit, "Allocated LSM idx=%d\n"), lsmIdx));
        }


        /* get the currently configured push-down SMAC. */
        rv = soc_sbx_g2p3_oam_sa_2hi_get(unit, &pushdown_hi);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get smac (hi 2 bytes) for up MEP %d\n"),
                   ep_info->id));
        }
        rv = soc_sbx_g2p3_oam_sa_4lo_get(unit, &pushdown_lo);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get smac (lo 4 bytes) for up MEP %d\n"),
                   ep_info->id));
        }

        /* get the currently configured secondary push-down SMAC. */
        rv = soc_sbx_g2p3_oam_sa_2hi_aux_get(unit, &pushdown_aux_hi);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit,"Failed to get secondary smac (hi 2 bytes) for up MEP %d\n"),
                   ep_info->id));
        }
        rv = soc_sbx_g2p3_oam_sa_4lo_aux_get(unit, &pushdown_aux_lo);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit,"Failed to get secondary smac (lo 4 bytes) for up MEP %d\n"),
                   ep_info->id));
        }

        /* convert the newly configured smac byte
         * array to two uint32s.
         */
        smac_hi = (ep_info->src_mac_address[0] << 8) +
                  (ep_info->src_mac_address[1]);

        smac_lo = (ep_info->src_mac_address[2] << 24) +
                  (ep_info->src_mac_address[3] << 16) +
                  (ep_info->src_mac_address[4] << 8) +
                  (ep_info->src_mac_address[5]);

        pushdown_lo_26b = pushdown_lo >> 6;
        smac_lo_26b     = smac_lo >> 6;
        pushdown_lo_6b = pushdown_lo & 0x3f;
        smac_lo_6b     = smac_lo & 0x3f;

        pushdown_aux_lo_26b = pushdown_aux_lo >> 6;
        pushdown_aux_lo_6b = pushdown_aux_lo & 0x3f;

        /* if a MEP, then add the SMAC address. */
        if (!(ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) {
            if (ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING) {
                /* up MEPs use the push-down to write the SMAC into
                 * the transmit packet.  -see lengthy comment below...
                 */
                if (_state[unit]->isSmacPri == FALSE) {
                    /* can only set this push-down once.  it is possible
                     * (if more code is writen) to reset the isSmacSet
                     * flag so that the push-down could be changed.
                     */
                    _state[unit]->isSmacPri = TRUE;
                    rv = soc_sbx_g2p3_oam_sa_2hi_set(unit, smac_hi);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to set smac (hi 2 bytes) for up MEP %d\n"),
                               ep_info->id));
                    }
                    rv = soc_sbx_g2p3_oam_sa_4lo_set(unit, smac_lo);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to set smac (lo 4 bytes) for up MEP %d\n"),
                               ep_info->id));
                    }
                    pushdown_lo = smac_lo;
                    pushdown_lo_26b = pushdown_lo >> 6;
                    pushdown_lo_6b = pushdown_lo & 0x3f;
                } else {
                    /* new smacs must be within 64 of the smallest smac configured.
                     * allow the 64 entry smac range to "wrap-around" if the range
                     * starts at a large LSB.  the carry-bit could wrap several bytes
                     * if there are 0xFF bytes within the smac.
                     */
                  if (pushdown_hi == smac_hi) {
                    /* upper 2 bytes the same. */
                    if (pushdown_lo_26b == smac_lo_26b) {
                      if (pushdown_lo_6b > smac_lo_6b) {
                        /* first smac configured (pushdown) must be smallest. */
                        rv = BCM_E_PARAM;
                      }
                    } else if (pushdown_lo_26b+1 == smac_lo_26b) {
                      /* lower 4 bytes have wrapped. */
                      if (smac_lo_6b >= pushdown_lo_6b) {
                        /* must be within the 64 entry range. */
                        rv = BCM_E_PARAM;
                      }
                    } else {
                      /* not even close. */
                      rv = BCM_E_PARAM;
                    }
                  } else if (pushdown_hi+1 == smac_hi) {
                    /* upper 2 bytes have wrapped. */
                    if (pushdown_lo_26b == smac_lo_26b) {
                      /* not even close. */
                      rv = BCM_E_PARAM;
                    } else if ((pushdown_lo_26b+1) == (smac_lo_26b+(1<<26))) {
                      if (smac_lo_6b >= pushdown_lo_6b) {
                        /* must be within the 64 entry range. */
                        rv = BCM_E_PARAM;
                      }
                    } else {
                      /* not even close. */
                      rv = BCM_E_PARAM;
                    }
                  } else {
                    /* not even close. */
                    rv = BCM_E_PARAM;
                  }
                }

                if (BCM_FAILURE(rv)) {
                    is_smacAux = 1;
                    rv = BCM_E_NONE;
                    if (_state[unit]->isSmacAux == FALSE) {
                      /* can only set this push-down once.  it is possible
                       * (if more code is writen) to reset the isSmacSet
                       * flag so that the push-down could be changed.
                       */
                      _state[unit]->isSmacAux = TRUE;

                      rv = soc_sbx_g2p3_oam_sa_2hi_aux_set(unit, smac_hi);
                      if (BCM_FAILURE(rv)) {
                          OAM_ERR((_SBX_D(unit,"Failed to set secondary smac (hi 2 bytes) for up MEP %d\n"),
                                 ep_info->id));
                      }

                      rv = soc_sbx_g2p3_oam_sa_4lo_aux_set(unit, smac_lo);
                      if (BCM_FAILURE(rv)) {
                          OAM_ERR((_SBX_D(unit,"Failed to set secondary smac (lo 4 bytes) for up MEP %d\n"),
                                 ep_info->id));
                      }
                      pushdown_aux_lo = smac_lo;
                      pushdown_aux_lo_26b = pushdown_aux_lo >> 6;
                      pushdown_aux_lo_6b = pushdown_aux_lo & 0x3f;
                    } else {
                        /* new smacs must be within 64 of the smallest smac configured.
                         * allow the 64 entry smac range to "wrap-around" if the range
                         * starts at a large LSB.  the carry-bit could wrap several bytes
                         * if there are 0xFF bytes within the smac.
                         */
                      if (pushdown_aux_hi == smac_hi) {
                        /* upper 2 bytes the same. */
                        if (pushdown_aux_lo_26b == smac_lo_26b) {
                          if (pushdown_aux_lo_6b > smac_lo_6b) {
                            /* first smac configured (pushdown) must be smallest. */
                            rv = BCM_E_PARAM;
                          }
                        } else if (pushdown_aux_lo_26b+1 == smac_lo_26b) {
                          /* lower 4 bytes have wrapped. */
                          if (smac_lo_6b >= pushdown_aux_lo_6b) {
                            /* must be within the 64 entry range. */
                            rv = BCM_E_PARAM;
                          }
                        } else {
                          /* not even close. */
                          rv = BCM_E_PARAM;
                        }
                      } else if (pushdown_aux_hi+1 == smac_hi) {
                        /* upper 2 bytes have wrapped. */
                        if (pushdown_aux_lo_26b == smac_lo_26b) {
                          /* not even close. */
                          rv = BCM_E_PARAM;
                        } else if ((pushdown_aux_lo_26b+1) == (smac_lo_26b+(1<<26))) {
                          if (smac_lo_6b >= pushdown_aux_lo_6b) {
                            /* must be within the 64 entry range. */
                            rv = BCM_E_PARAM;
                          }
                        } else {
                          /* not even close. */
                          rv = BCM_E_PARAM;
                        }
                      } else {
                        /* not even close. */
                        rv = BCM_E_PARAM;
                      }
                  }

                  if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Invalid SMAC.  Trying to configure: 0x%04x%08x "
                                    "Pushdown already configured: 0x%04x%08x\n"),
                                    smac_hi, smac_lo, pushdown_hi, pushdown_lo));
                  }
                }
            } else {
                /* down MEPs use the egress SMAC table to write the SMAC
                 * into the transmit packet.
                 */
                rv = _oam_egr_path_smac_set(unit, egrPath, ep_info->src_mac_address);
                if (BCM_FAILURE(rv)) {
                    OAM_ERR((_SBX_D(unit, "Failed to set smac on egress path for EP %d\n"),
                             ep_info->id));
                }
            }
        }
        if (BCM_SUCCESS(rv) && ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING) {
            bcm_mac_t  oamupmask = {0xff,0xff,0xff,0xff,0xff,0xe0};
            int smacIndex = (is_smacAux * 2 ) + ((ep_info->src_mac_address[5] & 0x20) >> 5);
            sal_memcpy(oamupmac.mac, ep_info->src_mac_address, sizeof(bcm_mac_t));
            sal_memcpy(oamupmac.mask, oamupmask, sizeof(bcm_mac_t));
            rv = soc_sbx_g2p3_oamupmac_set(unit, smacIndex, &oamupmac);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "unable to set up mac at index %01X.  Error:  %d (%s)\n"),
                         smacIndex, rv, bcm_errmsg(rv)));
                return rv;
            } else {
                OAM_VERB((_SBX_D(unit, "Stored up LSM MAC " L2_6B_MAC_FMT " at idx %d\n"),
                          L2_6B_MAC_PFMT(ep_info->src_mac_address), smacIndex));
            }

            /* get the encap ETE */
            rv = soc_sbx_g2p3_eteencap_get(unit, _state[unit]->eteencap_lsm, &eteEncap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "unable to read ete[%08X]Encap: %d (%s)\n"),
                         _state[unit]->eteencap_lsm, rv, bcm_errmsg(rv)));
                return rv;
            }

          /* These ETEs are accessed from the Port2FTIdx table only.
           * They are used for packets that missed the up MEP lookup.
           *   IMPORTANT
           * These packets would not have even gone through the up MEP
           * lookup unless the packet's original DMAC was IEEE (CCM)
           * or one of the local box's LSM (PM or MIP).
           *
           * The up MEP lookup (mirror to ingress) path is only taken
           * for OAM packets addressed to the local station!  either
           * IEEE dmac (CCM) or an LSM (LM/DM).  When CCM messages
           * miss the up mep lookup we forward them on (P2FTI IEEE).
           * when LM/DM or MIP miss the up mep lookup we forward them
           * on (P2FTIdx LSM) restoring the original DMAC (the local LSM).
           *
           * Here we set the local endpoint's smac (LSM) into the ETE's
           * dmac because the original packet's dmac was the LSM (or IEEE
           * but that is the other P2FTI -> ETE chain).
           */
            eteEncap.dmac0 = ep_info->src_mac_address[0];
            eteEncap.dmac1 = ep_info->src_mac_address[1];
            eteEncap.dmac2 = ep_info->src_mac_address[2];
            eteEncap.dmac3 = ep_info->src_mac_address[3];
            eteEncap.dmac4 = ep_info->src_mac_address[4];
  
            /* write the encap ETE */
            rv = soc_sbx_g2p3_eteencap_set(unit, _state[unit]->eteencap_lsm, &eteEncap);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "unable to write ete[%08X]Encap: %d (%s)\n"),
                         _state[unit]->eteencap_lsm, rv, bcm_errmsg(rv)));
                return rv;
            }

            /* Endpoints can be configured to use any one of 64 unique
             * SMACs.  For down MEPs, these SMACs are stored in the
             * egress SMAC table for TX, and in the PPE's LSM rules
             * for RX.  For up MEPs, these SMACs must all be from a 
             * contiguous range of 64 addresses.  The above code
             * to set the smac mask for up MEPs allows any SMAC
             * within that 64 address range to trigger PPE LSM.  For
             * transmitting up MEP packets, the TX ucode needs to know
             * the 6 bits uniquely identifying the SMAC.  the smacoffset
             * is taking the least significant byte of SMAC and storing
             * it in the endpoint - it takes the entire byte because
             * the ucode cant write anything less than one byte.
             */
            /* first check if there was a "wrap-around", if so set the
             * endpoint entry's ADD flag. */
            if (is_smacAux == 0) {
              if (pushdown_lo_26b != smac_lo_26b) {
                /* the smac range extends beyond the sixth bit boundary.
                 * tell the ucode to perform an add instead of replace.
                 * subtract the push-down from the new smac, the ucode 
                 * will add the difference to the pushdown.
                 */
                /* make the new smac 6bit range larger (for subtraction)
                 * by or'ing in 1 << 6.
                 */
                smac_lo_6b = smac_lo_6b + (1 << 6);
                oamep2.smacoffset = smac_lo_6b - pushdown_lo_6b;
                oamep2.smacadd = 1;
                oamep2.smacaux = 0;
              } else {
                oamep2.smacoffset = ep_info->src_mac_address[5];
                oamep2.smacadd = 0;
                oamep2.smacaux = 0;
              }
            } else {
              if (pushdown_aux_lo_26b != smac_lo_26b) {
                smac_lo_6b = smac_lo_6b + (1 << 6);
                oamep2.smacoffset = smac_lo_6b - pushdown_aux_lo_6b;
                oamep2.smacadd = 1;
                oamep2.smacaux = 1;
              } else {
                oamep2.smacoffset = ep_info->src_mac_address[5];
                oamep2.smacadd = 0;
                oamep2.smacaux = 1;
              }
            }
        } /* end if up facing */

        if (BCM_SUCCESS(rv)) {
            /* the interval in the ep_info->ccm_period field is an actual time,
             * it is not an enumeration representing a time.  This function 
             * returns the enum corresponding to the interval.
             */
            rv = _bcm_tcal_interval_encode(unit, ep_info->ccm_period, &oamep.interval);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "oam interval %d encode failed: %d %s\n"), 
                         ep_info->ccm_period, rv, bcm_errmsg(rv)));
            }
        }

        if (BCM_SUCCESS(rv)) {
            if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
                /* A second entry has already been alocated, use the same one. */
                /* so long as REPLACE is set, it doesnt matter if WITH-ID is set or not,
                 * because the ID can not change for the first or second entry.
                 */
                ep_rec_index2 = oamep.nextentry;
            } else {
                /* 48 byte MAID requires a second oamep, allocate it now. */
                /* if REPLACE flag is clear, and WITH_ID flag is set, then the 
                 * second entry's ID has already been reserved and it is the
                 * first entry's ID + 1.
                 */
                if (ep_info->flags & BCM_OAM_ENDPOINT_WITH_ID) {
                    ep_rec_index2 = ep_rec_index + 1;
                } else {
                    rv = shr_idxres_list_alloc(_state[unit]->ep_pool, &ep_rec_index2);
                    if (BCM_FAILURE(rv)) {
                        OAM_ERR((_SBX_D(unit, "Failed to allocate an endpoint record: %d %s\n"),
                                 rv, bcm_errmsg(rv)));
                    }
                }
            }
        }

        if (BCM_SUCCESS(rv)) {
            oamep.nextentry = ep_rec_index2;

            /* the MEG ID field in the CCM is encoded by the application. 
             * all 48 will be written into the packet during transmit,
             * and all 48 will be read on reception.  however, the compare
             * on receive uses a hash over the entire MAID rather than
             * comparing each byte.
             */
            oamep.maidw0   = MAID_ID_TO_ZF(&group_info->name[0]);
            oamep.maidw1   = MAID_ID_TO_ZF(&group_info->name[4]);
            oamep.maidw2   = MAID_ID_TO_ZF(&group_info->name[8]);
            oamep.maidw3   = MAID_ID_TO_ZF(&group_info->name[12]);
            oamep.maidw4   = MAID_ID_TO_ZF(&group_info->name[16]);
            oamep.maidw5   = MAID_ID_TO_ZF(&group_info->name[20]);

            /* sal_memcpy(&oamep2, &group_info->name[16], sizeof(oamep2)); */

            oamep2.maidw6  = MAID_ID_TO_ZF(&group_info->name[24]);
            oamep2.maidw7  = MAID_ID_TO_ZF(&group_info->name[28]);
            oamep2.maidw8  = MAID_ID_TO_ZF(&group_info->name[32]);
            oamep2.maidw9  = MAID_ID_TO_ZF(&group_info->name[36]);
            oamep2.maidw10 = MAID_ID_TO_ZF(&group_info->name[40]);
            oamep2.maidw11 = MAID_ID_TO_ZF(&group_info->name[44]);

            oamep2.function = _state[unit]->ep_type_ids.ccm_second;
            oamep2.type     = 1; /* Ethernet */

            /* the soc layer doesn't handle overlays very well.
             * Must clear all fields not related to this type to ensure
             * bits are set as expected
             */
            _oam_oamep_prepare(unit, &oamep2);

            rv = soc_sbx_g2p3_oamep_set(unit, ep_rec_index2, &oamep2);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to write the second oamep 0x%x: %d %s\n"),
                         ep_rec_index2, rv, bcm_errmsg(rv)));
            }
        }
    } /* end LOCAL entry */


    /* the remaining calls are common to local and peer/remote endpoint
     * configuration
     */
    if (BCM_SUCCESS(rv)) {

        /* the soc layer doesn't handle overlays very well.
         * Must clear all fields not related to this type to ensure
         * bits are set as expected
         */
        _oam_oamep_prepare(unit, &oamep);

        rv = soc_sbx_g2p3_oamep_set(unit, ep_rec_index, &oamep);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to write oamep 0x%x: %d %s\n"),
                     ep_rec_index, rv, bcm_errmsg(rv)));
        } else {
          /* we only care about whether we wrote a PEER record because of
           * the policer watch-dog timer ID (only present in the PEER entry).
           */
          if (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {
            peer_record_written++;
          }
        }
    }

    if (BCM_SUCCESS(rv) && 
        !(ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) &&
        !(ep_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) {

        /* Set up the timer calendar to trigger the transmission of
         * OAM packets.
         * ! the endpoint entry MUST be added before these tcal entries        !
         * ! because they (tcal) will trigger the start of packet transmission !
         * ! which reads the endpoint entries.  order matters.                 !
         */

        int tx_enable = !!(ep_info->flags & _SBX_OAM_CCM_TX_ENABLE);

        if (ep_info->flags & BCM_OAM_ENDPOINT_REPLACE) {
            *tcal_id = _state[unit]->hash_data_store[ep_rec_index].tcal_id;
            rv = _bcm_tcal_update(unit, ep_info->ccm_period, tx_enable,
                                  ep_rec_index, tcal_id);
        } else {
            rv = _bcm_tcal_alloc(unit, ep_info->ccm_period, tx_enable,
                                 ep_rec_index, tcal_id);
        }

        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to write/update timer calendar 0x%x: %d %s\n"),
                     *tcal_id, rv, bcm_errmsg(rv)));
        }
    }

    if (BCM_FAILURE(rv)) {
        int tmpRv;
        OAM_ERR((_SBX_D(unit, "failed: err=%d %s\n"), rv, bcm_errmsg(rv)));
        OAM_DEBUG_EP(BCM_DBG_ERR, ep_info);
 
        if (mamep_committed) {
            OAM_VERB((_SBX_D(unit, "removing oam maidmep2e lookup\n")));
            tmpRv = soc_sbx_g2p3_oammaidmep2e_delete(unit, ep_info->local_id,
                                             ep_info->name);
            if (BCM_FAILURE(tmpRv)) {
                OAM_ERR((_SBX_D(unit, "Failed to write/update endpoint entry.  Then failed "
                                      "to back-out the maid-mepid table entry: %d %s\n"),
                         tmpRv, bcm_errmsg(tmpRv)));
            }
        }
            
        if (oampv2e_committed) {
            OAM_VERB((_SBX_D(unit, "removing oam pvd2e lookup\n")));
            tmpRv = soc_sbx_g2p3_oampvd2e_delete(unit, port, ep_info->vlan,
                                         !!(ep_info->flags & BCM_OAM_ENDPOINT_UP_FACING), 0);
            if (BCM_FAILURE(tmpRv)) {
                OAM_ERR((_SBX_D(unit, "Failed to write/update endpoint entry.  Then failed "
                                      "to back-out the PVD table entry: %d %s\n"),
                         tmpRv, bcm_errmsg(tmpRv)));
            }
        }
            
        if (peer_record_written) {
            /* remove policer from db record on error after it's been commmited to
             * avoid resource leaks and pointers to free'd policers
             */
            OAM_VERB((_SBX_D(unit, "clearing timers in peer/remote oamep\n")));
            oamep.policerid = INVALID_POLICER_ID;
            tmpRv = soc_sbx_g2p3_oamep_set(unit, ep_rec_index, &oamep);
        }
        
        if (watchdog_id != INVALID_POLICER_ID) {
            OAM_VERB((_SBX_D(unit, "freeing watchdog timer\n")));
            _oam_timer_free(unit, &watchdog_id);
        }
        
    }

    return rv;
}

int _oam_enet_endpoint_delete(int unit, bcm_oam_endpoint_info_t *ep_info,
                              bcm_trunk_add_info_t *trunk_info)
{
    int rv = BCM_E_NONE;
    int rec_idx, secondEntry=0;
    /* uint32 seqnum_generator_id = INVALID_POLICER_ID; */
    uint32 watchdog_id            = INVALID_POLICER_ID;
    bcm_port_t                         port;
    bcm_module_t                       mod;
    soc_sbx_g2p3_oamep_t               oamep;
    oam_sw_hash_data_t                *hash_data;
    
    /* sanity check pointers */
    if ((ep_info==NULL) || (trunk_info==NULL)) {
        return BCM_E_INTERNAL;
    }

    rec_idx = ep_info->id;

    if (!ENDPOINT_ID_VALID(unit, ep_info->id)) {
        OAM_ERR((_SBX_D(unit, "Invalid endpoint found: 0x%04x\n"), ep_info->id));
        return BCM_E_PARAM;
    }

    hash_data = &_state[unit]->hash_data_store[ep_info->id];
    if (hash_data->delay_idx != INVALID_RECORD_INDEX) {
        OAM_ERR((_SBX_D(unit, "Must remove delay measurement before"
                        "deleting endpoint\n")));
        return BCM_E_BUSY;
    }

    if (hash_data->loss_idx != INVALID_RECORD_INDEX) {
        OAM_ERR((_SBX_D(unit, "Must remove loss measurement before"
                        "deleting endpoint\n")));
        return BCM_E_BUSY;
    }

    /* check if we're configuring this endpoint on a LAG. */
    if (BCM_GPORT_IS_TRUNK(ep_info->gport)) {
        /* select the "designate" port/mod */
        mod  = trunk_info->tm[0];
        port = trunk_info->tp[0];

    } else if (BCM_GPORT_IS_MODPORT(ep_info->gport)) {
        /* not a trunk, but fill in the trunk_add_info structure
           anyways, as a common representation for the port(s). */
        mod  = BCM_GPORT_MODPORT_MODID_GET(ep_info->gport);
        port = BCM_GPORT_MODPORT_PORT_GET(ep_info->gport);
        trunk_info->num_ports = 1;
        trunk_info->tp[0] = port;
        trunk_info->tm[0] = mod;
    } else {
        return BCM_E_PARAM;
    }

    /* Get the sequence number generator and watchdog id
     */
    rv = soc_sbx_g2p3_oamep_get(unit, rec_idx, &oamep);
    if (BCM_FAILURE(rv)) {
        /* non-fatal error, keep trying */
        OAM_ERR((_SBX_D(unit, "Failed to find EP record idx=%d err=%d %s\n"),
                 rec_idx, rv, bcm_errmsg(rv)));
    }

    if (ep_info->flags & BCM_OAM_ENDPOINT_REMOTE) {
        watchdog_id = oamep.policerid;
        secondEntry = (uint32_t) (soc_sbx_g2p3_oamep_t*) NULL;

        /* remove the MAID/MEPID cuckoo entries associated with this remove endpoint. */
        rv = _oam_enet_remove_rx_peer(unit, hash_data->maid_cookie, hash_data->ep_name);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to remove receive-side table entries: %d %s\n"),
                     rv, bcm_errmsg(rv)));
        }

    } else {        /* LOCAL endpoint entry */
        uint32 unknownIdx = ~0;
        watchdog_id = INVALID_POLICER_ID;
        secondEntry = oamep.nextentry;

        /* this is Enet CCM, so there should be a second local entry to hold the MAID,
         * clear the second local entry and release the ID.
         */
        if (secondEntry > 0) {
            soc_sbx_g2p3_oamep_t_init(&oamep);
            rv = soc_sbx_g2p3_oamep_set(unit, secondEntry, &oamep);
            if (BCM_FAILURE(rv)) {
                OAM_ERR((_SBX_D(unit, "Failed to clear the second EP record idx=%d err=%d %s\n"),
                         secondEntry, rv, bcm_errmsg(rv)));
            }
            shr_idxres_list_free(_state[unit]->ep_pool, secondEntry);
        } else {
            OAM_ERR((_SBX_D(unit, "Failed to find second local entry for epidx=%d \n"),
                     rec_idx));
        }

        /* remove the PVD, PMD, and remap entries associated with this local endpoint. */
        rv = _oam_enet_remove_rx_local(unit, ep_info, trunk_info);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to remove receive-side table entries: %d %s\n"),
                     rv, bcm_errmsg(rv)));
        }

        /* remove the LSM entry */
        rv = _sbx_gu2_ismac_idx_free(unit, ep_info->src_mac_address, 
                                     _SBX_GU2_RES_UNUSED_PORT, &unknownIdx);
        if (rv == BCM_E_EMPTY) {
            soc_sbx_g2p3_lsmac_t lsm;
            soc_sbx_g2p3_lsmac_t_init (&lsm);

            OAM_VERB((_SBX_D(unit, "Found no more users of idx %d, clearing hw table\n"),
                      unknownIdx));
            rv = soc_sbx_g2p3_lsmac_set (unit, unknownIdx, &lsm);

            if (BCM_FAILURE(rv)) {
                OAM_WARN((_SBX_D(unit, "Failed to delete local station MAC: %d %s\n"),
                          rv, bcm_errmsg(rv)));
            }
        }
    }

    /* 'clear' the entry */
    soc_sbx_g2p3_oamep_t_init(&oamep);
    rv = soc_sbx_g2p3_oamep_set(unit, rec_idx, &oamep);

    if (watchdog_id != INVALID_POLICER_ID ) {
        _oam_timer_free(unit, &watchdog_id);
    }

    return rv;
}

int _bcm_fe2000_validate_oam_eth_endpoint(int unit, 
                                          bcm_oam_endpoint_info_t *endpoint_info)
{
    int status = BCM_E_PARAM;

    if(!endpoint_info) {
        OAM_ERR((_SBX_D(unit, "Null endpoint info\n")));

    } else if (endpoint_info->ccm_period > OAM_SBX_MAX_PERIOD) {
        OAM_ERR((_SBX_D(unit, "Maximim period supported by this device is %dms\n"),
                 OAM_SBX_MAX_PERIOD));

    } else if ((endpoint_info->flags & _SBX_OAM_CCM_TX_ENABLE) &&
               (endpoint_info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE)) {
        OAM_ERR((_SBX_D(unit, "Invalid flags (MIP and TX): 0x%08x \n"),
                 endpoint_info->flags));
    } else {
        status = BCM_E_NONE;
    }
    return status;
}

STATIC int 
_oam_up_miss_egr_path_init(int unit, int dmac_type)
{
    int rv = BCM_E_NONE;
    bcm_module_t mod;
    bcm_port_t   port;
    int fabUnit, fabNode, fabPort;
    pbmp_t pbmp;

    uint32_t    ftIdx = 0;
    uint32_t    eteL2Idx = 0;
    uint32_t    eteEncapIdx = 0;
    uint32_t    ohIdx = 0;
    
    soc_sbx_g2p3_p2fti_t    p2fti;
    soc_sbx_g2p3_ft_t       fte;
    soc_sbx_g2p3_etel2_t    eteL2;
    soc_sbx_g2p3_eteencap_t eteEncap;
    soc_sbx_g2p3_oi2e_t     oi2e;

    soc_sbx_g2p3_p2fti_t_init(&p2fti);
    soc_sbx_g2p3_ft_t_init(&fte);
    soc_sbx_g2p3_etel2_t_init(&eteL2);
    soc_sbx_g2p3_eteencap_t_init(&eteEncap);
    soc_sbx_g2p3_oi2e_t_init(&oi2e);

    /* alocate raw ETE (OHI, Encap, & L2 */
    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_OHI, 1,
                                 &ohIdx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate ohi: %d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_ENCAP, 1,
                                 &eteEncapIdx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate raw ete encap: %d %s\n"),
                 rv, bcm_errmsg(rv)));
        return rv;
    }

    /* save oam lsm eteencap index */
    if (dmac_type == 0) {
        _state[unit]->eteencap_lsm = eteEncapIdx;
    }

    rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_L2, 1,
                                 &eteL2Idx, 0);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to allocate ete l2: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Configure the ETE for basic pass-through egress handling.
     * The one thing to note is that setting the vid to 0xfff will
     * cause no vlan header to be appended.
     */
    eteEncap.l2ete = eteL2Idx;
    eteEncap.ipttldec = FALSE;
    eteEncap.ttlcheck = FALSE;
    eteEncap.nosplitcheck = TRUE;
    eteEncap.dmacset = TRUE;
    eteEncap.dmacsetlsb = FALSE;
    eteEncap.dmac0 = 0x01;
    eteEncap.dmac1 = 0x80;
    eteEncap.dmac2 = 0xC2;
    eteEncap.dmac3 = 0x00;
    eteEncap.dmac4 = 0x00;
    eteEncap.smacset = FALSE;
    eteEncap.nostrip = FALSE;

    eteL2.mtu = SBX_DEFAULT_MTU_SIZE;
    eteL2.nostrip = FALSE;
    eteL2.nosplitcheck = TRUE;
    eteL2.usetag = TRUE;
    eteL2.usevid = TRUE;
    eteL2.vid = _BCM_VLAN_G2P3_UNTAGGED_VID;

    oi2e.eteptr = eteEncapIdx;

    /* write the L2 ETE */
    rv = soc_sbx_g2p3_etel2_set(unit, eteL2Idx, &eteL2);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write ete[0x%08X]L2: %d (%s)\n"),
                 eteL2Idx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* write the encap ETE */
    rv = soc_sbx_g2p3_eteencap_set(unit, eteEncapIdx, &eteEncap);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write ete[0x%08X]Encap: %d (%s)\n"),
                 eteEncapIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* write the oi2e */
    rv = soc_sbx_g2p3_oi2e_set(unit, ohIdx - SBX_RAW_OHI_BASE, &oi2e);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "unable to write oi2e[0x%08X]: %d (%s)\n"),
                 ohIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* get mod from unit */
    rv = bcm_stk_my_modid_get(unit, &mod);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get modid: %d %s\n"), 
                 rv, bcm_errmsg(rv)));
        return rv;
    }

    /* populate the per-port forwarding entries. */
    SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
    SOC_PBMP_PORT_REMOVE(pbmp, SOC_SBX_OAM_SPI_LB_PORT(unit));
    SOC_PBMP_ITER(pbmp, port) {

        /* get the fab node/port from the dest node/port */
        rv = soc_sbx_node_port_get(unit, mod, port,
                                   &fabUnit, &fabNode, &fabPort);

        if (BCM_FAILURE(rv)) {
            /* failed to get target information */
            OAM_WARN((_SBX_D(unit, "soc_sbx_node_port_get(%d,%d,%d,&(%d),&(%d),&(%d)) "
                             "returned %d (%s)\n"),
                      unit, mod, port, fabUnit, fabNode, fabPort,
                      rv, bcm_errmsg(rv)));
            /* this port is invalid.  thats fine.  try the next port. */
            rv = BCM_E_NONE;
            continue;
        }

        /* alocate FTI & FTE, populate p2fti & FTE */
        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_FTE_UNICAST, 1,
                                     &ftIdx, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to alloc FTE: %d %s\n"), rv, bcm_errmsg(rv)));
            return rv;
        }

        /* get the qid from the fab node/port */
        fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,fabNode, fabPort, NUM_COS(unit));
        fte.oi = ohIdx;
        rv = soc_sbx_g2p3_ft_set(unit, ftIdx, &fte);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to write ft[%08X]: %d (%s)\n"),
                     ftIdx, rv, bcm_errmsg(rv)));
            return rv;
        }

        p2fti.ftidx = ftIdx;
        rv = soc_sbx_g2p3_p2fti_set(unit, port, dmac_type, &p2fti);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "unable to write p2fti[%08X][%d]: %d (%s)\n"),
                     port, dmac_type, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    return rv;
}


STATIC int _oam_set_initial_queue_state(int unit)
{
    int rv = BCM_E_NONE;
    sbhandle sbh = SOC_SBX_CONTROL(unit)->sbhdl;
#if OAM_MAC_LOOPBACK
    uint32_t uAddress = OAM_MAC_LB_QUEUE;
#else
    uint32_t uAddress = SOC_SBX_OAM_SPI_LB_QUEUE(unit);
#endif
    uint32_t uCode = OAM_IQS_CODE;
    sbZfFe2000PpInitialQueueStateEntry_t ca_iqs;
    sbZfC2PpInitialQueueStateEntry_t c2_iqs;

    /* Using MAC-loopback on port 23 or spi loopback on spi0.13 */
    /* sws-queue for port 23 is 0x1b. */
    /* the code for front-panel loopback is UPMEP (0x18). */
    /* sws-queue for spi loopback is 0x38 */
    /* the code for SPI loopback is UPMEP_ERH (0x1A). */

    if (SOC_IS_SBX_FE2KXT(unit)) {
        if (C2PpInitialQueueStateRead(sbh, uAddress, &c2_iqs) == TRUE) {
            c2_iqs.m_uState |= (uCode << 8);
#if !OAM_MAC_LOOPBACK
            c2_iqs.m_uHeaderType = 0;
#endif
            if (C2PpInitialQueueStateWrite(sbh, uAddress, &c2_iqs) == FALSE) {
                rv = BCM_E_INTERNAL;
            }
        } else {
            rv = BCM_E_INTERNAL;
        }
    } else {
        if (CaPpInitialQueueStateRead(sbh, uAddress, &ca_iqs) == TRUE ) {
            ca_iqs.m_uState |= (uCode << 8);
#if !OAM_MAC_LOOPBACK
            ca_iqs.m_uHeaderType = 0;
#endif
            if (CaPpInitialQueueStateWrite(sbh, uAddress, &ca_iqs) == FALSE) {
                rv = BCM_E_INTERNAL;
            }
        } else {
            rv = BCM_E_INTERNAL;
        }
    }

    return rv;
}

STATIC int _oam_egr_mirror_set(int unit)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_emirror20_t ca_emirror;
    soc_sbx_g2p3_emirror25_t c2_emirror;

#if OAM_MAC_LOOPBACK
    int hdrpresent = 0;
    int queue = OAM_MAC_LB_QUEUE + OAM_QUEUE_OFFSET;
#else
    int hdrpresent = 1;
    int queue = SOC_SBX_OAM_SPI_LB_QUEUE(unit) + OAM_QUEUE_OFFSET;
#endif

    soc_sbx_g2p3_emirror20_t_init(&ca_emirror);
    soc_sbx_g2p3_emirror25_t_init(&c2_emirror);

    if (!(SOC_IS_SBX_FE2KXT(unit))) {
        ca_emirror.queue = queue;
        ca_emirror.hdrpresent = hdrpresent;
        ca_emirror.dropsrcpkt = 1;
        rv = soc_sbx_g2p3_emirror20_set(unit, OAM_MIRROR_ID, &ca_emirror);
    } else {
        c2_emirror.queue = queue;
        c2_emirror.hdrpresent = hdrpresent;
        c2_emirror.dropsrcpkt = 1;
        rv = soc_sbx_g2p3_emirror25_set(unit, OAM_MIRROR_ID, &c2_emirror);
    }

    return rv;
}


int
_oam_set_qid(int unit) {
    int rv = BCM_E_NONE;
    bcm_module_t mod;
    bcm_port_t   port;   
    int          fabUnit, fabNode, fabPort;
    unsigned int qid;
 
    /* get mod from unit */
    rv = bcm_stk_my_modid_get(unit, &mod);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get mod from unit %d: %d %s\n"),
                 unit, rv, bcm_errmsg(rv)));
        return rv;
    }

    port = SOC_SBX_OAM_SPI_LB_PORT(unit);

    /* get the fab node/port from the dest node/port */
    rv = soc_sbx_node_port_get(unit, mod, port,
                               &fabUnit, &fabNode, &fabPort);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get fab unit/node/port from unit %d, mod %d, port %d:\n %d %s\n"),
                 unit, mod, port, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* get the qid from the fab node/port */
    qid = SOC_SBX_NODE_PORT_TO_QID(unit, fabNode, fabPort, NUM_COS(unit));
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to push-down loopback qid %d on unit %d: %d %s\n"),
                 qid, unit, rv, bcm_errmsg(rv)));
    }

    rv = soc_sbx_g2p3_oam_loopback_qid_set(unit, qid);

    OAM_VERB((_SBX_D(unit, "QID for OAM SPI Lookback (Mirror) port 0x%x\n"), qid));

    /* Now, set the "Same-FE" QID used to forward PM packets to an
     * egress stream (egr-oam-pm) on the same FE for Co-Co or RTC
     * updates.  This QID can be for ANY port as long as it is on
     * the same FE as the local up MEP.  This should be set for
     * each FE in the system.
     */
    port = 0;

    /* get the fab node/port from the dest node/port */
    rv = soc_sbx_node_port_get(unit, mod, port,
                               &fabUnit, &fabNode, &fabPort);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get fab unit/node/port from unit %d, mod %d, port %d:\n %d %s\n"),
                 unit, mod, port, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* get the qid from the fab node/port */
    qid = SOC_SBX_NODE_PORT_TO_QID(unit, fabNode, fabPort, NUM_COS(unit));
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to push-down same-FE qid %d on unit %d: %d %s\n"),
                 qid, unit, rv, bcm_errmsg(rv)));
    }

    rv = soc_sbx_g2p3_oam_local_fe_qid_set(unit, qid);

    OAM_VERB((_SBX_D(unit, "QID for OAM up MEP PM (same FE) port 0x%x\n"), qid));

    return rv;
}

int _bcm_fe2000_init_upmep(int unit)
{
    int rv = BCM_E_NONE;

    if (_state[unit]->up_init == 0) {
        _state[unit]->up_init = 1;

        /* configure the up-MEP egress path, including the p2fti and
         * the two port-based FTEs: 0=lsm, 1=ieee address */
        rv = _oam_up_miss_egr_path_init(unit, 0);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to configure up-MEP egress path (lsm): %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }
        rv = _oam_up_miss_egr_path_init(unit, 1);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to configure up-MEP egress path (ieee): %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }

        /* set a flag for the ingress PPE. */
        rv = _oam_set_initial_queue_state(unit);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to set initial queue state: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }

#if OAM_MAC_LOOPBACK
        /* enable mac loopback on font-panel port 23 */
        rv = bcm_port_loopback_set(unit, OAM_MAC_LB_PORT, BCM_PORT_LOOPBACK_MAC);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to set MAC loopback on port 23: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }
#else
        /* get the QID used in the SPI loopback.
         * then set the pushdown value. */
        rv = _oam_set_qid(unit);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to set QID pushdown: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }
#endif

        /* configure egress mirror 3 for oam */
        rv = _oam_egr_mirror_set(unit);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to enable egress mirror: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        }

        /* turn on the egress PPE rule that directs TB OAM packets to
         * the egress oam mirror stream. */
        rv = soc_sbx_g2p3_pp_oam_up_match_set(unit,1);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to enable egress PPE OAM rules: %d %s\n"),
                     rv, bcm_errmsg(rv)));
            return rv;
        } else {
            OAM_VERB((_SBX_D(unit, "egr PPE rules have been turned on\n")));
        }
    }

    return rv;
}

/*
 *   Function
 *      _oam_g2p3_coco_configure
 *   Purpose
 *      Set the g2p3 coherent counters in the LP, evp2e and oi2e 
 *      of the given endpoint
 *   Parameters
 *       unit        = BCM device number
 *       ep_data     = internal data representing port to count frames
 *       coco_idx    = coherent counter index
 *   Returns
 *       BCM_E_*
 */
int _bcm_fe2000_oam_enet_coco_configure(int unit,
                                        oam_sw_hash_data_t *ep_data, 
                                        int coco_idx,
                                        uint8_t pkt_pri_bitmap,
                                        int single_ended)
{
    egr_path_desc_t egrPath;
    soc_sbx_g2p3_lp_t lp;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_evp2e_t evp2e;
    soc_sbx_g2p3_ft_t fte;
    bcm_gport_t vlan_gport;
    int vid, lpi, rec_idx;
    bcm_port_t port;
    _oam_trunk_data_t    *trunk_data = NULL;
    bcm_trunk_add_info_t *trunk_info = NULL;
    bcm_trunk_t tid = 0;
    int rv;
    int priority_class = 0;
    int all_priorities = 0;
    int priority = 0;
    int level = 0;

    /* ep_data is the hash-data for the CCM endpoint that this LM service
     * is attached to.
     */

    soc_sbx_g2p3_ft_t_init(&fte);

    if (BCM_GPORT_IS_TRUNK(ep_data->gport)) {
        tid = BCM_GPORT_TRUNK_GET(ep_data->gport);

        if (OAM_TID_VALID(tid) == FALSE) {
            OAM_ERR((_SBX_D(unit, "Invalid TrunkId: %d\n"), tid));
        }

        trunk_data = &(_state[unit]->trunk_data[tid]);
        trunk_info = &(trunk_data->add_info);

        /* select the "designate" port/mod */
        /* mod  = trunk_info->tm[0]; */
        port = trunk_info->tp[0];
    } else if (BCM_GPORT_IS_MODPORT(ep_data->gport)) {
        port = BCM_GPORT_MODPORT_PORT_GET(ep_data->gport);
    } else {
        return BCM_E_PARAM;
    }

    vid = ep_data->vid_label;

    /* use the LM's EgrPath for single-ended,
     * and the CCM's EgrPath for dual-ended.
     */
    if (single_ended == TRUE ) {
        rec_idx = ep_data->loss_idx;
    } else {
        rec_idx = ep_data->rec_idx;
    }

    rv = _oam_egr_path_get(unit, &egrPath, rec_idx);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to get egress path for oamep 0x%04x: %d %s\n"),
                 rec_idx, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    OAM_DEBUG((_SBX_D(unit, "Getting PV2E with port=%d, vid=0x%03x\n"),
               port, vid));

    rv = soc_sbx_g2p3_pv2e_get(unit, vid, port, &pv2e);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read pv2e[%d, 0x%03x]: %d %s\n"),
                 port, vid, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    lpi = (pv2e.lpi) ? pv2e.lpi : port;

    OAM_DEBUG((_SBX_D(unit, "Getting LP with lpi=0x%08x\n"),lpi));

    rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read lp[0x%03x]: %d %s\n"),
                 lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    OAM_DEBUG((_SBX_D(unit, "Getting EVP2E with port=%d, vlan=0x%03x\n"),
               port, pv2e.vlan));

    rv = soc_sbx_g2p3_evp2e_get(unit, pv2e.vlan, port, &evp2e);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read evp2e[%d, 0x%03x]: %d %s\n"),
                 port, pv2e.vlan, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (ep_data->direction == OAM_DIR_DOWN) {
        lp.cocounter                           = OAM_COCO_RX(coco_idx);
        egrPath.oh.cocounter = evp2e.cocounter = OAM_COCO_TX(coco_idx);
    } else {
        /* up MEPs - reverse the RX and TX counters. */
        lp.cocounter                           = OAM_COCO_TX(coco_idx);
        egrPath.oh.cocounter = evp2e.cocounter = OAM_COCO_RX(coco_idx);
    }
 
    /* use the pkt_pri_bitmap array to set the configured packet priority
     * or if all priority levels are selected, then set the allpri flag.
     * if multiple (but not all) priority levels are set, use the last one.
     */
    all_priorities = TRUE;
    for (level=0; level < 8; level++) {
        priority = 1 << level;
        if (pkt_pri_bitmap & priority) {
            /* priority_class |= level; */
            /* the current ucode can only support one priority at a time.
             * so we will set to the highest priority set in the bitmap.
             * later, when the ucode supports multiple priorities, we
             * can remove this for loop entirely and simply copy the bitmap.
             */
            priority_class = level;
        } else {
            all_priorities = FALSE;
        }
    }
    /* if pkt_pri_bitmap is zero, then set all priority levels.
     * can also set each level (bit) in the pkt_pri_bitmap for same result.
     */
    all_priorities = (pkt_pri_bitmap == 0 ? 1 : all_priorities);
    lp.priclass = egrPath.oh.priclass = evp2e.priclass = priority_class;
    lp.allpri   = egrPath.oh.allpri   = evp2e.allpri   = all_priorities;

    rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write lp[0x%08x]: %d %s\n"),
                 lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    OAM_DEBUG((_SBX_D(unit, "Setting OH with ohi=(0x%08x - 0x%08x)\n"),
               egrPath.ohIdx,SBX_RAW_OHI_BASE));

    /* We need to find the DATA path's OIX.
     * Do not just use the EgrPath.OI as that is the path for the OAM control pkts.
     * This sets the Co-Co Idx into the oi2e table (LI mode).
     */
    rv = _bcm_fe2000_vswitch_port_gport_get(unit, port, pv2e.vlan, &vlan_gport);
    if (BCM_SUCCESS(rv)) {
        /* lookup FTE based on VSI from VLAN_GPORT. */
        rv = soc_sbx_g2p3_ft_get(unit, (BCM_GPORT_VLAN_PORT_ID_GET(vlan_gport)), &fte);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to get fte 0x%x from vlan_gport: %d %s\n"),
                     (BCM_GPORT_VLAN_PORT_ID_GET(vlan_gport)), rv, bcm_errmsg(rv)));
            return rv;
        }
        /* From FTE use the OI to set oi2e. */
        rv = soc_sbx_g2p3_oi2e_set(unit, fte.oi - SBX_RAW_OHI_BASE,
                                   &egrPath.oh);
        if (BCM_FAILURE(rv)) {
            OAM_ERR((_SBX_D(unit, "Failed to write oi2e[0x%08x]: %d %s\n"),
                     egrPath.ohIdx, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* Set the Co-Co Idx into the epv2e table (TB mode). */
    rv = soc_sbx_g2p3_evp2e_set(unit, pv2e.vlan, port, &evp2e);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write evp2e[%d, 0x%08x]: %d %s\n"),
                 port, pv2e.vlan, rv, bcm_errmsg(rv)));
        return rv;
    }


    OAM_VERB((_SBX_D(unit, "updated lp[0x%x].cocounter=0x%x\n"),
              lpi, coco_idx));
    OAM_VERB((_SBX_D(unit, "updated oi2e[0x%x].cocounter=0x%x\n"),
              egrPath.ohIdx, coco_idx));
    OAM_VERB((_SBX_D(unit, "updated evp2e[%d, 0x%x].cocounter=0x%x\n"),
              port, pv2e.vlan, coco_idx));

    return rv;
}


/*
 *   Function
 *      _oam_g2p3_dm_create
 *   Purpose
 *      Set a g2p3 oamep structure for delay measurements
 *   Parameters
 *       unit        = BCM device number
 *       flags       = BCM flags describing additional features
 *       dmIdx       = delay measurement hw index
 *       epIdx       = endpoint hw index associated with this DM
 *       ftIdx       = Egress path to peer
 *   Returns
 *       BCM_E_*
 */
int _bcm_fe2000_oam_enet_dm_create(int unit, int flags, uint16_t dmIdx, 
                        uint16_t epIdx, uint32_t ftIdx)
{
    int rv;
    soc_sbx_g2p3_oamep_t dmEp, localEp;

    /* There is no persistent data in the dm endpoint; regenerate it
     * each time 
     */
    soc_sbx_g2p3_oamep_t_init(&dmEp);

    rv = soc_sbx_g2p3_oamep_get(unit, epIdx, &localEp);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to read local endpoint at idx 0x%04x: "
                        "%d %s\n"), epIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    dmEp.mode         = !(flags & BCM_OAM_DELAY_ONE_WAY);
    dmEp.function     = _state[unit]->ep_type_ids.dm;
    dmEp.type         = 1; /* Ethernet */
    dmEp.conttohost   = (flags & BCM_OAM_DELAY_ALL_RX_COPY_TO_CPU) ? 1 : 0;
    dmEp.singletohost = (flags & BCM_OAM_DELAY_FIRST_RX_COPY_TO_CPU) ? 1 : 0;
    dmEp.dir          = localEp.dir;
    dmEp.mdlevel      = localEp.mdlevel;
    dmEp.ftidx        = ftIdx;

    rv = soc_sbx_g2p3_oamep_set(unit, dmIdx, &dmEp);
    if (BCM_FAILURE(rv)) {
        OAM_ERR((_SBX_D(unit, "Failed to write delay data at idx 0x%04x"
                        ": %d %s\n"), dmIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    OAM_VERB((_SBX_D(unit, "Wrote DelayIdx=0x%04x, EpIdx=0x%04x\n"),
              dmIdx, epIdx));
    return rv;
}

#endif  /* INCLUDE_L3 */
