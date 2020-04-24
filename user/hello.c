// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	sys_change_priority(0, 10000);             //change parent's priority to 10000
	for(int i = 1; i < 11; i++){
		int cid = fork();
		if(cid == 0){
			while(thisenv->priority == 10000) ;  //wait till priority changed
			cprintf("i am environment %08x\n", thisenv->env_id);
			return ;
		}
		cprintf("created child[%d]: %08x with priotity %d\n", i, cid, 100-i);
		sys_change_priority(cid, 100-i);   // Let the earlier created child get higher priority
	}
}
