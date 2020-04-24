// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	//sys_change_priority(0, 10000 - thisenv->env_id);
	cprintf("hello, world\n");
	cprintf("i am environment %08x\n", thisenv->env_id);
}
