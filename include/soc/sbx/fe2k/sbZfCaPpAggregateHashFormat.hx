/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpAggregateHashFormat.hx,v 1.3.36.4 2011/05/22 05:38:04 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#ifndef SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_H
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_H

#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_SIZE_IN_BYTES 4
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_SIZE 4
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_31_28_BITS "31:28"
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_27_16_BITS "27:16"
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_M_UAGGREGATEHASH_15_0_BITS "15:0"


typedef struct _sbZfCaPpAggregateHashFormat {
  uint32_t m_uAggregateHash_31_28;
  uint32_t m_uAggregateHash_27_16;
  uint32_t m_uAggregateHash_15_0;
} sbZfCaPpAggregateHashFormat_t;

uint32_t
sbZfCaPpAggregateHashFormat_Pack(sbZfCaPpAggregateHashFormat_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex);
void
sbZfCaPpAggregateHashFormat_Unpack(sbZfCaPpAggregateHashFormat_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex);
void
sbZfCaPpAggregateHashFormat_InitInstance(sbZfCaPpAggregateHashFormat_t *pFrame);

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_SET_AGGR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_SET_AGGR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_SET_AGGR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_GET_AGGR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_GET_AGGR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_CAPPAGGREGATEHASHFORMAT_GET_AGGR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
