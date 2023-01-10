/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright � 2013-2021 Sureflap Limited.
* All Rights Reserved.
*
* All information contained herein is, and remains the property of Sureflap 
* Limited.
* The intellectual and technical concepts contained herein are proprietary to
* Sureflap Limited. and may be covered by U.S. / EU and other Patents, patents 
* in process, and are protected by copyright law.
* Dissemination of this information or reproduction of this material is 
* strictly forbidden unless prior written permission is obtained from Sureflap 
* Limited.
*
* Filename: NetworkInterface.c   
* Author:   Chris Cowdery 16/10/2018
* Purpose:  Network Interface between FreeRTOS-TCP and iMXRT1021 Driver.
*
* We provide initialisation, Transmit and Receive functions. We also
* provide a few misc helper functions. 
*             
**************************************************************************/

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "Hermes-app.h"
#include "debug.h"
#include "hermes-time.h"

extern NetworkAddressingParameters_t xNetworkAddressing;

static void iprxisr_task(void *pvParameters);
static void network_watchdog_task(void *pvParameters);
volatile uint32_t last_ethernet_packet_timestamp = 0;
QueueHandle_t xRestartNetworkMailbox;

// This function initialises the MAC and PHY using helper functions in fsl_enet.c and fsl.phy.c
// Note it can be called in two scenarios:
// 1. When the system starts,
// 2. When the network restarts, e.g. a network cable is plugged back in.
// In the latter case, it doesn't have to do so much initialisation.
BaseType_t xNetworkInterfaceInitialise( void )
{
	
    return 0;
}

/**************************************************************
 * Function Name   : network_watchdog
 * Description     : Standalone task to ensure that we are still
 *                 : receiving packets. If not, we assert that the network
 *                 : is down which causes it to reinitialise and restart.
 *                 : This assessment is made by checking for a 2 minute
 *                 : timeout on received packets. 
 *                 :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
static void network_watchdog_task(void *pvParameters)
{
	while(1)
	{

		vTaskDelay(pdMS_TO_TICKS( 1000 )); // sleep for 1 second	
	}
}

/**************************************************************
 * Function Name   : testLinkStatus
 * Description     : Called from prvIPTask() to check that the link
 *                 : is still up. Changes in the return value of this
 *                 : function are tracked and the IP task will issue
 *                 : the eNetworkDown event when the network goes offline.
 *                 : When this occurs, the stack will end up in 
 *                 : InitialiseNetwork() whereupon it will block this
 *                 : task until the Phy has initialised (meaning a
 *                 : cable has been plugged in)
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
BaseType_t testLinkStatus(void)
{
	zprintf(HIGH_IMPORTANCE, "Can't talk to PHY. You should never see me.\r\n");
	return 0;
}

// This function transmits an Ethernet frame
BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend )
{

    return 0;
}


/* Next provide the vNetworkInterfaceAllocateRAMToBuffers() function, which
simply fills in the pucEthernetBuffer member of each descriptor. */
void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{

}

bool NetworkInterface_IsActive(void)
{
	return 0;
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    switch (eNetworkEvent)
    {
        case eNetworkDown:
            zprintf(LOW_IMPORTANCE,"Network down.\r\n");
			//xEventGroupClearBits(xConnectionStatus_EventGroup, CONN_STATUS_NETWORK_UP);
            break;
        case eNetworkUp:
//            zprintf(LOW_IMPORTANCE,"Network up.\r\n");
			//xEventGroupSetBits(xConnectionStatus_EventGroup, CONN_STATUS_NETWORK_UP);
            break;
        default:
            zprintf(MEDIUM_IMPORTANCE,"Unknown IP Network Event Hook\r\n");
            break;
    }
}

uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
													uint16_t usSourcePort,
													uint32_t ulDestinationAddress,
													uint16_t usDestinationPort )
{
	uint32_t result;
    result = hermes_rand();
	return result;
}

eFrameProcessingResult_t SureFilterFramesEarly(const uint8_t* const pucEthernetBuffer)
{
	extern List_t	xBoundUDPSocketsList; // TAM Bit of a bodge - should this be an accessor function?
	ARPPacket_t* ARP = (ARPPacket_t*)pucEthernetBuffer;
	UDPPacket_t* UDP = (UDPPacket_t*)pucEthernetBuffer;

	switch( ARP->xEthernetHeader.usFrameType )
	{
		case ipARP_FRAME_TYPE:
			// Got an ARP, now evaluate addresses to see if we need to care.
			if( *ipLOCAL_IP_ADDRESS_POINTER == 0UL ){ return eReleaseBuffer; } // We don't have an IP address yet.

			if( ipARP_REQUEST == ARP->xARPHeader.usOperation )
			{	// A request for anything other than our address can be ignored.
				if( ARP->xARPHeader.ulTargetProtocolAddress == *ipLOCAL_IP_ADDRESS_POINTER )
				{	// This is an ARP request for this address, so pass it up to the stack.
					return eProcessBuffer;
				}
				return eReleaseBuffer; // An ARP Request but not for us. Ignore.
			} else
			{	// If it's not a Request, it's a Reply that we should process.
				return eProcessBuffer;
			}
			break;

		case ipIPv4_FRAME_TYPE:
			if( ipPROTOCOL_UDP == UDP->xIPHeader.ucProtocol )
			{
				if( 0 == xBoundUDPSocketsList.uxNumberOfItems )
				{	// Got a UDP packet but no sockets to listen.
#if (ipconfigUSE_LLMNR == 1) || (ipconfigUSE_NBNS == 1)
					return eProcessBuffer; // If either of these protocols are in use, we might still use it.
#else
					return eReleaseBuffer;
#endif
				}
			}
			break;
	}

	return eProcessBuffer; // If we get here, it's not an ARP and should be processed.
}

// This task waits for a Notification from the ISR, whereupon it handles received data from the
// Enet MAC and passes it on to the stack. It is lifted from the FreeRTOS documentation
static void iprxisr_task(void *pvParameters)
{
    for( ;; )
    {
        vTaskDelay(1);
    }

}

void ENET_Receive_IRQHandler(void);
void ENET_IRQHandler(void)  // overrides PUBWEAK declaration in startup_MIMXRT1021.s with this function
{

	last_ethernet_packet_timestamp = get_microseconds_tick();	
    ENET_Receive_IRQHandler();      // handler is in fsl_enet.c.
}


// This initialises the Phy and MAC. It is shamelessly based on code lifted from enet_txrx_transfer.c which
// is an example driver from NXP
BaseType_t InitialiseNetwork(void)
{
	return 0;
}


// Set a DHCP Hostname
#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
/* DHCP has an option for clients to register their hostname.  */
const char *pcApplicationHostnameHook( void )
{
    return HUB2_HOSTNAME; 
}
#endif /* ipconfigDHCP_REGISTER_HOSTNAME */


