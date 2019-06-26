/* LWIP implementation of NetworkInterfaceAPI
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include "nsapi.h"
#include "nsapi_types.h"
#include "mbed_interface.h"
#include "mbed_assert.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lwip_stack.h"

#include "eth_arch.h"
#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/mld6.h"
#include "lwip/dns.h"
//#include "lwip/prot/dns.h"
#include "lwip/udp.h"
#include "netif/lwip_ethernet.h"
#include "emac_api.h"
#include "lwip_random.h"
#include "lwip_tcp_isn.h"


static nsapi_error_t mbed_lwip_err_remap(err_t err);


#define MBED_NETIF_INIT_FN eth_arch_enetif_init
#define MBED_NETIF_UPDATE_CONFIG eth_arch_enetif_update_config

static bool lwip_inited = false;
static bool lwip_connected = false;
static bool netif_inited = false;



/* TCP/IP and Network Interface Initialisation */
static struct netif lwip_netif;
#if LWIP_DHCP
static bool lwip_dhcp = false;
#endif
static char lwip_mac_address[NSAPI_MAC_SIZE];

#if !LWIP_IPV4 || !LWIP_IPV6
static bool all_zeros(const uint8_t *p, int len)
{
    for (int i = 0; i < len; i++) {
        if (p[i]) {
            return false;
        }
    }

    return true;
}
#endif

static bool convert_mbed_addr_to_lwip(ip_addr_t *out, const nsapi_addr_t *in)
{
#if LWIP_IPV6
    if (in->version == NSAPI_IPv6) {
         IP_SET_TYPE(out, IPADDR_TYPE_V6);
         MEMCPY(ip_2_ip6(out), in->bytes, sizeof(ip6_addr_t));
         return true;
    }
#if !LWIP_IPV4
    /* For bind() and other purposes, need to accept "null" of other type */
    /* (People use IPv4 0.0.0.0 as a general null) */
    if (in->version == NSAPI_UNSPEC ||
        (in->version == NSAPI_IPv4 && all_zeros(in->bytes, 4))) {
        ip_addr_set_zero_ip6(out);
        return true;
    }
#endif
#endif

#if LWIP_IPV4
    if (in->version == NSAPI_IPv4) {
         IP_SET_TYPE(out, IPADDR_TYPE_V4);
         MEMCPY(ip_2_ip4(out), in->bytes, sizeof(ip4_addr_t));
         return true;
    }
#if !LWIP_IPV6
    /* For symmetry with above, accept IPv6 :: as a general null */
    if (in->version == NSAPI_UNSPEC ||
        (in->version == NSAPI_IPv6 && all_zeros(in->bytes, 16))) {
        ip_addr_set_zero_ip4(out);
        return true;
    }
#endif
#endif

#if LWIP_IPV4 && LWIP_IPV6
    if (in->version == NSAPI_UNSPEC) {
#if IP_VERSION_PREF == PREF_IPV4
        ip_addr_set_zero_ip4(out);
#else
        ip_addr_set_zero_ip6(out);
#endif
        return true;
    }
#endif

    return false;
}

static bool convert_lwip_addr_to_mbed(nsapi_addr_t *out, const ip_addr_t *in)
{
#if LWIP_IPV6
    if (IP_IS_V6(in)) {
        out->version = NSAPI_IPv6;
        MEMCPY(out->bytes, ip_2_ip6(in), sizeof(ip6_addr_t));
        return true;
    }
#endif
#if LWIP_IPV4
    if (IP_IS_V4(in)) {
        out->version = NSAPI_IPv4;
        MEMCPY(out->bytes, ip_2_ip4(in), sizeof(ip4_addr_t));
        return true;
    }
#endif
#if LWIP_IPV6 && LWIP_IPV4
    return false;
#endif
}

#if LWIP_IPV4
static const ip_addr_t *mbed_lwip_get_ipv4_addr(const struct netif *netif)
{
    if (!netif_is_up(netif)) {
        return NULL;
    }

    if (!ip4_addr_isany(netif_ip4_addr(netif))) {
        return netif_ip_addr4(netif);
    }

    return NULL;
}
#endif

#if LWIP_IPV6
static const ip_addr_t *mbed_lwip_get_ipv6_addr(const struct netif *netif)
{

    if (!netif_is_up(netif)) {
        return NULL;
    }

    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)) &&
                !ip6_addr_islinklocal(netif_ip6_addr(netif, i))) {
            return netif_ip_addr6(netif, i);
        }
    }

    return NULL;
}
#endif

const ip_addr_t *mbed_lwip_get_ip_addr(bool any_addr, const struct netif *netif)
{
    const ip_addr_t *pref_ip_addr = 0;
    const ip_addr_t *npref_ip_addr = 0;

#if LWIP_IPV4 && LWIP_IPV6
#if IP_VERSION_PREF == PREF_IPV4
    pref_ip_addr = mbed_lwip_get_ipv4_addr(netif);
    npref_ip_addr = mbed_lwip_get_ipv6_addr(netif);
#else
    pref_ip_addr = mbed_lwip_get_ipv6_addr(netif);
    npref_ip_addr = mbed_lwip_get_ipv4_addr(netif);
#endif
#elif LWIP_IPV6
    pref_ip_addr = mbed_lwip_get_ipv6_addr(netif);
#elif LWIP_IPV4
    pref_ip_addr = mbed_lwip_get_ipv4_addr(netif);
#endif

    if (pref_ip_addr) {
        return pref_ip_addr;
    } else if (npref_ip_addr && any_addr) {
        return npref_ip_addr;
    }

    return NULL;
}

static void add_dns_addr_to_dns_list_index(const u8_t addr_type, const u8_t index)
{
#if LWIP_IPV6
    if (addr_type == IPADDR_TYPE_V6) {
        /* 2001:4860:4860::8888 google */
        ip_addr_t ipv6_dns_addr = IPADDR6_INIT(
                PP_HTONL(0x20014860UL),
                PP_HTONL(0x48600000UL),
                PP_HTONL(0x00000000UL),
                PP_HTONL(0x00008888UL));
        dns_setserver(index, &ipv6_dns_addr);
    }
#endif
#if LWIP_IPV4
    if (addr_type == IPADDR_TYPE_V4) {
        /* 8.8.8.8 google */
        ip_addr_t ipv4_dns_addr = IPADDR4_INIT(0x08080808);
        dns_setserver(index, &ipv4_dns_addr);
    }
#endif
}

static int get_ip_addr_type(const ip_addr_t *ip_addr)
{
#if LWIP_IPV6
    if (IP_IS_V6(ip_addr)) {
        return IPADDR_TYPE_V6;
    }
#endif
#if LWIP_IPV4
    if (IP_IS_V4(ip_addr)) {
        return IPADDR_TYPE_V4;
    }
#endif
#if LWIP_IPV6 && LWIP_IPV4
    return IPADDR_TYPE_ANY;
#endif
}

void add_dns_addr(struct netif *lwip_netif)
{
    // Check for existing dns address
    for (char numdns = 0; numdns < DNS_MAX_SERVERS; numdns++) {
        const ip_addr_t *dns_ip_addr = dns_getserver(numdns);
        if (!ip_addr_isany(dns_ip_addr)) {
            return;
        }
    }

    // Get preferred ip version
    const ip_addr_t *ip_addr = mbed_lwip_get_ip_addr(false, lwip_netif);
    u8_t addr_type = IPADDR_TYPE_ANY;

    // Add preferred ip version dns address to index 0
    if (ip_addr) {
        addr_type = get_ip_addr_type(ip_addr);
        add_dns_addr_to_dns_list_index(addr_type, 0);
    }

#if LWIP_IPV4 && LWIP_IPV6
    if (!ip_addr) {
        // Get address for any ip version
        ip_addr = mbed_lwip_get_ip_addr(true, lwip_netif);
        if (!ip_addr) {
            return;
        }
        addr_type = get_ip_addr_type(ip_addr);
        // Add the dns address to index 0
        add_dns_addr_to_dns_list_index(addr_type, 0);
    }

    if (addr_type == IPADDR_TYPE_V4) {
        // If ipv4 is preferred and ipv6 is available add ipv6 dns address to index 1
        ip_addr = mbed_lwip_get_ipv6_addr(lwip_netif);
    } else if (addr_type == IPADDR_TYPE_V6) {
        // If ipv6 is preferred and ipv4 is available add ipv4 dns address to index 1
        ip_addr = mbed_lwip_get_ipv4_addr(lwip_netif);
    } else {
        ip_addr = NULL;
    }

    if (ip_addr) {
        addr_type = get_ip_addr_type(ip_addr);
        add_dns_addr_to_dns_list_index(addr_type, 1);
    }
#endif
}

//[iot2-debug]: Clean later, this part in only kept in case it is needed.
/*
static sys_sem_t lwip_tcpip_inited;
static void mbed_lwip_tcpip_init_irq(void *eh)
{
    sys_sem_signal(&lwip_tcpip_inited);
}

static sys_sem_t lwip_netif_linked;
static sys_sem_t lwip_netif_unlinked;
static void mbed_lwip_netif_link_irq(struct netif *lwip_netif)
{
    if (netif_is_link_up(lwip_netif)) {
        sys_sem_signal(&lwip_netif_linked);
    } else {
        sys_sem_signal(&lwip_netif_unlinked);
    }
}
*/
static char lwip_has_addr_state = 0;

#if LWIP_ETHERNET
static void mbed_lwip_set_mac_address(struct netif *netif)
{
#if (MBED_MAC_ADDRESS_SUM != MBED_MAC_ADDR_INTERFACE)
    netif->hwaddr[0] = MBED_MAC_ADDR_0;
    netif->hwaddr[1] = MBED_MAC_ADDR_1;
    netif->hwaddr[2] = MBED_MAC_ADDR_2;
    netif->hwaddr[3] = MBED_MAC_ADDR_3;
    netif->hwaddr[4] = MBED_MAC_ADDR_4;
    netif->hwaddr[5] = MBED_MAC_ADDR_5;
#else
    mbed_mac_address((char *)netif->hwaddr);
#endif

    netif->hwaddr_len = ETH_HWADDR_LEN;

    /* Use mac address as additional seed to random number generator */
    uint64_t seed = netif->hwaddr[0];
    for (uint8_t i = 1; i < 8; i++) {
        seed <<= 8;
        seed |= netif->hwaddr[i % 6];
    }
    lwip_add_random_seed(seed);
}

static void mbed_lwip_record_mac_address(const struct netif *netif)
{
    const u8_t *mac = netif->hwaddr;
    snprintf(lwip_mac_address, NSAPI_MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
#endif // LWIP_ETHERNET

/* LWIP interface implementation */
const char *mbed_lwip_get_mac_address(void)
{
    return lwip_mac_address[0] ? lwip_mac_address : NULL;
}

char *mbed_lwip_get_ip_address(char *buf, nsapi_size_t buflen)
{
    const ip_addr_t *addr = mbed_lwip_get_ip_addr(true, &lwip_netif);
    if (!addr) {
        return NULL;
    }
#if LWIP_IPV6
    if (IP_IS_V6(addr)) {
        return ip6addr_ntoa_r(ip_2_ip6(addr), buf, buflen);
    }
#endif
#if LWIP_IPV4
    if (IP_IS_V4(addr)) {
        return ip4addr_ntoa_r(ip_2_ip4(addr), buf, buflen);
    }
#endif
#if LWIP_IPV6 && LWIP_IPV4
    return NULL;
#endif
}

char *mbed_lwip_get_netmask(char *buf, nsapi_size_t buflen)
{
#if LWIP_IPV4
    const ip4_addr_t *addr = netif_ip4_netmask(&lwip_netif);
    if (!ip4_addr_isany(addr)) {
        return ip4addr_ntoa_r(addr, buf, buflen);
    } else {
        return NULL;
    }
#else
    return NULL;
#endif
}

char *mbed_lwip_get_gateway(char *buf, nsapi_size_t buflen)
{
#if LWIP_IPV4
    const ip4_addr_t *addr = netif_ip4_gw(&lwip_netif);
    if (!ip4_addr_isany(addr)) {
        return ip4addr_ntoa_r(addr, buf, buflen);
    } else {
        return NULL;
    }
#else
    return NULL;
#endif
}


void eth_input(){
    _eth_arch_ethernetif_input(&lwip_netif);
}


static void mbed_lwip_core_init(void)
{

    // Check if we've already brought up lwip
    if (!lwip_inited) {
	// Seed lwip random
        lwip_seed_random();

        // Initialise TCP sequence number
        uint32_t tcp_isn_secret[4];
        for (int i = 0; i < 4; i++) {
            tcp_isn_secret[i] = LWIP_RAND();
        }
        lwip_init_tcp_isn(0, (u8_t *) &tcp_isn_secret);

        // initialize lwip for a baremetal system
        lwip_init();
        lwip_inited = true;
    }
}

nsapi_error_t mbed_lwip_emac_init(emac_interface_t *emac)
{
#if LWIP_ETHERNET
    // Choose a MAC address - driver can override
    mbed_lwip_set_mac_address(&lwip_netif);

    // Set up network
    if (!netif_add(&lwip_netif,
#if LWIP_IPV4
                   0, 0, 0,
#endif
                   emac, MBED_NETIF_INIT_FN, &ethernet_input)) {
        return -1;//NSAPI_ERROR_DEVICE_ERROR;
    }

    // Note the MAC address actually in use
    mbed_lwip_record_mac_address(&lwip_netif);

    return 0;//NSAPI_ERROR_OK;

#endif //LWIP_ETHERNET
}


int mbed_lwip_bringup_2(bool dhcp, bool ppp, const char *ip, const char *netmask, const char *gw, const nsapi_ip_stack_t stack)
{
    // Check if we've already connected
    if (lwip_connected) {
        return -1;//NSAPI_ERROR_PARAMETER;
    }

    mbed_lwip_core_init();

    
    if (mbed_lwip_emac_init(NULL) < 0){
        return -1;//[iot2-debug]: use predefined errors, possible NSAPI
    }

    netif_inited = true;

    netif_set_default(&lwip_netif);
    
    //netif_set_status_callback(&lwip_netif, mbed_lwip_netif_status_irq);

#if LWIP_IPV6
    if (stack != IPV4_STACK) {
        if (lwip_netif.hwaddr_len == ETH_HWADDR_LEN) {
            netif_create_ip6_linklocal_address(&lwip_netif, 1/*from MAC*/);
        }

#if LWIP_IPV6_MLD
        /*
         * For hardware/netifs that implement MAC filtering.
         * All-nodes link-local is handled by default, so we must let the hardware know
         * to allow multicast packets in.
         * Should set mld_mac_filter previously. */
        if (lwip_netif.mld_mac_filter != NULL) {
            ip6_addr_t ip6_allnodes_ll;
            ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
            lwip_netif.mld_mac_filter(&lwip_netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
        }
#endif /* LWIP_IPV6_MLD */

#if LWIP_IPV6_AUTOCONFIG
        /* IPv6 address autoconfiguration not enabled by default */
        lwip_netif.ip6_autoconfig_enabled = 1;
    } else {
        // Disable router solidifications
        lwip_netif.rs_count = 0;
    }
#endif /* LWIP_IPV6_AUTOCONFIG */
#endif // LWIP_IPV6

#if LWIP_IPV4
    if (stack != IPV6_STACK) {
        if (!dhcp ) {
            ip4_addr_t ip_addr;
            ip4_addr_t netmask_addr;
            ip4_addr_t gw_addr;

            if (!inet_aton(ip, &ip_addr) ||
                !inet_aton(netmask, &netmask_addr) ||
                !inet_aton(gw, &gw_addr)) {
                return NSAPI_ERROR_PARAMETER;
            }

            netif_set_addr(&lwip_netif, &ip_addr, &netmask_addr, &gw_addr);
        }
    }
#endif

    if (netif_is_link_up(&lwip_netif)) {

        netif_set_up(&lwip_netif);
            //return NSAPI_ERROR_NO_CONNECTION;
    }
    else{
        netif_set_down(&lwip_netif);
    }

    netif_set_link_callback(&lwip_netif, MBED_NETIF_UPDATE_CONFIG);

#if LWIP_DHCP
    if (stack != IPV6_STACK) {
        // Connect to the network
        lwip_dhcp = dhcp;

        if (lwip_dhcp) {
            err_t err = dhcp_start(&lwip_netif);
            if (err) {
                return NSAPI_ERROR_DHCP_FAILURE;
            }
        }
    }
#endif

//[iot2-debug]: update the below to catch any errors with DHCP/static network setup
/*
    // If doesn't have address
    if (!mbed_lwip_get_ip_addr(true, &lwip_netif)) {
        if (sys_arch_sem_wait(&lwip_netif_has_any_addr, DHCP_TIMEOUT * 1000) == SYS_ARCH_TIMEOUT) {
            
            return NSAPI_ERROR_DHCP_FAILURE;
        }
    }

#if PREF_ADDR_TIMEOUT
    if (stack != IPV4_STACK && stack != IPV6_STACK) {
        // If address is not for preferred stack waits a while to see
        // if preferred stack address is acquired
        if (!mbed_lwip_get_ip_addr(false, &lwip_netif)) {
            sys_arch_sem_wait(&lwip_netif_has_pref_addr, PREF_ADDR_TIMEOUT * 1000);
        }
    }
#endif
#if BOTH_ADDR_TIMEOUT
    if (stack != IPV4_STACK && stack != IPV6_STACK) {
        // If addresses for both stacks are not available waits a while to
        // see if address for both stacks are acquired
        if (!(mbed_lwip_get_ipv4_addr(&lwip_netif) && mbed_lwip_get_ipv6_addr(&lwip_netif))) {
            sys_arch_sem_wait(&lwip_netif_has_both_addr, BOTH_ADDR_TIMEOUT * 1000);
        }
    }
#endif
*/
    add_dns_addr(&lwip_netif);

    lwip_connected = true;
    return 0;
}

#if LWIP_IPV6
void mbed_lwip_clear_ipv6_addresses(struct netif *lwip_netif)
{
    for (u8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
        netif_ip6_addr_set_state(lwip_netif, i, IP6_ADDR_INVALID);
    }
}
#endif

// Backwards compatibility with people using DEVICE_EMAC
nsapi_error_t mbed_lwip_bringdown(void)
{
    return mbed_lwip_bringdown_2(false);
}

nsapi_error_t mbed_lwip_bringdown_2(bool ppp)
{
    // Check if we've connected
    if (!lwip_connected) {
        return NSAPI_ERROR_PARAMETER;
    }

#if LWIP_DHCP
    // Disconnect from the network
    if (lwip_dhcp) {
        dhcp_release(&lwip_netif);
        dhcp_stop(&lwip_netif);
        lwip_dhcp = false;
    }
#endif

    netif_set_down(&lwip_netif); // [iot2-debug] multiple ppp deletion here

#if LWIP_IPV6
    mbed_lwip_clear_ipv6_addresses(&lwip_netif);
#endif


    lwip_has_addr_state = 0;
    lwip_connected = false;
    return 0;
}

// [iot2-debug]: This has been changed to report LwIP errors directly
/* LWIP error remapping */
/* LWIP error remapping */
static nsapi_error_t mbed_lwip_err_remap(err_t err) {
    switch (err) {
        case ERR_OK:
        case ERR_CLSD:
            return 0;
        case ERR_MEM:
        case ERR_BUF:
            return NSAPI_ERROR_NO_MEMORY;
        case ERR_CONN:
        case ERR_RST:
        case ERR_ABRT:
            return NSAPI_ERROR_NO_CONNECTION;
        case ERR_TIMEOUT:
        case ERR_RTE:
        case ERR_WOULDBLOCK:
            return NSAPI_ERROR_WOULD_BLOCK;
        case ERR_VAL:
        case ERR_USE:
        case ERR_ARG:
            return NSAPI_ERROR_PARAMETER;
        case ERR_INPROGRESS:
            return NSAPI_ERROR_IN_PROGRESS;
        case ERR_ALREADY:
            return NSAPI_ERROR_ALREADY;
        case ERR_ISCONN:
            return NSAPI_ERROR_IS_CONNECTED;
        default:
            return NSAPI_ERROR_DEVICE_ERROR;
    }
}


//[iot2-debug]: this is a special case for EVAL_F469NI board to pass the global
// to the ethernet setup link function.
#if defined(TARGET_STM32F469NI)
#include "stm32469i_eval.h"
#include "stm32469i_eval_io.h"
/**
  * @brief EXTI line detection callbacks
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  
  if (GPIO_Pin == MFX_IRQOUT_PIN)
  {
    /* Get the IT status register value */
    if(BSP_IO_ITGetStatus(MII_INT_PIN))
    {
      eth_arch_enetif_set_link(&lwip_netif);
    }
    BSP_IO_ITClear();
  }
}

#endif //TARGET_STM32F469NI