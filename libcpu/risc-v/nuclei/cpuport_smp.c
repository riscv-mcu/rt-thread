/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 * Copyright (c) 2025-Present Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>

#include <nuclei_sdk_soc.h>

#ifdef RT_USING_SMP

int rt_hw_cpu_id(void)
{
    return __RV_CSR_READ(CSR_MHARTID);
}

/**
 * @brief Initialize a spin lock
 * 
 * This function initializes a spin lock by setting its value to 0,
 * making it available for acquisition. A memory barrier is executed
 * before initialization to ensure proper synchronization in SMP systems.
 * 
 * @param lock Pointer to the spin lock structure to be initialized
 */
void rt_hw_spin_lock_init(rt_hw_spinlock_t *lock)
{
    lock->slock = 0;
    __SMP_RWMB();
}

/**
 * @brief Acquire a spin lock
 * @param lock Pointer to the spin lock structure
 */
void rt_hw_spin_lock(rt_hw_spinlock_t *lock)
{
    do {
        /* Keep checking if lock is held by another core */
        while (lock->slock) {
            __NOP(); /* CPU pause/yield while waiting */
        }

        /* Attempt to acquire the lock using atomic swap operation */
#if __riscv_xlen == 64
        /* For 64-bit RISC-V cores */
        if (__AMOSWAP_D(&(lock->slock), 1) == 0) {
#else
        /* For 32-bit RISC-V cores */
        if (__AMOSWAP_W(&(lock->slock), 1) == 0) {
#endif
            /* Memory barrier to ensure lock acquisition is visible to all cores */
            __SMP_RWMB();
            break;  /* Lock acquired successfully */
        }
        /* If lock acquisition failed, retry from the beginning */
    } while (1);
}

/**
 * @brief Release a spinlock
 * 
 * This function releases the previously acquired spinlock. It ensures proper memory 
 * ordering by using a read/write memory barrier before releasing the lock.
 * 
 * @param lock Pointer to the spinlock structure to be unlocked
 * 
 * @note This function should only be called after acquiring the spinlock using rt_hw_spin_lock()
 */
void rt_hw_spin_unlock(rt_hw_spinlock_t *lock)
{
    __SMP_RWMB();
    lock->slock = 0;
}


void rt_hw_ipi_send(int ipi_vector, unsigned int cpu_mask)
{
    int idx;

    for (idx = 0; idx < RT_CPUS_NR; idx ++)
    {
        if (cpu_mask & (1 << idx))
        {
            if (rt_cpu_index(idx)->current_thread) {
                rt_schedule_cpu(idx);
            }
        }
    }
}

extern int rt_hw_ticksetup(void);
rt_base_t secondary_boot_flag = 0;

void rt_hw_secondary_cpu_up(void)
{
    secondary_boot_flag = 0xa55a;
    __RWMB();
}

void secondary_cpu_c_start(void)
{
    rt_hw_spin_lock(&_cpus_lock);

    rt_hw_ticksetup();

    rt_system_scheduler_start();
}

void rt_hw_secondary_cpu_idle_exec(void)
{
    asm volatile ("wfi");
}

extern void entry(void);
int smp_main(void)
{
    if (rt_hw_cpu_id() == 0) {
        /* Initialize the tick timer on the primary CPU */
        entry();
    } else {
        __disable_irq();
        while (secondary_boot_flag != 0xa55a) {
            /* Wait for the secondary CPU to be ready */
            __asm__ volatile ("nop");
        }
        secondary_cpu_c_start();
    }
}

#endif /*RT_USING_SMP*/
