/*
 * $Id: profile_mem.c 1.29.2.2 Broadcom SDK $
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
 * Provides generic routines for managing HW profile tables.
 */

#include <soc/profile_mem.h>
#include <soc/error.h>
#include <soc/drv.h>

/*
 * Function:
 *      soc_profile_mem_t_init
 * Purpose:
 *      Initialize a soc_profile_mem_t structure.
 *
 * Parameters:
 *      profile_mem - (IN) Pointer to profile memory structure
 * Returns:
 *      void
 */
void
soc_profile_mem_t_init(soc_profile_mem_t *profile)
{
    if (NULL != profile) {
        profile->tables = NULL;
        profile->table_count = 0;
        profile->flags = 0;
    }
}

STATIC void
_soc_profile_mem_free(soc_profile_mem_t *profile)
{
    soc_profile_mem_table_t *table;
    int table_index;

    if (profile->tables == NULL) {
        return;
    }

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        if (table->entries != NULL) {
            sal_free(table->entries);
        }
        if (table->cache_p != NULL) {
            sal_free(table->cache_p);
        }
    }
    sal_free(profile->tables);
    profile->tables = NULL;
}

/*
 * Function:
 *      soc_profile_mem_index_create
 * Purpose:
 *      Create a shadow copy and refcounts of a profile table.
 *      If called during WARM BOOT, the shadow copy is populated with
 *      the HW contents, otherwise, both the shadow copy and the
 *      HW entries are cleared.
 *
 * Parameters:
 *      unit              - (IN) Unit
 *      mem_array         - (IN) Pointer to memory id array
 *      entry_words_array - (IN) Pointer to entry size array
 *      index_min_array   - (IN) Pointer to index min array
 *      index_max_array   - (IN) Pointer to index max array
 *      table_count       - (IN) Number of entries in memory id array
 *      profile           - (IN) Pointer to profile memory structure
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_index_create(int unit,
                             soc_mem_t *mem_array,
                             int *entry_words_array,
                             int *index_min_array,
                             int *index_max_array,
                             int table_count,
                             soc_profile_mem_t *profile)
{
    soc_profile_mem_table_t *table;
    int rv;
    int alloc_size;
    int num_entries, table_index, i;
    uint32 *cache_p;

    if (profile == NULL) {
        return SOC_E_INIT;
    }

    if (mem_array == NULL || entry_words_array == NULL ||
        index_min_array == NULL || index_max_array == NULL ||
        table_count == 0) {
        return SOC_E_PARAM;
    }

    if (profile->tables != NULL) {
        _soc_profile_mem_free(profile);
    }

    alloc_size = table_count * sizeof(soc_profile_mem_table_t);
    profile->tables = sal_alloc(alloc_size, "Profile Mem Tables");
    if (profile->tables == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(profile->tables, 0, alloc_size);
    profile->table_count = table_count;

    for (table_index = 0; table_index < table_count; table_index++) {
        table = &profile->tables[table_index];
        if (index_max_array[table_index] <= index_min_array[table_index]) {
            _soc_profile_mem_free(profile);
            return SOC_E_PARAM;
        }
        table->mem = mem_array[table_index];
        table->index_min = index_min_array[table_index];
        table->index_max = index_max_array[table_index];
        table->entry_words = entry_words_array[table_index];

        num_entries = table->index_max - table->index_min + 1;
        alloc_size = num_entries * sizeof(soc_profile_mem_entry_t);
        table->entries = sal_alloc(alloc_size, "Profile Mem Entries");
        if (table->entries == NULL) {
            _soc_profile_mem_free(profile);
            return SOC_E_MEMORY;
        }
        sal_memset(table->entries, 0, alloc_size);

        alloc_size = num_entries * table->entry_words * sizeof(uint32);
        table->cache_p = sal_alloc(alloc_size, "Profile Mem Cache");
        if (table->cache_p == NULL) {
            _soc_profile_mem_free(profile);
            return SOC_E_MEMORY;
        }
        sal_memset(table->cache_p, 0, alloc_size);
    }

    if (SOC_WARM_BOOT(unit) || SOC_IS_RCPU_ONLY(unit)) {
        for (table_index = 0; table_index < profile->table_count;
             table_index++) {
            table = &profile->tables[table_index];
            num_entries = table->index_max - table->index_min + 1;
            for (i = 0; i < num_entries; i++) {
                cache_p = &table->cache_p[table->entry_words * i];
                rv = soc_mem_read(unit, table->mem, MEM_BLOCK_ANY,
                                  table->index_min + i, cache_p);
                if (rv < 0) {
                    _soc_profile_mem_free(profile);
                    return rv;
                }
            }
        }
    } else {
        /* Clear HW memory */
        for (table_index = 0; table_index < profile->table_count;
             table_index++) {
            table = &profile->tables[table_index];
            rv = soc_mem_clear(unit, table->mem, COPYNO_ALL, TRUE);
            if (rv < 0) {
                _soc_profile_mem_free(profile);
                return rv;
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_profile_mem_create(int unit,
                       soc_mem_t *mem_array,
                       int *entry_words_array,
                       int table_count,
                       soc_profile_mem_t *profile)
{
    int index_min_array[_SOC_PROFILE_MEM_MAX_NUM_TABLES];
    int index_max_array[_SOC_PROFILE_MEM_MAX_NUM_TABLES];
    int i;

    if (table_count > _SOC_PROFILE_MEM_MAX_NUM_TABLES) {
        return SOC_E_INTERNAL;
    }

    for (i = 0; i < table_count; i++) {
        index_min_array[i] = soc_mem_index_min(unit, mem_array[i]);
        index_max_array[i] = soc_mem_index_max(unit, mem_array[i]);
    }
    return soc_profile_mem_index_create(unit, mem_array, entry_words_array,
                                        index_min_array, index_max_array,
                                        table_count, profile);
}

/*
 * Function:
 *      soc_profile_mem_destroy
 * Purpose:
 *      Destroy the shadow copy and refcounts of a profile table.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile_mem - (IN) Pointer to profile memory structure
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_destroy(int unit,
                        soc_profile_mem_t *profile)
{
    if (profile == NULL) {
        return SOC_E_PARAM;
    }

    _soc_profile_mem_free(profile);
    return SOC_E_NONE;
}

STATIC int
_soc_profile_mem_check(int unit, soc_profile_mem_t *profile,
                       int base0)                       
{
    soc_profile_mem_table_t *table;
    int num_entries, num_sets, entries_per_set, ref_count;
    int set, table_index, i, base;

    table = &profile->tables[0];
    entries_per_set = table->entries[base0].entries_per_set;

    if (profile->table_count == 1 && entries_per_set == 1) {
        return SOC_E_NONE;
    }

    num_entries = table->index_max - table->index_min + 1;
    num_sets = num_entries / entries_per_set;
    set = base0 / entries_per_set;

    ref_count = table->entries[base0].ref_count;
    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        for (i = 0; i < entries_per_set; i++) {
            if (table->entries[base + i].entries_per_set != entries_per_set ||
                table->entries[base + i].ref_count != ref_count) {
                return SOC_E_INTERNAL;
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_mem_add
 * Purpose:
 *      Add a set of entries (one or more entries) to a profile table. This
 *      routine searches for a matching set in the profile table. If a matching
 *      set is found, the ref count for that entry is incremented and
 *      its base index is returned. If a matching set is not found and a free
 *      set is found, the HW table is updated, the ref count is incremented,
 *      and the base index of the set is returned. If no free set is found, an
 *      error is returned
 *
 * Parameters:
 *      unit             - (IN) Unit
 *      profile          - (IN) Pointer to profile memory structure
 *      entries_array    - (IN) Array of pointer to table entries set
 *      entries_per_set0 - (IN) Number of entries in the set for table 0
 *      index0           - (OUT) Base index to the entries in HW for table 0
 *                                    
 * Returns:
 *      SOC_E_XXX
 *
 * Notes:
 * For example
 * Usually a profile structure is formed by single memory table. However if a
 * profile structure is formed by the combination of 2 tables (table1 and
 * table2 in this example). Each entry in table1 is 2 words long, and each
 * entry in table2 is 3 words long. Argument entries_per_set is uniform on all
 * tables (4 in this example).
 *   +------------------+     +--------------------------+
 *   | entries_array[0] |---> |  table1[0], 2 words long |
 *   +------------------+     +--------------------------+
 *   | entries_array[1] |-+   |  table1[1]               |
 *   +------------------+ |   +--------------------------+
 *                        |   |  table1[2]               |
 *                        |   +--------------------------+
 *                        |   |  table1[3]               |
 *                        |   +--------------------------+
 *                        |   +---------------------------------+
 *                        +-> |  table2[0], 3 words long        |
 *                            +---------------------------------+
 *                            |  table2[1]                      |
 *                            +---------------------------------+
 *                            |  table2[2]                      |
 *                            +---------------------------------+
 *                            |  table2[3]                      |
 *                            +---------------------------------+
 *
 * The code for above example may look like:
 * {
 *     void *entries[2];
 *     table1_entry_t table1[4];
 *     table2_entry_t table2[4];
 *     int entries_per_set0;
 *     uint32 index[2];
 *
 *     fill table1[0], table1[1], table1[2], table1[3]
 *     fill table2[0], table2[1], table2[2], table2[3]
 *     entries[0] = &table1;
 *     entries[1] = &table2;
 *     entries_per_set0 = 4;
 *     soc_profile_mem_add(unit, profile_mem, &entries, entries_per_set0,
 *                         index);
 * }
 */
int
soc_profile_mem_add(int unit,
                    soc_profile_mem_t *profile,
                    void **entries_array,
                    int entries_per_set0,
                    uint32 *index0)
{
    soc_profile_mem_table_t *table;
    int index_min, num_entries, num_sets, entries_per_set;
    int set, table_index, i, base, base0, free_set;
    int entry_words, data_words;
    uint32 *cache_p, *entry_p;

    if (profile == NULL || entries_array == NULL || entries_per_set0 <= 0 ||
        index0 == NULL) {
        return SOC_E_PARAM;
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    num_entries = table->index_max - table->index_min + 1;

    if (num_entries % entries_per_set0) {
        return SOC_E_PARAM;
    }

    num_sets = num_entries / entries_per_set0;

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        num_entries = table->index_max - table->index_min + 1;
        if (entries_array[table_index] == NULL) {
            return SOC_E_PARAM;
        }

        if (num_entries % num_sets) {
            return SOC_E_PARAM;
        }
    }

    /*
     * Search for an existing set that has the same configuration.
     */
    free_set = -1;
    for (set = 0; set < num_sets; set++) {
        base0 = set * entries_per_set0;

        /* Skip unused entries. */
        if (profile->tables[0].entries[base0].ref_count == 0) {
            if (free_set != -1) {
                continue;
            }

            /* Preserve location of free slot. */
            free_set = set;
            if (profile->table_count == 1 && entries_per_set0 == 1) {
                continue;
            }

            for (table_index = 0; table_index < profile->table_count;
                 table_index++) {
                table = &profile->tables[table_index];
                num_entries = table->index_max - table->index_min + 1;
                entries_per_set = num_entries / num_sets;
                base = set * entries_per_set;
                for (i = 0; i < entries_per_set; i++) {
                    if (table->entries[base + i].ref_count) {
                        free_set = -1;
                        break;
                    }
                }
                if (free_set == -1) {
                    break;
                }
            }
            /* If non-shared, break out of main loop on first empty entry */
            if ((profile->flags & SOC_PROFILE_MEM_F_NO_SHARE) &&
                free_set >= 0) {
                break;
            }
            continue;
        }

        /* If non-shared, continue looking for empty entry */
        if (profile->flags & SOC_PROFILE_MEM_F_NO_SHARE) {
            continue;
        }

        /* Skip set of different size */
        if (profile->tables[0].entries[base0].entries_per_set !=
            entries_per_set0) {
            continue;
        }

        /* Compare the new set of entries against the cache */
        for (table_index = 0; table_index < profile->table_count;
             table_index++) {
            table = &profile->tables[table_index];
            entry_words = table->entry_words;
            num_entries = table->index_max - table->index_min + 1;
            entries_per_set = num_entries / num_sets;
            base = set * entries_per_set;
            entry_p = entries_array[table_index];
            cache_p = &table->cache_p[base * entry_words];
            for (i = 0; i < entries_per_set; i++) {
                if (soc_mem_compare_entry(unit, table->mem, cache_p,
                                          entry_p)) {
                    break;
                }
                entry_p += entry_words;
                cache_p += entry_words;
            }
            if (i != entries_per_set) {
                break;
            }
        }
        if (table_index != profile->table_count) {
            continue;
        }

        /* Do optional data integrity check */
        SOC_IF_ERROR_RETURN(_soc_profile_mem_check(unit, profile, base0));

        /* Matched set found */
        for (table_index = 0; table_index < profile->table_count;
             table_index++) {
            table = &profile->tables[table_index];
            num_entries = table->index_max - table->index_min + 1;
            entries_per_set = num_entries / num_sets;
            base = set * entries_per_set;
            for (i = 0; i < entries_per_set; i++) {
                table->entries[base + i].ref_count++;
            }
        }
        *index0 = base0 + profile->tables[0].index_min;

        return SOC_E_NONE;
    }

    if (free_set == -1) {
        return SOC_E_RESOURCE;
    }

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        index_min = table->index_min;
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = free_set * entries_per_set;
        entry_words = table->entry_words;
        data_words = soc_mem_entry_words(unit, table->mem);
        entry_p = entries_array[table_index];
        cache_p = &table->cache_p[base * entry_words];
        for (i = 0; i < entries_per_set; i++) {
            /* Insert the new entries into profile table */
            SOC_IF_ERROR_RETURN(soc_mem_write(unit, table->mem, MEM_BLOCK_ANY,
                                              index_min + base + i, entry_p));

            /* Copy entry into the software cache. */
            sal_memcpy(cache_p, entry_p, data_words * sizeof(uint32));
            entry_p += entry_words;
            cache_p += entry_words;

            table->entries[base + i].ref_count++;
            table->entries[base + i].entries_per_set = entries_per_set;
        }
    }
    *index0 = free_set * entries_per_set0 + profile->tables[0].index_min;

    return SOC_E_NONE;
}

int
soc_profile_mem_single_table_add(int unit,
                                 soc_profile_mem_t *profile,
                                 void *entries,
                                 int entries_per_set,
                                 int *index)
{
    void *entries_array[1];

    entries_array[0] = entries;
    return soc_profile_mem_add(unit, profile, entries_array,
                               entries_per_set, (uint32 *)index);
}

/*
 * Function:
 *      soc_profile_mem_delete
 * Purpose:
 *      Delete the reference to the set of entries (one or more entries) at
 *      the specified base index.
 *
 * Parameters:
 *      unit    - (IN) Unit
 *      profile - (IN) Pointer to profile memory structure
 *      index0  - (IN) Base index to the entries in HW for table 0
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_delete(int unit,
                       soc_profile_mem_t *profile,
                       uint32 index0)
{
    soc_profile_mem_table_t *table;
    int index_min, num_entries, num_sets, entries_per_set;
    int set, table_index, i, base, base0;

    if (profile == NULL) {
        return SOC_E_PARAM;
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    num_entries = table->index_max - table->index_min + 1;
    if (index0 < table->index_min || index0 > table->index_max) {
        return SOC_E_PARAM;
    }

    base0 = index0 - table->index_min;

    if (table->entries[base0].ref_count == 0) {
        return SOC_E_NOT_FOUND;
    }

    entries_per_set = table->entries[base0].entries_per_set;

    if (base0 % entries_per_set) {
        return SOC_E_PARAM;
    }

    num_sets = num_entries / entries_per_set;
    set = base0 / entries_per_set;

    /* Do optional data integrity check */
    SOC_IF_ERROR_RETURN(_soc_profile_mem_check(unit, profile, base0));

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        for (i = 0; i < entries_per_set; i++) {
            table->entries[base + i].ref_count--;
        }
    }

    if (profile->tables[0].entries[base0].ref_count != 0) {
        return SOC_E_NONE;
    }

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        index_min = table->index_min;
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        for (i = 0; i < entries_per_set; i++) {
            /* Insert the new entries into profile table */
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, table->mem, MEM_BLOCK_ANY,
                               index_min + base + i,
                               soc_mem_entry_null(unit, table->mem)));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_mem_set
 * Purpose:
 *      Update existing non-shared entry.
 *
 * Parameters:
 *      unit            - (IN) Unit
 *      profile         - (IN) Pointer to profile memory structure
 *      entries_array   - (IN) Array of pointer to table entries set
 *      index0          - (IN) Base index to update
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_set(int unit,
                    soc_profile_mem_t *profile,
                    void **entries_array,
                    uint32 index0)
{
    soc_profile_mem_table_t *table;
    int index_min, num_entries, num_sets, entries_per_set;
    int set, table_index, i, base, base0;
    int entry_words, data_words;
    uint32 *cache_p, *entry_p;

    if (profile == NULL) {
        return SOC_E_PARAM;
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    num_entries = table->index_max - table->index_min + 1;
    if (index0 < table->index_min || index0 > table->index_max) {
        return SOC_E_PARAM;
    }

    base0 = index0 - table->index_min;

    if (table->entries[base0].ref_count == 0) {
        return SOC_E_NOT_FOUND;
    }

    entries_per_set = table->entries[base0].entries_per_set;

    if (base0 % entries_per_set) {
        return SOC_E_PARAM;
    }

    num_sets = num_entries / entries_per_set;
    set = base0 / entries_per_set;

    /* Do optional data integrity check */
    SOC_IF_ERROR_RETURN(_soc_profile_mem_check(unit, profile, base0));

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        index_min = table->index_min;
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        entry_words = table->entry_words;
        data_words = soc_mem_entry_words(unit, table->mem);
        entry_p = entries_array[table_index];
        cache_p = &table->cache_p[base * entry_words];
        for (i = 0; i < entries_per_set; i++) {
            /* Write the new entries into profile table */
            SOC_IF_ERROR_RETURN(soc_mem_write(unit, table->mem, MEM_BLOCK_ANY,
                                              index_min + base + i, entry_p));

            /* Copy entry into the software cache. */
            sal_memcpy(cache_p, entry_p, data_words * sizeof(uint32));
            entry_p += entry_words;
            cache_p += entry_words;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_mem_get
 * Purpose:
 *      Get a set of entries (one or more entries) at the specified index.
 *
 * Parameters:
 *      unit          - (IN) Unit
 *      profile       - (IN) Pointer to profile memory structure
 *      index0        - (IN) Base index to the entries in HW for table 0
 *      count         - (IN) Array of number of entries to retrieve
 *      entries_array - (OUT) Array of pointer to table entries set
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_get(int unit,
                    soc_profile_mem_t *profile,
                    int index0,
                    int count,
                    void **entries_array)
{
    soc_profile_mem_table_t *table;
    int num_entries, num_sets, entries_per_set;
    int set, table_index, i, base, base0;
    int entry_words;
    uint32 *cache_p, *entry_p;

    if (profile == NULL || count <= 0) {
        return SOC_E_PARAM;
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    num_entries = table->index_max - table->index_min + 1;
    if (index0 < table->index_min || index0 > table->index_max) {
        return SOC_E_PARAM;
    }

    base0 = index0 - table->index_min;

    if (table->entries[base0].ref_count == 0) {
        return SOC_E_NOT_FOUND;
    }

    entries_per_set = table->entries[base0].entries_per_set;

    if (base0 % entries_per_set) {
        return SOC_E_PARAM;
    }

    num_sets = num_entries / entries_per_set;
    set = base0 / entries_per_set;

    /* Do optional data integrity check */
    SOC_IF_ERROR_RETURN(_soc_profile_mem_check(unit, profile, base0));

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        entry_words = table->entry_words;
        entry_p = entries_array[table_index];
        cache_p = &table->cache_p[base * entry_words];
        for (i = 0; i < entries_per_set; i++) {
            if (i >= count) {
                break;
            }
            /* Copy entry from the software cache. (include pad) */
            sal_memcpy(entry_p, cache_p, entry_words * sizeof(uint32));
            entry_p += entry_words;
            cache_p += entry_words;
        }
    }

    return SOC_E_NONE;
}

int
soc_profile_mem_single_table_get(int unit,
                                 soc_profile_mem_t *profile,
                                 int index,
                                 int count,
                                 void *entries)
{
    void *entries_array[1];

    entries_array[0] = entries;
    return soc_profile_mem_get(unit, profile, index, count, entries_array);
}

/*
 * Function:
 *      soc_profile_mem_reference
 * Purpose:
 *      Add the reference to the set of entries (one or more entries) at
 *      the specified base index.
 *
 * Parameters:
 *      unit             - (IN) Unit
 *      profile          - (IN) Pointer to profile memory structure
 *      index0           - (IN) Base index to the entries in HW for table 0
 *      entries_per_set0 - (IN) Number of entries in the set for table 0
 *                              (for WARM BOOT only)
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_reference(int unit,
                          soc_profile_mem_t *profile,
                          int index0,
                          int entries_per_set0)
{
    soc_profile_mem_table_t *table;
    int num_entries, num_sets, entries_per_set;
    int set, table_index, i, base, base0;

    if (profile == NULL) {
        return SOC_E_PARAM;
    }

    if (SOC_WARM_BOOT(unit)) {
        if (entries_per_set0 <= 0) {
            return SOC_E_PARAM;
        }
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    num_entries = table->index_max - table->index_min + 1;
    if (index0 < table->index_min || index0 > table->index_max) {
        return SOC_E_PARAM;
    }

    base0 = index0 - table->index_min;

    if (SOC_WARM_BOOT(unit)) {
        if (num_entries % entries_per_set0) {
            return SOC_E_PARAM;
        }
        entries_per_set = entries_per_set0;
    } else {
        if (table->entries[base0].ref_count == 0) {
            return SOC_E_NOT_FOUND;
        }

        entries_per_set = table->entries[base0].entries_per_set;
    }

    if (base0 % entries_per_set) {
        return SOC_E_PARAM;
    }

    num_sets = num_entries / entries_per_set;
    set = base0 / entries_per_set;

#if 0
    /* Do optional data integrity check */
    SOC_IF_ERROR_RETURN(_soc_profile_mem_check(unit, profile, base0));
#endif

    for (table_index = 0; table_index < profile->table_count; table_index++) {
        table = &profile->tables[table_index];
        num_entries = table->index_max - table->index_min + 1;
        entries_per_set = num_entries / num_sets;
        base = set * entries_per_set;
        for (i = 0; i < entries_per_set; i++) {
            table->entries[base + i].ref_count++;
            table->entries[base + i].entries_per_set = entries_per_set;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_mem_ref_count_get
 * Purpose:
 *      Get the reference count of the cached entry at the specified index.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile     - (IN) Pointer to profile memory structure
 *      index0      - (IN) Base index to the entries in HW for table 0
 *      ref_count   - (OUT) Reference count
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_mem_ref_count_get(int unit,
                              soc_profile_mem_t *profile,
                              int index0,
                              int *ref_count)
{
    soc_profile_mem_table_t *table;
    int base0;

    if (profile == NULL || ref_count == NULL) {
        return SOC_E_PARAM;
    }

    if (profile->tables == NULL || profile->table_count == 0) {
        return SOC_E_INIT;
    }

    table = &profile->tables[0];
    if (index0 < table->index_min || index0 > table->index_max) {
        return SOC_E_PARAM;
    }

    base0 = index0 - table->index_min;

    *ref_count = table->entries[base0].ref_count;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_reg_t_init
 * Purpose:
 *      Initialize a soc_profile_reg_t structure.
 *
 * Parameters:
 *      profile_reg - (IN) Pointer to profile register structure
 * Returns:
 *      void
 */
void
soc_profile_reg_t_init(soc_profile_reg_t *profile_reg)
{
    if (NULL != profile_reg) {
        sal_memset(profile_reg, 0, sizeof(soc_profile_reg_t));
    }
}

/*
 * Function:
 *      soc_profile_reg_create
 * Purpose:
 *      Create a shadow copy and refcounts of a profile table.
 *      If called during WARM BOOT, the shadow copy is populated with
 *      the HW contents, otherwise, both the shadow copy and the
 *      HW entries are cleared.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      regs        - (IN) Pointer to register id array
 *      regs_count  - (IN) Number of entries in register id array
 *      profile_reg - (IN) Pointer to profile register structure
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_create(int unit,
                       soc_reg_t *regs,
                       int regs_count,
                       soc_profile_reg_t *profile_reg)
{
    soc_reg_t reg;
    int rv;
    int num_entries, reg_idx, i;
    int alloc_size;
    uint64 *cache_p;
    uint32 addr;
    uint64 rval;

    if (profile_reg == NULL) {
        return SOC_E_INIT;
    }

    if (regs == NULL || !regs_count) {
        return SOC_E_PARAM;
    }

    num_entries = SOC_REG_NUMELS(unit, regs[0]);
    if (num_entries <= 0) {
        return SOC_E_BADID;
    }
    for (reg_idx = 1; reg_idx < regs_count; reg_idx++) {
        reg = regs[reg_idx];
        /* Make sure index mechanism is consistent across all registers */
        if (SOC_REG_NUMELS(unit, regs[0]) != num_entries) {
            return SOC_E_PARAM;
        }
    }

    alloc_size = num_entries * \
        (sizeof(soc_profile_reg_entry_t) + regs_count * sizeof(uint64)) +
         regs_count * sizeof(soc_reg_t);

    /* If profile_reg->regs is NULL, init the profile_reg for the first
     * time, otherwise simply check for null pointers */
    if (profile_reg->regs != NULL) {
        if (profile_reg->entries == NULL) {
            return SOC_E_INTERNAL;
        }
    } else {
        profile_reg->entries = sal_alloc(alloc_size,  "Profile Reg Entries");
        if (profile_reg->entries == NULL) {
            return SOC_E_MEMORY;
        }
    }
    sal_memset(profile_reg->entries, 0, alloc_size);

    cache_p = (uint64 *)&profile_reg->entries[num_entries];
    for (i = 0; i < num_entries; i++) {
        profile_reg->entries[i].cache_p = cache_p;
        cache_p += regs_count;
    }
    profile_reg->regs = (soc_reg_t *)cache_p;
    for (reg_idx = 0; reg_idx < regs_count; reg_idx++) {
        profile_reg->regs[reg_idx] = regs[reg_idx];
    }
    profile_reg->regs_count = regs_count;

    if (SOC_WARM_BOOT(unit) || SOC_IS_RCPU_ONLY(unit)) {
        for (i = 0; i < num_entries; i++) {
            cache_p = profile_reg->entries[i].cache_p;
            for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
                reg = profile_reg->regs[reg_idx];
                addr = soc_reg_addr(unit, reg, REG_PORT_ANY, i);
                rv = soc_reg_get(unit, reg, REG_PORT_ANY, i, cache_p);
                if (rv < 0) {
                    sal_free(profile_reg->entries);
                    profile_reg->regs = NULL;
                    profile_reg->entries = NULL;
                    return rv;
                }
                cache_p++;
            }
        }
    } else {
        /* Clear HW memory */
        COMPILER_64_ZERO(rval);
        for (i = 0; i < num_entries; i++) {
            for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
                reg = profile_reg->regs[reg_idx];
                addr = soc_reg_addr(unit, reg, REG_PORT_ANY, i);
                rv = soc_reg_set(unit, reg, REG_PORT_ANY, i, rval);
                if (rv < 0) {
                    sal_free(profile_reg->entries);
                    profile_reg->regs = NULL;
                    profile_reg->entries = NULL;
                    return rv;
                }
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_profile_reg_single_reg_create(int unit,
                                  soc_reg_t reg,
                                  soc_profile_reg_t *profile_reg)
{
    return soc_profile_reg_create(unit, &reg, 1, profile_reg);
}

/*
 * Function:
 *      soc_profile_reg_destroy
 * Purpose:
 *      Destroy the shadow copy and refcounts of a profile table.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile_reg - (IN) Pointer to profile register structure
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_destroy(int unit,
                        soc_profile_reg_t *profile_reg)
{
    if (profile_reg != NULL) {
        if (profile_reg->entries != NULL) {
            sal_free(profile_reg->entries);
        }
        profile_reg->regs = NULL;
        profile_reg->entries = NULL;
        return SOC_E_NONE;
    }
    return SOC_E_PARAM;
}

/*
 * Function:
 *      soc_profile_reg_add
 * Purpose:
 *      Add a set of entries (one or more entries) to a profile table. This
 *      routine searches for a matching set in the profile table. If a matching
 *      set is found, the ref count for that entry is incremented and
 *      its base index is returned. If a matching set is not found and a free
 *      set is found, the HW table is updated, the ref count is incremented,
 *      and the base index of the set is returned. If no free set is found, an
 *      error is returned
 *
 * Parameters:
 *      unit            - (IN) Unit
 *      profile_reg     - (IN) Pointer to profile register structure
 *      entries         - (IN) Array of pointer to register entries set
 *      entries_per_set - (IN) Number of entries in the set
 *      index           - (OUT) Base index for the entries allocated in HW
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_add(int unit,
                    soc_profile_reg_t *profile_reg,
                    uint64 **entries,
                    int entries_per_set,
                    uint32 *index)
{
    soc_reg_t reg;
    int base, free_index;
    int num_entries, reg_idx, i;
    uint32 addr;
    uint64 *cache_p, *entry_p;

    if (profile_reg == NULL || profile_reg->regs == NULL ||
        profile_reg->entries == NULL) {
        return SOC_E_INIT;
    }

    num_entries = SOC_REG_NUMELS(unit, profile_reg->regs[0]);
    if (num_entries <= 0) {
        return SOC_E_INTERNAL;
    }

    if (entries == NULL || index == NULL ||
        entries_per_set < 1 || entries_per_set > num_entries) {
        return SOC_E_PARAM;
    }

    /*
     * Search for an existing entry that has the same configuration.
     */
    free_index = -1;
    for (base = 0; base < num_entries; base += entries_per_set) {
        /* Skip unused entries. */
        if (profile_reg->entries[base].ref_count == 0) {
            if (free_index == -1) {
                /* Preserve location of free slot. */
                free_index = base;
                for (i = 1; i < entries_per_set; i++) {
                    if (profile_reg->entries[base + i].ref_count) {
                        free_index = -1;
                        break;
                    }
                }
            }
            continue;
        }

        for (i = 0; i < entries_per_set; i++) {
            if (profile_reg->entries[base + i].entries_per_set !=
                entries_per_set) {
                break;
            }
            for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
                reg = profile_reg->regs[reg_idx];
                entry_p = &entries[reg_idx][i];
                cache_p = &profile_reg->entries[base + i].cache_p[reg_idx];
                if (COMPILER_64_NE(*cache_p, *entry_p)) {
                    break;
                }
            }
            if (reg_idx != profile_reg->regs_count) {
                break;
            }
        }
        if (i == entries_per_set) {
            for (i = 0; i < entries_per_set; i++) {
                profile_reg->entries[base + i].ref_count++;
            }
            *index = base;
            return SOC_E_NONE;
        }
        if (profile_reg->entries[base].entries_per_set > entries_per_set) {
            base += profile_reg->entries[base].entries_per_set -
                    entries_per_set;
        }
    }

    if (free_index == -1) {
        return SOC_E_RESOURCE;
    }

    for (i = 0; i < entries_per_set; i++) {
        for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
            reg = profile_reg->regs[reg_idx];
            entry_p = &entries[reg_idx][i];
            cache_p = &profile_reg->entries[free_index + i].cache_p[reg_idx];

            /* Insert the new entries into profile table */
            addr = soc_reg_addr(unit, reg, REG_PORT_ANY, free_index + i);
            SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, REG_PORT_ANY, free_index + i, 
                                            *entry_p));

            /* Copy entry into the software cache. */
            *cache_p = *entry_p;
        }
        profile_reg->entries[free_index + i].ref_count++;
        profile_reg->entries[free_index + i].entries_per_set = entries_per_set;
    }
    *index = free_index;
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_reg_delete
 * Purpose:
 *      Delete the reference to the set of entries (one or more entries) at
 *      the specified index.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile_reg - (IN) Pointer to profile register structure
 *      index       - (IN) Base index for the entries allocated in HW
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_delete(int unit,
                       soc_profile_reg_t *profile_reg,
                       uint32 index)
{
    soc_reg_t reg;
    int num_entries, entries_per_set, reg_idx, i;
    uint32 addr;
    uint64 rval;

    if (profile_reg == NULL || profile_reg->regs == NULL ||
        profile_reg->entries == NULL) {
        return SOC_E_INIT;
    }

    num_entries = SOC_REG_NUMELS(unit, profile_reg->regs[0]);
    if (num_entries <= 0) {
        return SOC_E_INTERNAL;
    }

    if ((int)index >= num_entries) {
        return SOC_E_PARAM;
    }

    if (profile_reg->entries[index].ref_count == 0) {
        return SOC_E_NOT_FOUND;
    }

    entries_per_set = profile_reg->entries[index].entries_per_set;
    if (index % entries_per_set) {
        return SOC_E_BADID;
    }

    profile_reg->entries[index].ref_count--;
    for (i = 1; i < entries_per_set; i++) {
        profile_reg->entries[index + i].ref_count--;
        if (profile_reg->entries[index + i].ref_count !=
            profile_reg->entries[index].ref_count ||
            profile_reg->entries[index + i].entries_per_set !=
            profile_reg->entries[index].entries_per_set) {
            return SOC_E_INTERNAL;
        }
    }

    if (profile_reg->entries[index].ref_count != 0) {
        return SOC_E_NONE;
    }

    COMPILER_64_ZERO(rval);
    for (i = 0; i < entries_per_set; i++) {
        for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
            /* Clear the entry in the HW */
            reg = profile_reg->regs[reg_idx];
            addr = soc_reg_addr(unit, reg, REG_PORT_ANY, index + i);
            SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, REG_PORT_ANY, index + i, rval));
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_reg_get
 * Purpose:
 *      Get a set of entries (one or more entries) at the specified index.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile_reg - (IN) Pointer to profile register structure
 *      index       - (IN) Base index for the entries allocated in HW
 *      count       - (IN) Number of entries to retreive
 *      entries     - (OUT) Array of pointer to register entries set
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_get(int unit,
                    soc_profile_reg_t *profile_reg,
                    uint32 index,
                    int count,
                    uint64 **entries)
{
    soc_reg_t reg;
    int num_entries, reg_idx, i;
    uint64 *cache_p, *entry_p;

    if (profile_reg == NULL || profile_reg->regs == NULL ||
        profile_reg->entries == NULL) {
        return SOC_E_INIT;
    }

    num_entries = SOC_REG_NUMELS(unit, profile_reg->regs[0]);
    if (num_entries <= 0) {
        return SOC_E_INTERNAL;
    }

    if (entries == NULL || (int)index >= num_entries || count <= 0) {
        return SOC_E_PARAM;
    }

    if (profile_reg->entries[index].ref_count == 0) {
        return SOC_E_NOT_FOUND;
    }

    if (count > profile_reg->entries[index].entries_per_set) {
        return SOC_E_PARAM;
    }

    if (index % profile_reg->entries[index].entries_per_set) {
        return SOC_E_BADID;
    }

    for (i = 1; i < count; i++) {
        /* Do some optional data integrity check */
        if (profile_reg->entries[index + i].ref_count !=
            profile_reg->entries[index].ref_count ||
            profile_reg->entries[index + i].entries_per_set !=
            profile_reg->entries[index].entries_per_set) {
            return SOC_E_INTERNAL;
        }
    }

    for (i = 0; i < count; i++) {
        for (reg_idx = 0; reg_idx < profile_reg->regs_count; reg_idx++) {
            reg = profile_reg->regs[reg_idx];
            entry_p = &entries[reg_idx][i];
            cache_p = &profile_reg->entries[index + i].cache_p[reg_idx];

            /* Copy entry from the software cache. */
            *entry_p = *cache_p;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_reg_reference
 * Purpose:
 *      Add the reference to the set of entries (one or more entries) at
 *      the specified base index.
 *
 * Parameters:
 *      unit            - (IN) Unit
 *      profile_reg     - (IN) Pointer to profile register structure
 *      index           - (IN) Base index for the entries allocated in HW
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_reference(int unit,
                          soc_profile_reg_t *profile_reg,
                          uint32 index,
                          int entries_per_set_override)
{
    int num_entries, entries_per_set, i;

    if (profile_reg == NULL || profile_reg->regs == NULL ||
        profile_reg->entries == NULL) {
        return SOC_E_INIT;
    }

    num_entries = SOC_REG_NUMELS(unit, profile_reg->regs[0]);
    if (num_entries <= 0) {
        return SOC_E_INTERNAL;
    }

    if ((int)index >= num_entries) {
        return SOC_E_PARAM;
    }

    if ((profile_reg->entries[index].ref_count == 0) && 
        (!SOC_WARM_BOOT(unit))) {
        return SOC_E_NOT_FOUND;
    }

    if (SOC_WARM_BOOT(unit)) {
        /* During WB, use the passed in entries_per_set_override value */
        if ((entries_per_set_override < 1) || 
            (entries_per_set_override > num_entries)) {
            return SOC_E_PARAM;
        }
        profile_reg->entries[index].entries_per_set = 
            entries_per_set_override;
    }
    entries_per_set = profile_reg->entries[index].entries_per_set;
    if (index % entries_per_set) {
        return SOC_E_BADID;
    }

    profile_reg->entries[index].ref_count++;
    for (i = 1; i < entries_per_set; i++) {
        profile_reg->entries[index + i].ref_count++;
        if (SOC_WARM_BOOT(unit)) {
            /* During WB, use the passed in entries_per_set_override value */
            profile_reg->entries[index + i].entries_per_set = 
                entries_per_set_override;
        }
        /* Do some optional data integrity check */
        if (profile_reg->entries[index + i].ref_count !=
            profile_reg->entries[index].ref_count ||
            profile_reg->entries[index + i].entries_per_set !=
            profile_reg->entries[index].entries_per_set) {
            return SOC_E_INTERNAL;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_profile_reg_ref_count_get
 * Purpose:
 *      Get the reference count of the cached entry at the specified index.
 *
 * Parameters:
 *      unit        - (IN) Unit
 *      profile_reg - (IN) Pointer to profile register structure
 *      index       - (IN) Base index for the entries allocated in HW
 *      ref_count   - (OUT) Reference count
 * Returns:
 *      SOC_E_XXX
 */
int
soc_profile_reg_ref_count_get(int unit,
                              soc_profile_reg_t *profile_reg,
                              uint32 index, int *ref_count)
{
    int num_entries, entries_per_set;

    if (profile_reg == NULL || profile_reg->regs == NULL ||
        profile_reg->entries == NULL || ref_count == NULL) {
        return SOC_E_INIT;
    }

    num_entries = SOC_REG_NUMELS(unit, profile_reg->regs[0]);
    if (num_entries <= 0) {
        return SOC_E_INTERNAL;
    }

    if ((int)index >= num_entries) {
        return SOC_E_PARAM;
    }

    if (profile_reg->entries[index].ref_count == 0) {
        *ref_count = 0;
        return SOC_E_NONE;
    }

    entries_per_set = profile_reg->entries[index].entries_per_set;
    if (index % entries_per_set) {
        return SOC_E_BADID;
    }

    *ref_count = profile_reg->entries[index].ref_count;

    return SOC_E_NONE;
}
