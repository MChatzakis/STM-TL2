/**
 * @file   tm.c
 * @author Emmanouil (Manos) Chatzakis
 *
 * @section LICENSE
 *
 * MIT License
 *
 * Copyright (c) [2023] [Emmanouil Chatzakis]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This is a custom implementation of a Software Transactional Memory (STM) library.
 * It is based on the TL2 algorithm, and it is implemented in C11, using atomic operations.
 * 
 * Every structure is implemented from scratch, including the locks, the sets, the versioned write spinlocks, etc.
 *
 * Memory leaks are verified using valgrind (on a Linux machine) and leaks (on a Macbook).
 **/

// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#ifdef __STDC_NO_ATOMICS__
#error Current C11 compiler does not support atomic operations
#endif

// External headers
#include <pthread.h>

// Internal headers
#include <tm.h>
#include <assert.h>
#include <string.h>

#include "globals.h"
#include "tm_types.h"
#include "utils.h"
#include "rw_sets.h"

#include "macros.h"

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
 **/
shared_t tm_create(size_t size, size_t align)
{
    // Allocate memory for the region struct fields
    region_t *region = (region_t *)malloc(sizeof(region_t));
    if (unlikely(!region))
    {
        dprint_cwarn(COLOR_RED, stdout, "tm_create: Allocation for new TM region failed!\n");
        return invalid_shared;
    }

    // Allign and allocate start memory for the shared region (word_size=align)
    if (unlikely(posix_memalign(&(region->start), align, size)))
    {
        dprint_cwarn(COLOR_RED, stdout, "tm_create: Allocation of start location of TM failed!\n");
        free(region);
        return invalid_shared;
    }

    // Initialize the segment_list lock
    if (unlikely(!def_lock_t_init(&region->segment_list_lock)))
    {
        dprint_cwarn(COLOR_RED, stdout, "tm_create: Allocation of segment lock of the TM failed!\n");
        free(region->start);
        free(region);
        return invalid_shared;
    }

    // Initialize the region struct fields
    memset(region->start, 0, size);
    region->size = size;
    region->align = align;
    region->allocs = NULL;

    // Initialize the global versioned clock
    global_versioned_clock_t_init(&region->global_versioned_clock);

    // Initialized all spinlocks. Spinlocks are mapped to shared memory regions
    for (int i = 0; i < VWSL_NUM; i++)
    {
        versioned_write_spinlock_t_init(&region->versioned_write_spinlock[i]);
    }

    return region;
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
 **/
void tm_destroy(shared_t shared)
{
    region_t *region = (region_t *)shared;

    // Free the start address of the region
    free(region->start);

    // Destroy the locks related to this region
    global_versioned_clock_t_destroy(&region->global_versioned_clock);
    def_lock_t_destroy(&region->segment_list_lock);
    for (int i = 0; i < VWSL_NUM; i++)
    {
        versioned_write_spinlock_t_destroy(&region->versioned_write_spinlock[i]);
    }

    // Free all the allocated segments
    while (region->allocs) {
        segment_list tail = region->allocs->next;
        free(region->allocs);
        region->allocs = tail;
    }

    // Free the region struct
    free(region);
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
 **/
void *tm_start(shared_t shared)
{
    return ((region_t *)shared)->start;
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
 **/
size_t tm_size(shared_t shared)
{
    return ((region_t *)shared)->size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
 **/
size_t tm_align(shared_t shared)
{
    return ((region_t *)shared)->align;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @param is_ro  Whether the transaction is read-only
 * @return Opaque transaction ID, 'invalid_tx' on failure
 **/
tx_t tm_begin(shared_t shared, bool is_ro)
{
    region_t *region = (region_t *)shared;

    // Here, we create a new transaction and return a pointer to the struct representing the transaction
    // TL2 Algorithm: Sample load the current value of the global version clock as rv

    txn_t *txn = txn_t_init(is_ro, global_versioned_clock_t_get_clock(&region->global_versioned_clock), -1);
    if (unlikely(!txn))
    {
        dprint_cwarn(COLOR_RESET, stdout, "tm_begin: Could not allocate a new transaction!\n");
        return invalid_tx;
    }

    return (tx_t)txn;
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
 **/
bool tm_end(shared_t shared, tx_t tx)
{
    // Infer the region and txn that this write is associated with
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;

    bool commit_result;
    if (txn->is_ro || txn->write_set->head == NULL)
    {
        // Read-only txns are validated each time they read a word
        // Reaching this point means that all the reads are succesfully validated
        // Thus, it can commit right away. Same goes for write txns that did not write anything.
        commit_result = COMMIT;
    }
    else
    {
        // Check commit using TL2 algorithm
        commit_result = utils_check_commit(region, txn);
    }

    // Dealloacate the memory used for this txn
    txn_t_destroy(txn);

    dprint_clog(COLOR_RESET, stdout, "tm_end  [%lu]: Deallocated. Commit: %d\n", (tx_t)txn, commit_result);

    return commit_result;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
 **/
bool tm_read(shared_t shared, tx_t tx, void const *source, size_t size, void *target)
{
    // Infer the region and txn that this write is associated with
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;
    size_t word_size = region->align;

    dprint_clog(COLOR_RESET, stdout, "tm_read [%lu]:  Reading from %lu to %lu\n", (tx_t)txn, source, target);

    if (txn->is_ro)
    {
        //
        // TL2 Algorithm (Read instruction for a read-only txn):
        //
        // Execute the transaction code.
        //
        // Txn is post-validated by checking that
        //  - The location’s versioned write-lock is free
        //  - Making sure that the lock’s version field is <= rv
        //
        // If it is greater than rv: the transaction is aborted, otherwise continues.
        //
        // This is very fast, as ro txns do not keep any read set, and are automatically commited when end() is called
        //

        // Iterate over the words of the region to be read
        for (size_t i = 0; i < size; i += word_size)
        {
            void *word_addr = (char *)source + i; // Source is the TM segment
            void *targ_addr = (char *)target + i; // Target is the memory that the value of the TM words will be stored

            // Get the versioned write spinlock for this word and validate it
            versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, word_addr);
            
            // Pre-Validate the lock
            int l = versioned_write_spinlock_t_load(vws);
            int readv = l >> 1;
            if (l & 0x1 || (readv > txn->rv))
            {
                txn_t_destroy(txn);
                return false;
            }

            memcpy(targ_addr, word_addr, word_size);

            // Post-Validate the lock
            int n = versioned_write_spinlock_t_load(vws);
            int after_readv = n >> 1;
            if (n & 0x1 || after_readv != readv)
            {
                txn_t_destroy(txn);
                return false;
            }
        }

        dprint_clog(COLOR_RESET, stdout, "tm_read [%lu]:  Read only txn, validated all locks and copied the values\n", (tx_t)txn);
    }
    else
    {
        //
        // TL2 Algorithm (Read instruction for a write txn):
        //
        // Run though speculative execution:
        //  - Add to the read set of txn the addresses of the words the txn reds
        //  - Add to the write set of txn the pairs of (word_address, new value)
        //
        // Here, we only add items to the read set, since the txn aims to read these locations
        //
        // The txn first checks (using a Bloom filter) to see if the source word address is in the write set
        //
        // Sample the associated versioned write lock of the word to load and read.
        // Post-Validate the instruction by checking:
        //  a. The versioned write lock is not locked
        //  b. The version of the versioned write lock is <= rv
        //      - This makes sure that the value of the memory word has not changed since the start of txn
        //
        // If a ^ b, the txn can indeed continue. If either of a or b condition does not hold, the txn aborts
        //
        // The txn reads the contents of the value to the target. If the source word_addr appeared in the write set,
        // it copies the latest value to be written in the location.
        //

        // Iterate over the memory words (word_size = align)
        for (size_t i = 0; i < size; i += word_size)
        {
            void *word_addr = (char *)source + i; // Source is the TM region to be read
            void *targ_addr = (char *)target + i; // Target is the memory that the value of the TM words will be stored

            // Check if the source_word appears in the write set.
            void *val = set_t_get_val_or_null(txn->write_set, word_addr);
            if (val != NULL)
            {
                // If this txn plans to write this word, update it with the current value
                memcpy(targ_addr, val, word_size);
                continue;
            }

            versioned_write_spinlock_t *vws = utils_get_mapped_lock(region->versioned_write_spinlock, word_addr);
            
            // Pre-Validate the lock
            int l = versioned_write_spinlock_t_load(vws);
            int readv = l >> 1;
            if (l & 0x1 || (readv > txn->rv))
            {
                txn_t_destroy(txn);
                return false;
            }

            memcpy(targ_addr, word_addr, word_size);

            // Post-Validate the lock
            int n = versioned_write_spinlock_t_load(vws);
            int after_readv = n >> 1;
            if (n & 0x1 || after_readv != readv)
            {
                txn_t_destroy(txn);
                return false;
            }

            if (unlikely(!set_t_add_or_update(txn->read_set, word_addr, NULL, word_size)))
            {
                txn_t_destroy(txn);
                exit(EXIT_FAILURE);
            }
        }
    }

    dprint_clog(COLOR_RESET, stdout, "tm_read [%lu]:  Actions passed, txn can continue!\n", (tx_t)txn);

    return true;
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
 **/
bool tm_write(shared_t shared, tx_t tx, void const *source, size_t size, void *target)
{
    // Infer the region and txn that this write is associated with
    region_t *region = (region_t *)shared;
    txn_t *txn = (txn_t *)tx;

    //
    // TL2 Algorithm (Write intstruction):
    //
    // Run though speculative execution:
    //  - Add to the read set of txn the addresses of the words the txn reds
    //  - Add to the write set of txn the pairs of (word_address, new value)
    //
    // Here, we only add items to the write set, since the txn aims to write to these locations
    // Specifically, txn aims to write the source data to the target data
    //

    // Iterate the words of the segment (word_size = align)
    for (size_t i = 0; i < size; i += region->align)
    {
        void *word_addr = (char *)target + i;   // Target is the address of the segment in the TM
        void *source_addr = (char *)source + i; // Source contents are the data to be written
        size_t word_size = region->align;

        dprint_clog(COLOR_RESET, stdout, "tm_write[%lu]:  Word write from %lu to %lu\n", (tx_t)txn, source_addr, word_addr);

        // Add or update the entry of word_addr in the write set, setting the value to source_addr
        if (unlikely(!set_t_add_or_update(txn->write_set, word_addr, source_addr, word_size)))
        {
            dprint_cwarn(COLOR_RESET, stdout, "tm_write[%lu]:  Something went wrong when adding data to write-set.\n", (tx_t)txn);
            txn_t_destroy(txn);
            exit(EXIT_FAILURE);
        }
    }

    return true;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
 **/
alloc_t tm_alloc(shared_t shared, tx_t unused(tx), size_t size, void **target)
{
    // Infer the region associated with this alloc call
    region_t *region = (region_t *)shared;

    // Make sure alignment is okay
    size_t align = region->align;
    align = align < sizeof(segment_t *) ? sizeof(void *) : align;

    // Allocate the memory for this new segment
    segment_t *sn;
    if (unlikely(posix_memalign((void **)&sn, align, sizeof(segment_t) + size) != 0))
    {
        dprint_cwarn(COLOR_RESET, stdout, "tm_alloc[%lu]: Something went wrong when memalign. Stoppping!\n", tx);
        return nomem_alloc;
    }

    // Insert the segment in the linked list in a thread-safe way
    def_lock_t_lock(&region->segment_list_lock);
    sn->prev = NULL;
    sn->next = region->allocs;
    if (sn->next)
        sn->next->prev = sn;
    region->allocs = sn;
    def_lock_t_unlock(&region->segment_list_lock);

    // Initialize segment words with NULL
    void *segment = (void *)((uintptr_t)sn + sizeof(segment_t));
    memset(segment, 0, size);

    // Set the target pointing to the first word of this newly allocated segment
    *target = segment;

    return success_alloc;
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
 **/
bool tm_free(shared_t unused(shared), tx_t unused(tx), void *unused(target))
{
    // Note: All the segments allocated by any txn will be freed when the region is destroyed.
    return true;
}
