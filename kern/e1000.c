#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
// LAB 6: Your driver code here
volatile uint32_t* e1000_bar0;

struct tx_desc bufdesc[MAXBUF];
struct rx_desc rxbfdesc[MAXRBUF];
int pci_attach_e1000(struct pci_func *f){
	pci_func_enable(f);
	e1000_bar0 = mmio_map_region(f->reg_base[0], f->reg_size[0]);
	cprintf("e1000 status = %016x\n", e1000_bar0[E1000_STATUS >> 2]);
	memset(bufdesc, 0, sizeof(struct tx_desc) * MAXBUF);
	memset(rxbfdesc, 0, sizeof(struct rx_desc) * MAXRBUF);
	struct PageInfo* page;
	for(int i = 0; i < MAXBUF; i++){
		page = page_alloc(1);
		if(!page) panic("No memory for transmit buffers\n");
		bufdesc[i].addr = page2pa(page);
		bufdesc[i].status = E1000_TXD_STAT_DD;
	}
	for(int i = 0; i < MAXRBUF; i++)
	{
		page = page_alloc(1);
		if(!page) panic("No memory for receive buffers\n");
		rxbfdesc[i].buffer_addr = page2pa(page);
		rxbfdesc[i].status = 0;
	}
	e1000_bar0[E1000_TDBAL >> 2] = PADDR(bufdesc);
	e1000_bar0[E1000_TDBAH >> 2] = PADDR(bufdesc) >> 32;
  e1000_bar0[E1000_TDLEN >> 2] = sizeof(struct tx_desc) * MAXBUF;
	e1000_bar0[E1000_TDH >> 2] = 0x0;
	e1000_bar0[E1000_TDT >> 2] = 0x0;

	e1000_bar0[E1000_TCTL >> 2] = E1000_TCTL_EN | E1000_TCTL_PSP | 0x00000100 | 0x00040000;
	e1000_bar0[E1000_TIPG >> 2] = 0x0060100A;

	e1000_bar0[E1000_RA >> 2] = 0x12005452;
	e1000_bar0[(E1000_RA >> 2) + 1] = 0x00005634 | E1000_RAH_AV;
	e1000_bar0[E1000_MTA >> 2] = 0x0;

	e1000_bar0[E1000_RDBAL >> 2] = PADDR(rxbfdesc);
	e1000_bar0[E1000_RDBAH >> 2] = PADDR(rxbfdesc) >> 32;
	e1000_bar0[E1000_RDLEN >> 2] = sizeof(struct rx_desc) * MAXRBUF;
	e1000_bar0[E1000_RDH >> 2] = 0x1;
	e1000_bar0[E1000_RDT >> 2] = 0x0;
	e1000_bar0[E1000_RCTL >> 2] = E1000_RCTL_EN | E1000_RCTL_SZ_2048 | E1000_RCTL_SECRC | E1000_RCTL_BAM;

	return 0;
}

int send_packet(char* buf, int len){
	if(len > BUFSIZE || len == 0) return -1;
	int tail = e1000_bar0[E1000_TDT >> 2];
	struct tx_desc* desc = bufdesc + tail;
	if(desc->status & E1000_TXD_STAT_DD){
		memmove(KADDR(desc->addr), buf, len);
		desc->cmd |= (E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP) >> 24;
		desc->status &= ~(E1000_TXD_STAT_DD);
		desc->length = len;
		desc->status = 0;
		tail = (tail + 1) % MAXBUF;
		e1000_bar0[E1000_TDT >> 2] = tail;
		return 0;
	} 
	else{
		return -1;
	}
}
int recv_packet(char* buf, int* len){
	int tail = e1000_bar0[E1000_RDT >> 2];
	tail = (tail + 1) % MAXRBUF;
	struct rx_desc* desc = rxbfdesc + tail;
	if(desc->status & E1000_RXD_STAT_DD){
		*len = desc->length;
		memmove(buf, KADDR(desc->buffer_addr), desc->length);
		desc->status &= ~E1000_RXD_STAT_DD;
		e1000_bar0[E1000_RDT >> 2] = tail;
		return 0;
	}
	else{
		return -1;
	}
}
