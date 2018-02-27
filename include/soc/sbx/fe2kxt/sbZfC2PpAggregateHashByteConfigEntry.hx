/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpAggregateHashByteConfigEntry.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#ifndef SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_H
#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_H

#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_SIZE_IN_BYTES 5
#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_SIZE 5
#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_M_UDATA1_BITS "39:32"
#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_M_UDATA0_BITS "31:0"


typedef struct _sbZfC2PpAggregateHashByteConfigEntry {
  uint32_t m_uData1;
  uint32_t m_uData0;
} sbZfC2PpAggregateHashByteConfigEntry_t;

uint32_t
sbZfC2PpAggregateHashByteConfigEntry_Pack(sbZfC2PpAggregateHashByteConfigEntry_t *pFrom,
                                          uint8_t *pToData,
                                          uint32_t nMaxToDataIndex);
void
sbZfC2PpAggregateHashByteConfigEntry_Unpack(sbZfC2PpAggregateHashByteConfigEntry_t *pToStruct,
                                            uint8_t *pFromData,
                                            uint32_t nMaxToDataIndex);
void
sbZfC2PpAggregateHashByteConfigEntry_InitInstance(sbZfC2PpAggregateHashByteConfigEntry_t *pFrame);

#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_SET_DATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_SET_DATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_GET_DATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_GET_DATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
