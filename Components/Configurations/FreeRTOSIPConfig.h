
#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

#define ipconfigUSE_RMII 1

// These are all #defines to override the defaults in FreeRTOS TCP IP
#define ipconfigIP_TASK_STACK_SIZE_WORDS        512
//#define ipconfigIP_TASK_PRIORITY                NORMAL_TASK_PRIORITY    // top-but-1 priority, so RX Deferred ISR task can be top priority
#define ipconfigUSE_NETWORK_EVENT_HOOK          1   // enable callbacks for Network up/ Network down
#define ipconfigEVENT_QUEUE_LENGTH		        ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 )

#define ipconfigHAS_DEBUG_PRINTF				0 //1
//#define FreeRTOS_debug_printf(...)			zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)
#define ipconfigHAS_PRINTF						0 //1
//#define FreeRTOS_printf(...)					zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)

//#define FreeRTOS_printf(str, ...)

#define ipconfigNUM_SOCKETS 10
#define ipconfigSOCKETS_UDP     1
#define ipconfigUDP_MAX_RX_PACKETS    5
#define ipconfigUDP_MAX_TX_PACKETS    5

#define ipconfigBYTE_ORDER                      pdFREERTOS_LITTLE_ENDIAN        // Reference manual Pg 192

#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM  1       // not sure about this - Ethernet checksum is added by hardware, but not TCP cksum
#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM  1
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES 1   // hardware will filter Ethernet packets based on MAC address

#define ipconfigNETWORK_MTU                     1526
#define ipconfigTCP_MSS                         460//536 //1460
#define ipconfigTCP_TX_BUFFER_LENGTH            ( 6 * ipconfigTCP_MSS )
#define ipconfigTCP_RX_BUFFER_LENGTH            ( 24 * ipconfigTCP_MSS )

#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS  16
#define ipconfigZERO_COPY_TX_DRIVER             0   // zero copy would be good but fsl_enet.c/h doesn't support it so would have to be rerwitten.
#define ipconfigZERO_COPY_RX_DRIVER             0

#define ipconfigUSE_TCP                         1
#define ipconfigINCLUDE_FULL_INET_ADDR          1   // more flexible address specifications (i.e. strings)
#define ipconfigUSE_DNS                         1
#define ipconfigUSE_DNS_CACHE                   1
#define ipconfigDNS_CACHE_NAME_LENGTH			64
#define ipconfigUSE_DHCP                        1   // Attempt to retrieve IP address / netmask / DNS server and Gateway addresses
#define ipconfigDHCP_REGISTER_HOSTNAME          0   // set to 1 and implement *pcApplicationHostnameHook to give us a text name

#define ipconfigCAN_FRAGMENT_OUTGOING_PACKETS   1
#define ipconfigREPLY_TO_INCOMING_PINGS         1
#define ipconfigSUPPORT_OUTGOING_PINGS          1
#define ipconfigUSE_TCP_WIN						1	// Turns on sliding windows, so packets can appear out-of-order.
#define ipconfigTCP_WIN_SEG_COUNT				12

#define ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME	10000

#define iptraceSTACK_TX_EVENT_LOST(x)
#define iptraceETHERNET_RX_EVENT_LOST()
#define iptraceRECVFROM_DISCARDING_BYTES(x)
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER(x)
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR()

#define ipSTACK_TX_EVENT	17
#define arpGRATUITOUS_ARP_PERIOD					(pdMS_TO_TICKS(300000))

/* Default MAC address configuration.  The demo creates a virtual network
connection that uses this MAC address by accessing the raw Ethernet/WiFi data
to and from a real network connection on the host PC.  See the
configNETWORK_INTERFACE_TO_USE definition above for information on how to
configure the real network connection to use. */
#define configMAC_ADDR0		0x00
#define configMAC_ADDR1		0x11
#define configMAC_ADDR2		0x22
#define configMAC_ADDR3		0x33
#define configMAC_ADDR4		0x44
#define configMAC_ADDR5		0x41

/* Default IP address configuration.  Used in ipconfigUSE_DNS is set to 0, or
ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configIP_ADDR0		192
#define configIP_ADDR1		168
#define configIP_ADDR2		100
#define configIP_ADDR3		200

/* Default gateway IP address configuration.  Used in ipconfigUSE_DNS is set to
0, or ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configGATEWAY_ADDR0	192
#define configGATEWAY_ADDR1	168
#define configGATEWAY_ADDR2	100
#define configGATEWAY_ADDR3	1

/* Default DNS server configuration.  OpenDNS addresses are 208.67.222.222 and
208.67.220.220.  Used in ipconfigUSE_DNS is set to 0, or ipconfigUSE_DNS is set
to 1 but a DNS server cannot be contacted.*/
#define configDNS_SERVER_ADDR0 	192
#define configDNS_SERVER_ADDR1 	168
#define configDNS_SERVER_ADDR2 	100
#define configDNS_SERVER_ADDR3 	1

/* Default netmask configuration.  Used in ipconfigUSE_DNS is set to 0, or
ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		255
#define configNET_MASK3		0

#endif
