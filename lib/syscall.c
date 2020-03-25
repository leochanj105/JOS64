// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>
#define FAST_SYSCALL 0
static inline int64_t
syscall(int num, int check, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5)
{
	int64_t ret;
	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	//
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations
//	asm volatile("int $3");
#if FAST_SYSCALL
	asm volatile(
							 //"push %%rsi\n"
							// "push %%rcx\n"
							// "push %%rdx\n"
							// "push %%rbp\n"
							 //"int $3\n"
							 //"push %%rbp\n"
							 //"push %%r15\n"
							 //"int $3\n"
						   //"pushf\n"
							 "leaq return, %%r14\n"
							 "push %%rbp\n"
							 "mov %%rsp, %%rbp\n"
							 //"int $3\n"
							 //"mov %%rbp, -8(%%rsp)\n"
							 "sysenter\n"
							 "return:\n"
							 //"popf\n"
							 //"mov %%r15, %%rsp\n"
							  "pop %%rbp\n"
							 //"pop %%r14\n"
							 //"int $3\n"
							 //"mov -8(%%rsp), %%rbp\n"
							// "pop %%rbp\n" 
							 //"pop %%rdx\n" 
							 //"pop %%rcx\n"
							// "pop %%rsi\n"
						 : "=a" (ret)	 
				     : 
							// "8" (a4),
							 //"9" (a5):
					 :"cc","rdi","rsi","rdx", "rcx","r8", "r9", "memory","r14");
	  //asm volatile("pop %%rdx\n"
		 // 					 "pop %%rcx\n"
		//						 "int $3\n"::);
	//panic("ret = %d\n", ret);
#else
	asm volatile("int %1\n"
		     : "=a" (ret)
		     : "i" (T_SYSCALL),
		       "a" (num),
		       "d" (a1),
		       "c" (a2),
		       "b" (a3),
		       "D" (a4),
		       "S" (a5)
		     : "cc", "memory");
#endif
	//asm volatile("int $3");
	//asm volatile("int $3");
	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);
	//asm volatile("int $3");
	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint64_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

