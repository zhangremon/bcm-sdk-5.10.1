/******************************************************************************
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
 * g2p3_cuckoo.c: g2p3 cuckoo instance support functions
 *
 * A single EM32 table serves 4 different functions in this application:
 *   port, ovid, ivid: qinq
 *   port, vid, dir:   Ethernet OAM
 *   maid, mep:        Ethernet OAM
 *   isid:             PBB
 *
 * $Id: g2p3_cuckoo.c 1.5.112.1 Broadcom SDK $
 *
 * --------------------------------------------------------------------------*/

#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/g2p3/g2p3_em32.h>
#include <soc/sbx/g2p3/g2p3_em32_comp.h>

int
soc_sbx_g2p3_cuckoo_init(int unit)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    s = soc_sbx_g2p3_cuckoo_type_pvv_get(unit, &fe->cuckoo_pvv);
    if (!s) {
        s = soc_sbx_g2p3_cuckoo_type_pvd_get(unit, &fe->cuckoo_pvd);
    }
    if (!s) {
        s = soc_sbx_g2p3_cuckoo_type_maidmep_get(unit, &fe->cuckoo_maidmep);
    }
    if (!s) {
        s = soc_sbx_g2p3_cuckoo_type_isid_get(unit, &fe->cuckoo_isid);
    }

    return s;
}

int
soc_sbx_g2p3_pvv2e_add_ext (int unit, int iivid, int iovid, int iport, 
                            soc_sbx_g2p3_pvv2e_t *e)

{
    int s;

    s = soc_sbx_g2p3_pvv2epack_add(unit, iovid, iport, iivid,
                                   (soc_sbx_g2p3_pvv2epack_t *) e);
    return s;
}

int
soc_sbx_g2p3_pvv2e_renew_ext (int unit, int iivid, int iovid, int iport, 
                              soc_sbx_g2p3_pvv2e_t *e)
{
    int s;

    s = soc_sbx_g2p3_pvv2epack_renew(unit, iovid, iport, iivid,
                                     (soc_sbx_g2p3_pvv2epack_t *) e);
    return s;
}

int
soc_sbx_g2p3_pvv2e_get_ext (int unit, int iivid, int iovid, int iport,
                            soc_sbx_g2p3_pvv2e_t *e)
{
    int s;

    s = soc_sbx_g2p3_pvv2epack_get(unit, iovid, iport, iivid,
                                   (soc_sbx_g2p3_pvv2epack_t *) e);
    return s;
}

int
soc_sbx_g2p3_pvv2e_remove_ext (int unit, int iivid, int iovid, int iport)
{
    int s;

    s = soc_sbx_g2p3_pvv2epack_remove(unit, iovid, iport, iivid);
    return s;
}

int
soc_sbx_g2p3_pvv2e_commit_ext (int unit, int runlength)
{
    return soc_sbx_g2p3_pvv2epack_commit(unit, runlength);
}

int
soc_sbx_g2p3_pvv2e_first_ext (int unit, int *niivid, int *niovid, int *niport)
{
    int s;

    s = soc_sbx_g2p3_pvv2epack_first(unit, niovid, niport, niivid);
    return s;
}

int
soc_sbx_g2p3_pvv2e_next_ext(int unit, int iivid, int iovid, int iport,
                            int *niivid, int *niovid, int *niport)
{
    int s;

    s = soc_sbx_g2p3_pvv2epack_next(unit, iovid, iport, iivid,
                                    niovid, niport, niivid);
    return s;
}

/* Keys get (for diag shell) */
int
soc_sbx_g2p3_pvv2e_keys_get(int unit, int ivid, int ovid, int port,
                            uint32 *left, uint32 *right, uint32 *seed,
                            int *b)
{
    int s;
    int kp;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    void *em32 = ((soc_sbx_g2p3_em32_comp_t *) 
                  soc_sbx_g2p3_cuckoo_complex_state_get(unit))->em32;

    s = soc_sbx_g2p3_pvv2epack_index_check(unit, &kp, ovid, port, ivid);
    if (!s) {
        s = soc_sbx_g2p3_cuckoo_index_check(unit, &kp, kp, fe->cuckoo_pvv);
    }

    if (!s) {
        sbG2P3GetKeysEM32(em32, kp, left, right, seed, b);
    }

    return s;
}

int
soc_sbx_g2p3_pvv2epack_add_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvv;

    s = soc_sbx_g2p3_cuckoo_add(unit, *(int *) ikey, type, e);
    return s;
}

int
soc_sbx_g2p3_pvv2epack_renew_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvv;

    s = soc_sbx_g2p3_cuckoo_renew(unit, *(int *) ikey, type, e);
    return s;
}

int
soc_sbx_g2p3_pvv2epack_get_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvv;

    s = soc_sbx_g2p3_cuckoo_get(unit, *(int *) ikey, type, e);
    return s;
}

int
soc_sbx_g2p3_pvv2epack_remove_ext (int unit, uint8 *ikey)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvv;

    s = soc_sbx_g2p3_cuckoo_remove(unit, *(int *) ikey, type);
    return s;
}

int
soc_sbx_g2p3_pvv2epack_commit_ext (int unit, int runlength)
{
    return soc_sbx_g2p3_cuckoo_commit(unit, runlength);
}

int
soc_sbx_g2p3_pvv2epack_first_ext (int unit, uint8 *nikey)
{
    int s;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    s = soc_sbx_g2p3_cuckoo_first(unit, &k, &type);
    while (!s && type != fe->cuckoo_pvv) {
        s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
    }

    if (!s) {
        *(int32 *)nikey = k;
    }

    return s;
}

int
soc_sbx_g2p3_pvv2epack_next_ext (int unit, uint8 *ikey, uint8 *nikey)
{
    int s = SOC_E_MEMORY;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (fe != NULL) {
        type = fe->cuckoo_pvv;
        k = *(int32 *) ikey;
        do {
            s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
        } while (!s && type != fe->cuckoo_pvv);

        if (!s) {
            *(int32 *) nikey = k;
        }
    }

    return s;
}


/************************************************************************
 **                     OAM Port-Vid-Direction 2 Etc
 ************************************************************************/


int
soc_sbx_g2p3_oampvd2e_add_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvd;

    s = soc_sbx_g2p3_cuckoo_add(unit, *(int *) ikey, type, e);
    return s;
}

int
soc_sbx_g2p3_oampvd2e_renew_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvd;

    s = soc_sbx_g2p3_cuckoo_renew(unit, *(int *) ikey, type, e);
    return s;
}


int
soc_sbx_g2p3_oampvd2e_get_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvd;

    s = soc_sbx_g2p3_cuckoo_get(unit, *(int *) ikey, type, e);
    return s;
}


int
soc_sbx_g2p3_oampvd2e_remove_ext (int unit, uint8 *ikey)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_pvd;

    s = soc_sbx_g2p3_cuckoo_remove(unit, *(int *) ikey, type);
    return s;
}

int
soc_sbx_g2p3_oampvd2e_commit_ext (int unit, int runlength)
{
    return soc_sbx_g2p3_cuckoo_commit(unit, runlength);
}

int
soc_sbx_g2p3_oampvd2e_first_ext (int unit, uint8 *nikey)
{
    int s;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    s = soc_sbx_g2p3_cuckoo_first(unit, &k, &type);
    while (!s && type != fe->cuckoo_pvd) {
        s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
    }

    if (!s) {
        *(int32 *)nikey = k;
    }

    return s;
}


int
soc_sbx_g2p3_oampvd2e_next_ext (int unit, uint8 *ikey, uint8 *nikey)
{
    int s = SOC_E_MEMORY;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (fe != NULL) {
        type = fe->cuckoo_pvd;
        k = *(int32 *) ikey;
        do {
            s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
        } while (!s && type != fe->cuckoo_pvd);

        if (!s) {
            *(int32 *) nikey = k;
        }
    }

    return s;
}


/************************************************************************
 **                     OAM MaId-MepId 2 Etc
 ************************************************************************/


int
soc_sbx_g2p3_oammaidmep2e_add_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_maidmep;

    s = soc_sbx_g2p3_cuckoo_add(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_oammaidmep2e_renew_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_maidmep;

    s = soc_sbx_g2p3_cuckoo_renew(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_oammaidmep2e_get_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_maidmep;

    s = soc_sbx_g2p3_cuckoo_get(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_oammaidmep2e_remove_ext (int unit, uint8 *ikey)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_maidmep;

    s = soc_sbx_g2p3_cuckoo_remove(unit, *(int *) ikey, type);
    return s;
}

int 
soc_sbx_g2p3_oammaidmep2e_commit_ext (int unit, int runlength)
{
    return soc_sbx_g2p3_cuckoo_commit(unit, runlength);
}

int 
soc_sbx_g2p3_oammaidmep2e_first_ext (int unit, uint8 *nikey)
{
    int s;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    s = soc_sbx_g2p3_cuckoo_first(unit, &k, &type);
    while (!s && type != fe->cuckoo_maidmep) {
        s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
    }

    if (!s) {
        *(int32 *)nikey = k;
    }

    return s;
}


int 
soc_sbx_g2p3_oammaidmep2e_next_ext (int unit, uint8 *ikey, uint8 *nikey)
{
    int s = SOC_E_MEMORY;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (fe != NULL) {
        type = fe->cuckoo_maidmep;
        k = *(int32 *) ikey;
        do {
            s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
        } while (!s && type != fe->cuckoo_maidmep);

        if (!s) {
            *(int32 *) nikey = k;
        }
    }

    return s;
}


/************************************************************************
 **                     ISid 2 Etc
 ************************************************************************/

int 
soc_sbx_g2p3_isid2e_add_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_isid;

    s = soc_sbx_g2p3_cuckoo_add(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_isid2e_renew_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_isid;

    s = soc_sbx_g2p3_cuckoo_renew(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_isid2e_get_ext (int unit, uint8 *ikey, void *e)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_isid;

    s = soc_sbx_g2p3_cuckoo_get(unit, *(int *) ikey, type, e);
    return s;
}

int 
soc_sbx_g2p3_isid2e_remove_ext (int unit, uint8 *ikey)
{
    int s;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    int type = fe->cuckoo_isid;

    s = soc_sbx_g2p3_cuckoo_remove(unit, *(int *) ikey, type);
    return s;
}

int 
soc_sbx_g2p3_isid2e_commit_ext (int unit, int runlength)
{
    return soc_sbx_g2p3_cuckoo_commit (unit, runlength);
}

int
soc_sbx_g2p3_isid2e_first_ext (int unit, uint8 *nikey)
{
    int s;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    s = soc_sbx_g2p3_cuckoo_first(unit, &k, &type);
    while (!s && type != fe->cuckoo_isid) {
        s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
    }

    if (!s) {
        *(int32 *)nikey = k;
    }

    return s;
}

int 
soc_sbx_g2p3_isid2e_next_ext (int unit, uint8 *ikey, uint8 *nikey)
{
    int s = SOC_E_MEMORY;
    int k, type;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (fe != NULL) {
        type = fe->cuckoo_isid;
        k = *(int32 *) ikey;
        do {
            s = soc_sbx_g2p3_cuckoo_next(unit, k, type, &k, &type);
        } while (!s && type != fe->cuckoo_isid);

        if (!s) {
            *(int32 *) nikey = k;
        }
    }

    return s;
}


/************************************************************************
 **                     Label 2 Etc
 ************************************************************************/
int soc_sbx_g2p3_label2e_add_ext(int unit,
                uint8 *ikey,
                void *epacked)
{
    int status = SOC_E_NONE;
    if(!ikey || !epacked || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_add(unit, *(int*)ikey, 0, epacked);
        if(status == SOC_E_NONE) {
            status = soc_sbx_g2p3_innerlabelcuckoo_add(unit, *(int*)ikey, 0, epacked);
        }
    }
    return status;
}

int soc_sbx_g2p3_label2e_get_ext(int unit,
                uint8 *ikey,
                void *epacked)
{
    int status = SOC_E_NONE;
    if(!ikey || !epacked || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_get(unit, *(int*)ikey, 0, epacked);
        if(status == SOC_E_NONE) {
            status = soc_sbx_g2p3_innerlabelcuckoo_get(unit, *(int*)ikey, 0, epacked);
        }
    }
    return status;
}

int soc_sbx_g2p3_label2e_renew_ext(int unit,
                uint8 *ikey,
                void *epacked)
{
    int status = SOC_E_NONE;
    if(!ikey || !epacked || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_renew(unit, *(int*)ikey, 0, epacked);
        if(status == SOC_E_NONE) {
            status = soc_sbx_g2p3_innerlabelcuckoo_renew(unit, *(int*)ikey, 0, epacked);
        }
    }
    return status;
}

int soc_sbx_g2p3_label2e_remove_ext(int unit,
                                    uint8 *ikey)
{
    int status = SOC_E_NONE;
    if(!ikey || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_delete(unit, *(int*)ikey, 0);
        if(status == SOC_E_NONE) {
            status = soc_sbx_g2p3_innerlabelcuckoo_delete(unit, *(int*)ikey, 0);
        }
    }
    return status;
}

int soc_sbx_g2p3_label2e_first_ext(int unit,
                                    uint8 *nikey)
{
    int status = SOC_E_NONE;
    int type=0;
    if(!nikey || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_first(unit, (int*)nikey, &type);
    }
    return status;
}

int soc_sbx_g2p3_label2e_next_ext(int unit,
                uint8 *ikey,
                uint8 *nikey)
{
    int status = SOC_E_NONE;
    int type=0;
    if(!ikey || !nikey || !SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_next(unit, *(int*)ikey, type, (int*)nikey, &type);
    }
    return status;
}

int soc_sbx_g2p3_label2e_commit_ext(int unit, int runlength)
{
    int status = SOC_E_NONE;

    if(!SOC_SBX_MPLSTP_ENABLE(unit)) {
        status = SOC_E_PARAM;
    } else {
        status = soc_sbx_g2p3_outerlabelcuckoo_commit(unit,runlength);
        if(status == SOC_E_NONE) {
            status = soc_sbx_g2p3_innerlabelcuckoo_commit(unit, runlength);
        }
    }
    return status;
}
