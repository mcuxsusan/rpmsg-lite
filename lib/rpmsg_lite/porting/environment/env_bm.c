/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 * FILE NAME
 *
 *       bm_env.c
 *
 *
 * DESCRIPTION
 *
 *       This file is Bare Metal Implementation of env layer for OpenAMP.
 *
 *
 **************************************************************************/

#include "env.h"
#include "platform.h"
#include "virtqueue.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int env_init_counter = 0;

/* Max supported ISR counts */
#define ISR_COUNT (2) /* Change for multiple remote cores */
                      /*!
                       * Structure to keep track of registered ISR's.
                       */
struct isr_info
{
    void *data;
};
static struct isr_info isr_table[ISR_COUNT];

/*!
 * env_in_isr
 *
 * @returns True, if currently in ISR
 *
 */
inline int env_in_isr(void)
{
    return platform_in_isr();
}

/*!
 * env_init
 *
 * Initializes OS/BM environment.
 *
 */
int env_init()
{
    // verify 'env_init_counter'
    assert(env_init_counter >= 0);
    if (env_init_counter < 0)
        return -1;
    env_init_counter++;
    // multiple call of 'env_init' - return ok
    if (1 < env_init_counter)
        return 0;
    // first call
    memset(isr_table, 0, sizeof(isr_table));
    return platform_init();
}

/*!
 * env_deinit
 *
 * Uninitializes OS/BM environment.
 *
 * @returns Execution status
 */
int env_deinit()
{
    // verify 'env_init_counter'
    assert(env_init_counter > 0);
    if (env_init_counter <= 0)
        return -1;
    // counter on zero - call platform deinit
    env_init_counter--;
    // multiple call of 'env_deinit' - return ok
    if (0 < env_init_counter)
        return 0;
    // last call
    return platform_deinit();
}

/*!
 * env_allocate_memory - implementation
 *
 * @param size
 */
void *env_allocate_memory(unsigned int size)
{
    return (malloc(size));
}

/*!
 * env_free_memory - implementation
 *
 * @param ptr
 */
void env_free_memory(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}

/*!
 *
 * env_memset - implementation
 *
 * @param ptr
 * @param value
 * @param size
 */
void env_memset(void *ptr, int value, unsigned long size)
{
    memset(ptr, value, size);
}

/*!
 *
 * env_memcpy - implementation
 *
 * @param dst
 * @param src
 * @param len
 */
void env_memcpy(void *dst, void const *src, unsigned long len)
{
    memcpy(dst, src, len);
}

/*!
 *
 * env_strcmp - implementation
 *
 * @param dst
 * @param src
 */

int env_strcmp(const char *dst, const char *src)
{
    return (strcmp(dst, src));
}

/*!
 *
 * env_strncpy - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
void env_strncpy(char *dest, const char *src, unsigned long len)
{
    strncpy(dest, src, len);
}

/*!
 *
 * env_strncmp - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
int env_strncmp(char *dest, const char *src, unsigned long len)
{
    return (strncmp(dest, src, len));
}

/*!
 *
 * env_mb - implementation
 *
 */
void env_mb()
{
    MEM_BARRIER();
}

/*!
 * osalr_mb - implementation
 */
void env_rmb()
{
    MEM_BARRIER();
}

/*!
 * env_wmb - implementation
 */
void env_wmb()
{
    MEM_BARRIER();
}

/*!
 * env_map_vatopa - implementation
 *
 * @param address
 */
unsigned long env_map_vatopa(void *address)
{
    return platform_vatopa(address);
}

/*!
 * env_map_patova - implementation
 *
 * @param address
 */
void *env_map_patova(unsigned long address)
{
    return platform_patova(address);
}

/*!
 * env_create_mutex
 *
 * Creates a mutex with the given initial count.
 *
 */
int env_create_mutex(void **lock, int count)
{
    /* make the mutex pointer point to itself
     * this marks the mutex handle as initialized.
     * In BM, interrupt turning on and off is used
     * instead of mutex. */
    *lock = lock;
    return 0;
}

/*!
 * env_delete_mutex
 *
 * Deletes the given lock
 *
 */
void env_delete_mutex(void *lock)
{
}

/*!
 * env_lock_mutex
 *
 * Tries to acquire the lock, if lock is not available then call to
 * this function will suspend.
 */
void env_lock_mutex(void *lock)
{
    platform_interrupt_disable_all();
}

/*!
 * env_unlock_mutex
 *
 * Releases the given lock.
 */
void env_unlock_mutex(void *lock)
{
    platform_interrupt_enable_all();
}

/*!
 * env_sleep_msec
 *
 * Suspends the calling thread for given time , in msecs.
 */
void env_sleep_msec(int num_msec)
{
    platform_time_delay(num_msec);
}

/*!
 * env_disable_interrupts
 *
 * Disables system interrupts
 *
 */
void env_disable_interrupts()
{
    platform_interrupt_disable_all();
}

/*!
 * env_restore_interrupts
 *
 * Enables system interrupts
 *
 */
void env_restore_interrupts()
{
    platform_interrupt_enable_all();
}

/*!
 * env_register_isr
 *
 * Registers interrupt handler for the given interrupt vector.
 *
 * @param vq_id Virtual interrupt vector number
 * @param data Interrupt handler data (virtqueue)
 */
void env_register_isr(int vq_id, void *data)
{
    assert(vq_id < ISR_COUNT);
    if (vq_id < ISR_COUNT)
    {
        isr_table[vq_id].data = data;
    }
}

/*!
 * env_enable_interrupt
 *
 * Enables the given interrupt
 *
 * @param vq_id Interrupt vector number
 */

void env_enable_interrupt(unsigned int vq_id)
{
    platform_interrupt_enable(vq_id);
}

/*!
 * env_disable_interrupt
 *
 * Disables the given interrupt
 *
 * @param vq_id Interrupt vector number
 */

void env_disable_interrupt(unsigned int vq_id)
{
    platform_interrupt_disable(vq_id);
}

/*!
 * env_map_memory
 *
 * Enables memory mapping for given memory region.
 *
 * @param pa   - physical address of memory
 * @param va   - logical address of memory
 * @param size - memory size
 * param flags - flags for cache/uncached  and access type
 */

void env_map_memory(unsigned int pa, unsigned int va, unsigned int size, unsigned int flags)
{
    platform_map_mem_region(va, pa, size, flags);
}

/*!
 * env_disable_cache
 *
 * Disables system caches.
 *
 */

void env_disable_cache()
{
    platform_cache_all_flush_invalidate();
    platform_cache_disable();
}

/*========================================================= */
/* Util data / functions for BM */

void env_isr(int vector)
{
    struct isr_info *info;
    assert(vector < ISR_COUNT);
    if (vector < ISR_COUNT)
    {
        info = &isr_table[vector];
        virtqueue_notification((struct virtqueue *)info->data);
    }
}
