/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpAggregateHashFormat.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#ifndef SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_H
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_H

#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_SIZE_IN_BYTES 4
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_SIZE 4
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_31_28_BITS "31:28"
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_27_16_BITS "27:16"
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_15_0_BITS "15:0"


typedef struct _sbZfC2PpAggregateHashFormat {
  uint32_t m_uAggregateHash_31_28;
  uint32_t m_uAggregateHash_27_16;
  uint32_t m_uAggregateHash_15_0;
} sbZfC2PpAggregateHashFormat_t;

uint32_t
sbZfC2PpAggregateHashFormat_Pack(sbZfC2PpAggregateHashFormat_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex);
void
sbZfC2PpAggregateHashFormat_Unpack(sbZfC2PpAggregateHashFormat_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex);
void
sbZfC2PpAggregateHashFormat_InitInstance(sbZfC2PpAggregateHashFormat_t *pFrame);

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_SET_AGGR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_SET_AGGR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_SET_AGGR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_GET_AGGR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_GET_AGGR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_C2PPAGGREGATEHASHFORMAT_GET_AGGR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
