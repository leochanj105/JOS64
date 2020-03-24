/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushq $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
  pushq $0;							\
	pushq $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
  Find the PUSHA,POPA macros in kern/macro.h. Since amd64 doesn't support
   pusha,popa so you will find these macros handy.
 */

TRAPHANDLER_NOEC(TH_divide, T_DIVIDE)
TRAPHANDLER_NOEC(TH_debug, T_DEBUG)
TRAPHANDLER_NOEC(TH_nmi, T_NMI)
TRAPHANDLER_NOEC(TH_brkpt, T_BRKPT)
TRAPHANDLER_NOEC(TH_oflow, T_OFLOW)
TRAPHANDLER_NOEC(TH_bound, T_BOUND)
TRAPHANDLER_NOEC(TH_illop, T_ILLOP)
TRAPHANDLER_NOEC(TH_device, T_DEVICE)
TRAPHANDLER(TH_dblflt, T_DBLFLT)
TRAPHANDLER(TH_tss, T_TSS)
TRAPHANDLER(TH_segnp, T_SEGNP)
TRAPHANDLER(TH_stack, T_STACK)
TRAPHANDLER(TH_gpflt, T_GPFLT)
TRAPHANDLER(TH_pgflt, T_PGFLT)
TRAPHANDLER_NOEC(TH_fperr, T_FPERR)
TRAPHANDLER(TH_align, T_ALIGN)
TRAPHANDLER_NOEC(TH_mchk, T_MCHK)
TRAPHANDLER_NOEC(TH_simderr, T_SIMDERR)
TRAPHANDLER_NOEC(TH_syscall, T_SYSCALL)
TRAPHANDLER_NOEC(TH_default, T_DEFAULT)
/*
 * Lab 3: Your code here for _alltraps
 *
 * Hint: Be sure to review the x64 calling convention from lab1
 *       for how to pass a pointer to the trapframe.
 */
_alltraps:
subq $16, %rsp
mov %ds, 8(%rsp)
mov %es, 0(%rsp)
PUSHA
mov %rsp, %rdi
push %r15
xor %r15, %r15
mov $(GD_KD), %r15
mov %r15, %ds
mov %r15, %es
pop %r15
call trap
