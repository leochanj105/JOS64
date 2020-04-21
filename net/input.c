#include "ns.h"
#include "inc/lib.h"
extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
    binaryname = "ns_input";

    // LAB 6: Your code here:
    // 	- read a packet from the device driver
    //	- send it to the network server
    // Hint: When you IPC a page to the network server, it will be
    // reading from it for a while, so don't immediately receive
    // another packet in to the same physical page.
		envid_t id;
		int r, len;
		char buffer[2048];
		while(1){
			while((r = sys_recv_packet(buffer, &len)) < 0){
				sys_yield();
			}
			if(len == 0) continue;
			if((r = sys_page_alloc(0, &nsipcbuf, PTE_U | PTE_W | PTE_P))) 
				panic("No page for nsipc!\n");
			nsipcbuf.pkt.jp_len = len;
			memmove(nsipcbuf.pkt.jp_data, buffer, len);
			ipc_send(ns_envid, NSREQ_INPUT, &(nsipcbuf), PTE_P | PTE_W | PTE_U);
			sys_yield();
		}
}
