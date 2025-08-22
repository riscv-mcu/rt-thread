/*
 * Copyright (c) 2019-Present Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/03/26     Huaqi        First Nuclei RISC-V porting implementation
 */

#include "riscv_encoding.h"
#include <rtconfig.h>

#ifndef __riscv_32e
#define RT_SAVED_REGNUM         32
#else
#define RT_SAVED_REGNUM         14
#endif

#define RT_CONTEXT_SIZE         (RT_SAVED_REGNUM * REGBYTES)

#ifdef RT_USING_SMP
    .extern rt_from_threadsps
    .extern rt_to_threadsps
#else
    .extern rt_interrupt_from_thread
    .extern rt_interrupt_to_thread
#endif

.section    .text

#ifdef RT_USING_SMP
/*
 * void rt_hw_context_switch_to(rt_ubase_t to, struct rt_thread *to_thread);
 * a0 --> to
 * a1 --> to_thread
 */
#else
/*
 * void rt_hw_context_switch_to(rt_ubase_t to);
 * a0 --> to
 */
#endif
    .globl rt_hw_context_switch_to
/* Start the first task.  This also clears the bit that indicates the FPU is
	in use in case the FPU was used before the scheduler was started - which
	would otherwise result in the unnecessary leaving of space in the stack
	for lazy saving of FPU registers. */
.align 3
rt_hw_context_switch_to:
    /* Setup Interrupt Stack using
       The stack that was used by main()
       before the scheduler is started is
       no longer required after the scheduler is started.
       Interrupt stack pointer is stored in CSR_MSCRATCH */
#ifdef RT_USING_SMP
    /* 
     * In SMP (Symmetric Multi-Processing) mode:
     * - Save the 'to' parameter (next thread stack pointer) in s0.
     * - Move 'to_thread' (next thread control block) into a0 for the function call.
     * - Call rt_cpus_lock_status_restore to restore CPU lock status for the new thread.
     * - Restore 'to' back to a0 from s0 for subsequent context switch operations.
     */
    mv s0, a0
    mv a0, a1
    call rt_cpus_lock_status_restore
    mv a0, s0
    /* get correct sp for each cpu
     * each stack size is __STACK_SIZE
     * defined in linker script */
    lui t1, %hi(__STACK_SIZE)
    addi t1, t1, %lo(__STACK_SIZE)
    la t0, _sp
    csrr a1, CSR_MHARTID
    andi a1, a1, 0xFF
    li a2, 0
1:
    beq a1, a2, 2f
    sub t0, t0, t1
    addi a2, a2, 1
    j 1b
2:
#else
    la t0, _sp
#endif
    csrw CSR_MSCRATCH, t0
    LOAD sp, 0x0(a0)                /* Read sp from first TCB member(a0) */

    /* Pop PC from stack and set MEPC */
    LOAD t0,  0  * REGBYTES(sp)
    csrw CSR_MEPC, t0
    /* Pop mstatus from stack and set it */
    LOAD t0,  (RT_SAVED_REGNUM - 1)  * REGBYTES(sp)
    csrw CSR_MSTATUS, t0
    /* Interrupt still disable here */
    /* Restore Registers from Stack */
    LOAD x1,  1  * REGBYTES(sp)    /* RA */
    LOAD x5,  2  * REGBYTES(sp)
    LOAD x6,  3  * REGBYTES(sp)
    LOAD x7,  4  * REGBYTES(sp)
    LOAD x8,  5  * REGBYTES(sp)
    LOAD x9,  6  * REGBYTES(sp)
    LOAD x10, 7  * REGBYTES(sp)
    LOAD x11, 8  * REGBYTES(sp)
    LOAD x12, 9  * REGBYTES(sp)
    LOAD x13, 10 * REGBYTES(sp)
    LOAD x14, 11 * REGBYTES(sp)
    LOAD x15, 12 * REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 13 * REGBYTES(sp)
    LOAD x17, 14 * REGBYTES(sp)
    LOAD x18, 15 * REGBYTES(sp)
    LOAD x19, 16 * REGBYTES(sp)
    LOAD x20, 17 * REGBYTES(sp)
    LOAD x21, 18 * REGBYTES(sp)
    LOAD x22, 19 * REGBYTES(sp)
    LOAD x23, 20 * REGBYTES(sp)
    LOAD x24, 21 * REGBYTES(sp)
    LOAD x25, 22 * REGBYTES(sp)
    LOAD x26, 23 * REGBYTES(sp)
    LOAD x27, 24 * REGBYTES(sp)
    LOAD x28, 25 * REGBYTES(sp)
    LOAD x29, 26 * REGBYTES(sp)
    LOAD x30, 27 * REGBYTES(sp)
    LOAD x31, 28 * REGBYTES(sp)
#endif

    addi sp, sp, RT_CONTEXT_SIZE

    mret

.align 2
.global eclic_msip_handler
eclic_msip_handler:
    addi sp, sp, -RT_CONTEXT_SIZE
    STORE x1,  1  * REGBYTES(sp)    /* RA */
    STORE x5,  2  * REGBYTES(sp)
    STORE x6,  3  * REGBYTES(sp)
    STORE x7,  4  * REGBYTES(sp)
    STORE x8,  5  * REGBYTES(sp)
    STORE x9,  6  * REGBYTES(sp)
    STORE x10, 7  * REGBYTES(sp)
    STORE x11, 8  * REGBYTES(sp)
    STORE x12, 9  * REGBYTES(sp)
    STORE x13, 10 * REGBYTES(sp)
    STORE x14, 11 * REGBYTES(sp)
    STORE x15, 12 * REGBYTES(sp)
#ifndef __riscv_32e
    STORE x16, 13 * REGBYTES(sp)
    STORE x17, 14 * REGBYTES(sp)
    STORE x18, 15 * REGBYTES(sp)
    STORE x19, 16 * REGBYTES(sp)
    STORE x20, 17 * REGBYTES(sp)
    STORE x21, 18 * REGBYTES(sp)
    STORE x22, 19 * REGBYTES(sp)
    STORE x23, 20 * REGBYTES(sp)
    STORE x24, 21 * REGBYTES(sp)
    STORE x25, 22 * REGBYTES(sp)
    STORE x26, 23 * REGBYTES(sp)
    STORE x27, 24 * REGBYTES(sp)
    STORE x28, 25 * REGBYTES(sp)
    STORE x29, 26 * REGBYTES(sp)
    STORE x30, 27 * REGBYTES(sp)
    STORE x31, 28 * REGBYTES(sp)
#endif
    /* Push mstatus to stack */
    csrr t0, CSR_MSTATUS
    STORE t0,  (RT_SAVED_REGNUM - 1)  * REGBYTES(sp)

    /* Push additional registers */

    /* Store sp to task stack */
#ifdef RT_USING_SMP
    // t0 = &rt_to_threadsps[read_csr(CSR_MHARTID)];
    // t0 = *t0;
    la t0, rt_from_threadsps
    csrr t1, CSR_MHARTID
    slli t1, t1, LOG_REGBYTES
    add t0, t0, t1
    LOAD t0, 0(t0)
#else
    LOAD t0, rt_interrupt_from_thread
#endif
    STORE sp, 0(t0)
    csrr t0, CSR_MEPC
    STORE t0, 0(sp)

    jal rt_hw_taskswitch

    /* Switch task context */
#ifdef RT_USING_SMP
    // t0 = &rt_to_threadsps[read_csr(CSR_MHARTID)];
    // t0 = *t0;
    la t0, rt_to_threadsps
    csrr t1, CSR_MHARTID
    slli t1, t1, LOG_REGBYTES
    add t0, t0, t1
    LOAD t0, 0(t0)
#else
    LOAD t0, rt_interrupt_to_thread
#endif
    LOAD sp, 0x0(t0)

    /* Pop PC from stack and set MEPC */
    LOAD t0,  0  * REGBYTES(sp)
    csrw CSR_MEPC, t0

    /* Pop additional registers */

    /* Pop mstatus from stack and set it */
    LOAD t0,  (RT_SAVED_REGNUM - 1)  * REGBYTES(sp)
    csrw CSR_MSTATUS, t0
    /* Interrupt still disable here */
    /* Restore Registers from Stack */
    LOAD x1,  1  * REGBYTES(sp)    /* RA */
    LOAD x5,  2  * REGBYTES(sp)
    LOAD x6,  3  * REGBYTES(sp)
    LOAD x7,  4  * REGBYTES(sp)
    LOAD x8,  5  * REGBYTES(sp)
    LOAD x9,  6  * REGBYTES(sp)
    LOAD x10, 7  * REGBYTES(sp)
    LOAD x11, 8  * REGBYTES(sp)
    LOAD x12, 9  * REGBYTES(sp)
    LOAD x13, 10 * REGBYTES(sp)
    LOAD x14, 11 * REGBYTES(sp)
    LOAD x15, 12 * REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 13 * REGBYTES(sp)
    LOAD x17, 14 * REGBYTES(sp)
    LOAD x18, 15 * REGBYTES(sp)
    LOAD x19, 16 * REGBYTES(sp)
    LOAD x20, 17 * REGBYTES(sp)
    LOAD x21, 18 * REGBYTES(sp)
    LOAD x22, 19 * REGBYTES(sp)
    LOAD x23, 20 * REGBYTES(sp)
    LOAD x24, 21 * REGBYTES(sp)
    LOAD x25, 22 * REGBYTES(sp)
    LOAD x26, 23 * REGBYTES(sp)
    LOAD x27, 24 * REGBYTES(sp)
    LOAD x28, 25 * REGBYTES(sp)
    LOAD x29, 26 * REGBYTES(sp)
    LOAD x30, 27 * REGBYTES(sp)
    LOAD x31, 28 * REGBYTES(sp)
#endif

    addi sp, sp, RT_CONTEXT_SIZE
    mret
