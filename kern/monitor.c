// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/dwarf.h>
#include <kern/kdebug.h>
#include <kern/dwarf_api.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGNETA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define RESET "\x1b[0m"

static const char *const names_of_regs[] =
{
	"rax", "rdx", "rcx", "rbx",
	"rsi", "rdi", "rbp", "rsp",
	"r8",  "r9",  "r10", "r11",
	"r12", "r13", "r14", "r15",
	"rip",
	"xmm0",  "xmm1",  "xmm2",  "xmm3",
	"xmm4",  "xmm5",  "xmm6",  "xmm7",
	"xmm8",  "xmm9",  "xmm10", "xmm11",
	"xmm12", "xmm13", "xmm14", "xmm15",
	"st0", "st1", "st2", "st3",
	"st4", "st5", "st6", "st7",
	"mm0", "mm1", "mm2", "mm3",
	"mm4", "mm5", "mm6", "mm7",
	"rflags",
	"es", "cs", "ss", "ds", "fs", "gs", NULL, NULL,
	"fs.base", "gs.base", NULL, NULL,
	"tr", "ldtr",
	/* "mxcsr", "fcw", "fsw" */
};


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Stack backtrace", mon_backtrace },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

/*
uint64_t read_rdi_from_info(struct Ripdebuginfo info) {
	return info.reg_table.rules[5].dw_regnum;
}
*/

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	//cprintf("mon_backtrace(%016x %016x %016x)\n", argc, argv, tf);
	// Your code here.
	uint64_t rip, *rbp;
	read_rip(rip);
	rbp = (uint64_t *) read_rbp();
	cprintf("Stack backtrace:\n");
	while (rbp) {
		cprintf("  rbp %016x  rip %016x\n", rbp, rip);

		// print debuginfo_rip()
		char* rbpc = (char*) rbp;

		struct Ripdebuginfo info;
		debuginfo_rip((uintptr_t) rip, &info);

		cprintf("       %s:%d: %s+%016x  args:%d",
			info.rip_file, info.rip_line, info.rip_fn_name, rip - info.rip_fn_addr, info.rip_fn_narg);

		for (int i = 0; i < info.rip_fn_narg; ++ i) {
			char* arg_start = &rbpc[info.reg_table.cfa_rule.dw_offset + info.offset_fn_arg[i]];
			if (info.size_fn_arg[i] == 8) {
				cprintf("  %016x", *(uint64_t *) arg_start);
			} else if (info.size_fn_arg[i] == 4) {
				cprintf("  %016x", *(uint32_t *) arg_start);
			}
			else cprintf("  %016x", *arg_start);
		}

    cprintf("\n");
/*
		for (int i = 0; i < info.rip_fn_narg; ++ i) {
			cprintf("  %016x", rbpc[info.reg_table.cfa_rule.dw_offset + info.offset_fn_arg[i]]);
		}
		cprintf("\nCFA: reg %s off %d\n",
			names_of_regs[info.reg_table.cfa_rule.dw_regnum],
			info.reg_table.cfa_rule.dw_offset);
*/
		rip = rbp[1];
		rbp = (uint64_t *) *rbp;
	}
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	//cprintf("runcmd(%016x, %016x)\n", buf, tf);
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	cprintf(RED "Print RED and then RESET.\n" RESET);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
