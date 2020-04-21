#include "ns.h"
#include "inc/lib.h"
extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
    binaryname = "ns_output";

    // LAB 6: Your code here:
    // 	- read a packet from the network server
    //	- send the packet to the device driver
		envid_t id;
		int perm;
		while(1){
			int r = ipc_recv(&id, &nsipcbuf, &perm);
			if(thisenv->env_ipc_value != NSREQ_OUTPUT || thisenv->env_ipc_from != ns_envid){
				cprintf("Wrong output msg!\n");
				continue;
			}
			while(sys_send_packet(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)){
				sys_yield();
			}

		}
}
