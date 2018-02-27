#ifndef _SB_G2P3_FE_CLS_ACE_INTERNAL_H_
#define _SB_G2P3_FE_CLS_ACE_INTERNAL_H_
/* --------------------------------------------------------------------------
 *
 * $Id: g2p3_ace_internal.h 1.7.52.1 Broadcom SDK $
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
 * soc_sbx_g2p3_ace_internal.h : Classifier Access Control Entry interface Private
 *
 * --------------------------------------------------------------------------*/

/*
 * Doubly linked queue structure
 */

typedef struct adq_s *adq_p_t;
typedef struct adq_s
{
  volatile adq_p_t flink;	/* Forward link  */
  volatile adq_p_t blink;	/* Backward link */
}
adq_t;
/*
 * Conventions:
 * q - queue - (dp_p_t*)
 * e,n - element (adq_p_t)
 */

#define ACEDQ_PT(e) ((adq_p_t)(e))

#define ACEDQ_INIT(e)                     \
do                                      \
{                                       \
  ACEDQ_PT(e)->flink = ACEDQ_PT(e);     \
  ACEDQ_PT(e)->blink = ACEDQ_PT(e);     \
} while (0)

/* true if queue (q) is empty */
#define ACEDQ_EMPTY(q) (*(q) == NULL)

#define ACEDQ_HEAD(q) ( (ACEDQ_EMPTY(q) ? NULL : ACEDQ_PT(*(q))) )
#define ACEDQ_TAIL(q) ( (ACEDQ_EMPTY(q) ? NULL : ACEDQ_HEAD(q)->blink) )
#define ACEDQ_NEXT(e) ( ((e)==NULL) ? NULL : ACEDQ_PT(e)->flink)
#define ACEDQ_PREV(e) ( ((e)==NULL) ? NULL : ACEDQ_PT(e)->blink)

#define ACEDQ_IS_HEAD(q, e) ( ACEDQ_HEAD(q) == ACEDQ_PT(e) )
#define ACEDQ_IS_TAIL(q, e) ( ACEDQ_HEAD(q)->blink == ACEDQ_PT(e) )

/*
 * Arguments are:
 *   q: pointer to queue
 *   e: pointer to element
 *   n: pointer to new element to insert before e
*/
#define ACEDQ_INSERT_PREV(q, e, n)            \
do                                          \
{                                           \
  ACEDQ_PT(n)->flink = ACEDQ_PT(e);             \
  ACEDQ_PT(n)->blink = ACEDQ_PT(e)->blink;      \
  ACEDQ_PT(e)->blink->flink = ACEDQ_PT(n);      \
  ACEDQ_PT(e)->blink = ACEDQ_PT(n);             \
  if (ACEDQ_IS_HEAD(q, e)) {                  \
    *(adq_p_t*)(q) = ACEDQ_PT(n);              \
  }                                         \
  ACEDQ_SANITY(q);                            \
}while(0)

/*
 * Arguments are:
 *   q: pointer to queue
 *   e: pointer to element
 *   n: pointer to new element to insert after e
*/
#define ACEDQ_INSERT_NEXT(q, e, n)            \
do                                          \
{                                           \
  ACEDQ_PT(n)->flink = ACEDQ_PT(e)->flink;      \
  ACEDQ_PT(n)->blink = ACEDQ_PT(e);             \
  ACEDQ_PT(e)->flink->blink = ACEDQ_PT(n);      \
  ACEDQ_PT(e)->flink = ACEDQ_PT(n);             \
  ACEDQ_SANITY(q);                            \
}while(0)

/*
 * Arguments are:
 *   q: pointer to queue
 *   e: pointer to element (ACEDQ) to insert
*/
#define ACEDQ_INSERT_HEAD(q, e)               \
do                                          \
{                                           \
   if (ACEDQ_EMPTY(q)){                       \
       *(adq_p_t*)(q) = ACEDQ_PT(e);           \
   }else{                                   \
       adq_p_t h = ACEDQ_HEAD(q);              \
       ACEDQ_INSERT_PREV(q, h, e);            \
   }                                        \
  ACEDQ_SANITY(q);                            \
}while(0)

/*
 * Arguments are:
 *   q: pointer to queue
 *   n: pointer to element (ACEDQ) to insert
 */
#define ACEDQ_INSERT_TAIL(q, n)       \
do                                  \
{                                   \
  if (ACEDQ_EMPTY(q)){                \
      ACEDQ_INSERT_HEAD(q, n);        \
  }else{                            \
      adq_p_t e = ACEDQ_TAIL(q);       \
      ACEDQ_INSERT_NEXT(q, e, n);     \
  }                                 \
  ACEDQ_SANITY(q);                    \
} while (0)

/*
 * Argument is:
 *   q: pointer to queue
 *   e: pointer to element (ACEDQ) to remove
 */
#define ACEDQ_REMOVE(q, e)                                \
do                                                      \
{                                                       \
  SB_ASSERT(!ACEDQ_EMPTY(q));                             \
  if (ACEDQ_HEAD(q) == ACEDQ_TAIL(q)){                      \
     *(adq_p_t*)(q) = NULL;                              \
  }else{                                                \
     if (ACEDQ_IS_HEAD(q, e) ) {                          \
       *(adq_p_t*)(q) = ACEDQ_PT(e)->flink;                \
     }                                                  \
     ACEDQ_PT(e)->blink->flink = ACEDQ_PT(e)->flink;        \
     ACEDQ_PT(e)->flink->blink = ACEDQ_PT(e)->blink;        \
  }                                                     \
  ACEDQ_INIT(e);                                          \
  ACEDQ_SANITY(q);                                        \
} while (0)

/*
 * Arguments are:
 *   q: pointer to queue
 *   e: pointer to element (adq_p_t*) removed from the head of q
 */
#define ACEDQ_REMOVE_HEAD(q, e, t)                        \
do                                                      \
{                                                       \
  adq_p_t h;                                             \
  SB_ASSERT(!ACEDQ_EMPTY(q));                             \
  h = ACEDQ_HEAD(q);                                      \
  *(e) = (t*)(h);                                       \
  ACEDQ_REMOVE(q, h);                                     \
  ACEDQ_SANITY(q);                                        \
} while (0)


/*
 * Arguments are:
 *   q: pointer to queueblock
 *   e: pointer to element (adq_p_t*) removed from the head of q
 */
#define ACEDQ_REMOVE_TAIL(q, e)                           \
do                                                      \
{                                                       \
  ACEDQ_PT(e) = ACEDQ_TAIL(q);                              \
  ACEDQ_REMOVE(q, (e));                                   \
  ACEDQ_SANITY(q);                                        \
} while (0)

/*
 * Arguments:
 *   q:  queue on which to map f
 *   f:  function to apply to each element of q
 */
#define ACEDQ_MAP(q, f, a)                    \
do                                          \
{                                           \
  adq_p_t e;                                 \
  if (!ACEDQ_EMPTY(q)){                       \
    e = ACEDQ_HEAD(q);                        \
    do {                                    \
      (void)(f)(e, a);                      \
      e = ACEDQ_NEXT(e);                      \
    }while(e != ACEDQ_HEAD(q))                \
  }                                         \
}while(0)

/*
 * Arguments:
 *   q:  head of queue to determine length
 */
#define ACEDQ_LENGTH(q, cnt)                  \
do                                          \
{                                           \
  adq_p_t e;                                 \
  (cnt) = 0;                                \
  if (!ACEDQ_EMPTY(q)){                       \
    e = ACEDQ_HEAD(q);                        \
    do {                                    \
      (cnt)++;                              \
      e = e->flink;                         \
    }while(e != ACEDQ_HEAD(q));               \
  }                                         \
} while (0)                                 \

/* Useful for debugging 
 * ....limit of 0x100000 entries is arbitrary 
 */
#if 1
#define ACEDQ_SANITY(q)
#else
#define ACEDQ_SANITY(q)                               \
do                                                  \
{                                                   \
  adq_p_t e;                                         \
  uint32_t cnt = 0;                                 \
  if (!ACEDQ_EMPTY(q)){                               \
    e = ACEDQ_HEAD(q);                                \
    /* exception for single element list */         \
    if ( !((e->flink == e) && (e->blink == e)) ){   \
      do {                                          \
        cnt++;                                      \
        e = e->flink;                               \
        if ( (e == e->flink) || (cnt > 0x100000) ){ \
          /* Self-linked element */                 \
          SB_ASSERT(0);                             \
        }                                           \
      }while(e != ACEDQ_HEAD(q));                     \
    }                                               \
  }                                                 \
} while (0)                                         \

#endif

#define ACEDQ_TRAVERSE(q, e, t)              \
do                                         \
{                                          \
  adq_p_t n;                                \
  uint32_t last=0;                         \
  SB_ASSERT(!ACEDQ_EMPTY(q));                \
  (e) = (t*)ACEDQ_HEAD(q);                   \
  (n) = ACEDQ_NEXT(e);                       \
  do {                                     \
    if (ACEDQ_IS_TAIL(q, e)) last=1;         \

#define ACEDQ_TRAVERSE_END(q, e, t)          \
    if(last)                               \
        break;                             \
    (e) = (t*)n;                           \
    n = n->flink;                          \
  }while(last==0);                         \
  ACEDQ_SANITY(q);                           \
} while (0)

/*
 * Arguments:
 *   q:  head of queue
 *   e:  each elem during traverse
 */
#define ACEDQ_BACK_TRAVERSE(q, e, t)              \
do                                         \
{                                          \
  adq_p_t n;                                \
  uint32_t last=0;                         \
  SB_ASSERT(!ACEDQ_EMPTY(q));                \
  (e) = (t*)ACEDQ_TAIL(q);                   \
  (n) = ACEDQ_PREV(e);                       \
  do {                                     \
    if (ACEDQ_IS_HEAD(q, e)) last=1;         \

#define ACEDQ_BACK_TRAVERSE_END(q, e, t)     \
    if(last)                               \
        break;                             \
    (e) = (t*)n;                           \
    n = n->blink;                          \
  }while(last==0);                         \
  ACEDQ_SANITY(q);                           \
} while (0)

/**
 * @fn soc_sbx_g2p3_ace_init()
 *
 * @brief 
 *
 * This function sets up the classifier driver for use. It prepares 
 * the structures that are used for handling sets, and allocates all 
 * the required resources for use. It does NOT touch the hardware and 
 * therefore issues no DMA transactions affecting the classifier 
 * state. This function should always complete synchronously.
 *
 * @param fe    - driver control structure
 * @param pInit - initialization configuration structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_init(soc_sbx_g2p3_state_t *pFe, 
		 sbCommonConfigParams_p_t cParams,
		 sbIngressConfigParams_p_t iParams);

/**
 * @fn soc_sbx_g2p3_ace_un_init()
 *
 * @brief 
 *
 * This function breaks down the classifier driver, and returns all its
 * resources to the system. It does NOT affect the current state of the
 * classifier hardware. This allows the driver to be restarted on failure.
 * To nullify any existing rules in the classifier memory, a call should be
 * made to soc_sbx_g2p3_ace_clear_all().
 *
 * @param pFe - driver control structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_un_init(soc_sbx_g2p3_state_t *pFe);

/**
 * @fn soc_sbx_g2p3_ace_is_commit_in_progress()
 * 
 * @brief 
 *
 * This function can be used to check if a commit is currently in progress
 * for a given rule database. This can be used to test for completion, or
 * check availability before a commit is started.
 *
 * @param pFe - Driver control structure
 * @param eDb - Which rule database to commit to hardware (Security, Qos)
 *
 * @return - {1 = Commit in progress, 0 = No commit in progress}
 **/
uint8_t
soc_sbx_g2p3_ace_is_commit_in_progress(soc_sbx_g2p3_state_t *pFe, 
			       soc_sbx_g2p3_cls_rule_db_e_t eDb);

/**
 * @fn soc_sbx_g2p3_ace_mem_compact()
 * 
 * @brief 
 *
 * This function can be used to reclaim memory that was allocated to the
 * classification database for storing rules. When rules are allocated, if
 * there is not enough memory to hold the new rules it will request memory
 * from the system in "chunks". Later, when this memory is no longer in use
 * the entries can be coalesced into fewer "chunks", and the free memory can
 * be returned to the system
 *
 * @param pFe - Driver control structure
 *
 * @return - Number of memory 'chunks' that were reclaimed
 **/
uint32_t
soc_sbx_g2p3_ace_mem_compact(soc_sbx_g2p3_state_t *pFe);

#endif
