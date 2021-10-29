#include "ETH.h"
#include "stdbool.h"
#include "lwip/opt.h"
#include "lwip/dhcp.h"
#include "FreeRTOS.h"
#include "task.h"

#define ETH_MAX_PACKET_SIZE 1518
static uint8_t __align(4) RXBUFF[ETH_MAX_PACKET_SIZE];
static uint8_t __align(4) TXBUFF[ETH_MAX_PACKET_SIZE];
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"
#include "netif/ethernet.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 */
struct ethernetif {
  struct eth_addr *ethaddr;
	bool IsLinked;
	bool IsTXBusy;
	size_t RxLen;
} eth_helper={0};

static ip4_addr_t ip,gw,mask;

struct netif ethif={0};

static err_t
ethernetif_init(struct netif *netif);
static void
ethernetif_input(struct netif *netif);

static char hostname[ETH_MAX_HOSTNAME_LENGTH]="CH579";

static void __start_eth_receive(void)
{
	eth_helper.RxLen=0;
	R8_ETH_ECON1 |= RB_ETH_ECON1_RXRST;
	R8_ETH_ECON1 &= (~RB_ETH_ECON1_RXRST);
	//设置接收缓存,使能接受
	R16_ETH_ERXLN=ETH_MAX_PACKET_SIZE;
	R16_ETH_ERXST=(uint32_t)RXBUFF;
	R8_ETH_ECON1|=RB_ETH_ECON1_RXEN;
}

void ETH_Init(void)
{
	PWR_UnitModCfg( ENABLE, UNIT_SYS_PLL );		//打开PLL
	PWR_UnitModCfg( ENABLE, UNIT_ETH_PHY );		//打开ETH PHY
	
	GPIOAGPPCfg(ENABLE,RB_PIN_ETH_IE);//初始化引脚
	
	R8_ETH_EIE= RB_ETH_EIE_INTIE | RB_ETH_EIE_RXIE | RB_ETH_EIE_LINKIE | RB_ETH_EIE_TXIE | RB_ETH_EIE_RXERIE | RB_ETH_EIE_TXERIE | RB_ETH_EIE_R_EN50 ;//ETH中断设置
	
	R8_ETH_MACON1|=RB_ETH_MACON1_MARXEN;
	
	R8_ETH_MABBIPG=0x00;
	R16_ETH_MAMXFL=sizeof(RXBUFF);
	R8_ETH_MACON2 |=RB_ETH_MACON2_FULDPX | RB_ETH_MACON2_TXCRCEN;
	
	__start_eth_receive();
	
	NVIC_EnableIRQ(ETH_IRQn);//使能NVIC
	
		
	//添加netif
	netif_add(&ethif,&ip,&mask,&gw,NULL,ethernetif_init,ethernet_input);
	
	netif_set_default(&ethif);
	
}

void ETH_CheckState(void)
{
	if(eth_helper.RxLen>0)
	{
		ethernetif_input(&ethif);			
	}	
	{
		bool current_link=netif_is_link_up(&ethif);
		if(current_link!=eth_helper.IsLinked)
		{
			if(eth_helper.IsLinked)
			{
				dhcp_start(&ethif);
				netif_set_link_up(&ethif);					
			}
			else
			{
				netif_set_link_down(&ethif);			
			}
					
		}
	}
	
}

//ETH中断
void ETH_IRQHandler( void )
{
	uint8_t data=R8_ETH_EIR;
	R8_ETH_EIR=data;//清除中断状态
	
	if(data&RB_ETH_EIR_LINKIF)
	{
		eth_helper.IsLinked=!eth_helper.IsLinked;			
	}
	
	if(data&RB_ETH_EIR_TXIF)
	{
		eth_helper.IsTXBusy=false;
	}
	if(data&RB_ETH_EIR_RXERIF)
	{
		eth_helper.IsTXBusy=false;	
	}
	
	if(data&RB_ETH_EIR_RXIF)
	{
			eth_helper.RxLen=R16_ETH_ERXLN;				
	}
	if(data&RB_ETH_EIR_RXERIF)
	{
		  eth_helper.RxLen=R16_ETH_ERXLN;		
	}
	
}

//LWIP netif


/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
	
	ethernetif->RxLen=0;//设置为0

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[5] = ((R32_ETH_MAADRL >> 0)&0xFF);
	netif->hwaddr[4] = ((R32_ETH_MAADRL >> 8)&0xFF);
	netif->hwaddr[3] = ((R32_ETH_MAADRL >> 16)&0xFF);
	netif->hwaddr[2] = ((R32_ETH_MAADRL >> 24)&0xFF);
	netif->hwaddr[1] = ((R16_ETH_MAADRH >> 0)&0xFF);
  netif->hwaddr[0] = ((R16_ETH_MAADRH >> 8)&0xFF);;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
  /*
   * For hardware/netifs that implement MAC filtering.
   * All-nodes link-local is handled by default, so we must let the hardware know
   * to allow multicast packets in.
   * Should set mld_mac_filter previously. */
  if (netif->mld_mac_filter != NULL) {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;

#if ETH_PAD_SIZE
  pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

	size_t index=0;
	
  for (q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
		while(ethernetif->IsTXBusy)
		{
			vTaskDelay(1);
		}
		memcpy(TXBUFF+index,q->payload,q->len);
		index+=q->len;
		if(index>=sizeof(TXBUFF))
		{
			break;
		}
		
  }
	
	{
		//发送
		R16_ETH_ETXLN=index;
		R16_ETH_ETXST=(uint32_t)TXBUFF;
		R8_ETH_ECON1|=RB_ETH_ECON1_TXRTS;		
	}
	ethernetif->IsTXBusy=true;


  MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
  if (((u8_t *)p->payload)[0] & 1) {
    /* broadcast or multicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
  } else {
    /* unicast packet */
    MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
  }
  /* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
  pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = ethernetif->RxLen;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif
		
		
		size_t index=0;
		
    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for (q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
      //read data into(q->payload, q->len);
			memcpy(q->payload,RXBUFF+index,q->len);
			index+=q->len;
			if(index>=ethernetif->RxLen)
			{
				break;			
			}
    }
    //acknowledge that packet has been read();
		
		__start_eth_receive();
		

    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (((u8_t *)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
    } else {
      /* unicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
    }
#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    //drop packet();
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    MIB2_STATS_NETIF_INC(netif, ifindiscards);
  }

  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* if no packet could be read, silently ignore this */
  if (p != NULL) {
    /* pass all packets to ethernet_input, which decides what packets it supports */
    if (netif->input(p, netif) != ERR_OK) {
      LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
      pbuf_free(p);
      p = NULL;
    }
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t
ethernetif_init(struct netif *netif)
{


  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname =hostname;
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 10000000);

  netif->state = &eth_helper;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
#if LWIP_IPV4
  netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;

  eth_helper.ethaddr = (struct eth_addr *) & (netif->hwaddr[0]);
	eth_helper.IsLinked=false;
	eth_helper.IsTXBusy=false;
	
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

