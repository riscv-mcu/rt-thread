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
volatile unsigned long smp_lockcnt[RT_CPUS_NR] = {0};
volatile unsigned long smp_unlockcnt[RT_CPUS_NR] = {0};
#define MAXCNT  1000000
// volatile unsigned long lockidx = 0;
// volatile unsigned long unlockidx = 0;
// volatile uint8_t lockcore[MAXCNT] = {0};
// volatile uint8_t unlockcore[MAXCNT] = {0};
volatile unsigned long spinlockidx = 0;
volatile uint8_t spinidx[MAXCNT] = {0};


int rt_hw_cpu_id(void)
{
    return __RV_CSR_READ(CSR_MHARTID);
}

void rt_hw_spin_lock_init(rt_hw_spinlock_t *lock)
{
    lock->slock = 0;
    __SMP_RWMB();

}

void rt_hw_spin_lock(rt_hw_spinlock_t *lock)
{
    do {
#if __riscv_xlen == 64
        if (__AMOSWAP_D(&(lock->slock), (rt_hw_cpu_id() + 1)) == 0) {
#else
        if (__AMOSWAP_W(&(lock->slock), (rt_hw_cpu_id() + 1)) == 0) {
#endif
            smp_lockcnt[rt_hw_cpu_id()] ++;
            // if (lockidx < MAXCNT) {
            //     lockcore[lockidx] = rt_hw_cpu_id();
            // }
            // lockidx ++;
            if (spinlockidx > MAXCNT) {
                spinlockidx = 0;
            }
            spinidx[spinlockidx] = rt_hw_cpu_id() | 0x80;
            spinlockidx ++;
            break;
        }
    } while (1);
    // __SMP_RWMB();
}

void rt_hw_spin_unlock(rt_hw_spinlock_t *lock)
{
    smp_unlockcnt[rt_hw_cpu_id()] ++;
    // if (unlockidx < MAXCNT) {
    //     unlockcore[lockidx] = rt_hw_cpu_id();
    // }
    // unlockidx ++;
    if (spinlockidx > MAXCNT) {
        spinlockidx = 0;
    }

    spinidx[spinlockidx] = rt_hw_cpu_id();
    spinlockidx ++;

    // if (spinlockidx == 17) {
    //     while (1);
    // }
    lock->slock = 0;
    __SMP_RWMB();
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
