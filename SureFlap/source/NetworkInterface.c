/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright © 2013-2021 Sureflap Limited.
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

/* Hardware includes */
#include "board.h"
#include "fsl_enet.h"
#include "fsl_phy.h"        // KSZ8081RNB
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

#include "Hermes-app.h"
#include "debug.h"
#include "hermes-time.h"
	 
#define CORE_CLK_FREQ  CLOCK_GetFreq(kCLOCK_AhbClk)
#define ENET_RXBD_NUM (4)
#define ENET_TXBD_NUM (4)
#define ENET_RXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
#ifndef APP_ENET_BUFF_ALIGNMENT
#define APP_ENET_BUFF_ALIGNMENT ENET_BUFF_ALIGNMENT
#endif

BaseType_t 					InitialiseNetwork(void); // prototype
uint32_t 					RX_Int_Userdata;       // not sure what this is for yet...
extern EventGroupHandle_t	xConnectionStatus_EventGroup;
enet_handle_t 				g_handle;


// Note that this non cacheable stuff does not work. Instead, the cache is actively managed by defining FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL
/*! @brief Buffer descriptors should be in non-cacheable region and should be align to "ENET_BUFF_ALIGNMENT". */
AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);
/*! @brief The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and "ENET_BUFF_ALIGNMENT"
 * If use non-cache region, the alignment size is the "ENET_BUFF_ALIGNMENT".
 */
SDK_ALIGN(uint8_t g_rxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT) @ "ITCM";
SDK_ALIGN(uint8_t g_txDataBuff[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT) @ "ITCM";

extern NetworkAddressingParameters_t xNetworkAddressing;

static void iprxisr_task(void *pvParameters);
static TaskHandle_t xIpRxIsr_task_handle = NULL;
static void network_watchdog_task(void *pvParameters);
static TaskHandle_t xNetworkWatchdog_task_handle = NULL;
volatile uint32_t last_ethernet_packet_timestamp = 0;
QueueHandle_t xRestartNetworkMailbox;

// This function initialises the MAC and PHY using helper functions in fsl_enet.c and fsl.phy.c
// Note it can be called in two scenarios:
// 1. When the system starts,
// 2. When the network restarts, e.g. a network cable is plugged back in.
// In the latter case, it doesn't have to do so much initialisation.
BaseType_t xNetworkInterfaceInitialise( void )
{
	extern PRODUCT_CONFIGURATION product_configuration;
	static bool xFirstTime = true;
    BaseType_t xReturn = pdPASS;
	UBaseType_t xCurrentPriority;
	BaseType_t xRetVal;

	if( true == xFirstTime)
	{
		xRestartNetworkMailbox	= xQueueCreate(1, sizeof(bool));	// only make this once		
		
		// now start the deferred interrupt handler task
		if( xTaskCreate(iprxisr_task, "IpRxISR", ipISR_TASK_STACK_SIZE_WORDS, NULL, ISR_TASK_PRIORITY, &xIpRxIsr_task_handle) != pdPASS )
		{
			zprintf(CRITICAL_IMPORTANCE, "IP RX ISR task creation failed!.\r\n");
			xReturn = pdFAIL;
		}
		// And the Network Watchdog task
		if( PRODUCT_CONFIGURED == product_configuration.product_state)
		{
			if( xTaskCreate(network_watchdog_task, "Network Watchdog", NETWORK_WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xNetworkWatchdog_task_handle) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE, "Network Watchdog task creation failed!.\r\n");
				xReturn = pdFAIL;
			}		
		}
	}

	if( xReturn != pdFAIL)
	{	// Temporarily lower our task priority to the round-robin priority of the other tasks.
		xCurrentPriority = uxTaskPriorityGet(NULL);
		vTaskPrioritySet(NULL,NORMAL_TASK_PRIORITY);	// put our priority to the same as 
														// other tasks as this call blocks...
		xRetVal = InitialiseNetwork();
		vTaskPrioritySet(NULL,xCurrentPriority); // put our priority back to what it was
			
		if( xRetVal == pdFAIL )
    	{
        	xReturn = pdFAIL;
    	}
	}
	
	xFirstTime = false;	// future invocations of this call tree will do less initialisation
	
    return xReturn;
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
	uint32_t 						ethernet_timestamp_copy;
	bool							restart_flag;

	while(1)
	{
		ethernet_timestamp_copy = last_ethernet_packet_timestamp; // This copy is essential. Without
		// it, the value in last_ethernet_packet_timestamp may be updated in the interrupt _after_
		// the call to get_microseconds_tick() is made. This causes the result to go negative
		// and because they are unsigned numbers, in effect become a large positive number.
		if( (get_microseconds_tick()-ethernet_timestamp_copy) > (120 * usTICK_SECONDS))	// The AWS keepalive is 90 seconds
		{	// Note this is probably redundant as MQTT should identify an issue before this triggers
			zprintf(HIGH_IMPORTANCE,"No Ethernet traffic timeout - restart network\r\n");
			last_ethernet_packet_timestamp = get_microseconds_tick();
			FreeRTOS_NetworkDown(); 	// this will cause the IP task to restart the network
		}		
		if( pdPASS == xQueueReceive(xRestartNetworkMailbox, &restart_flag, 0) )
		{
			if( true == restart_flag)
			{	// restart network if requested to do so by the MQTT task
				zprintf(HIGH_IMPORTANCE,"MQTT requested restart network\r\n");
				last_ethernet_packet_timestamp = get_microseconds_tick(); // supress any imminent timeouts of lack of rx packets			
				FreeRTOS_NetworkDown(); 	// this will cause the IP task to restart the network
			}
		}
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
	bool link;

	if( kStatus_Success == PHY_GetLinkStatus(ENET, ENET_PHY_ADDR, &link) )
	{
		if( link == true )
		{
			return pdPASS;
		} else
		{
			return pdFAIL;
		}
	}

	zprintf(HIGH_IMPORTANCE, "Can't talk to PHY. You should never see me.\r\n");
	return pdFAIL;
}

phy_speed_t phy_speed;	 // These are externed from the CLI command 'version' 
phy_duplex_t phy_duplex; // for diagnostic purposes. 
// This function transmits an Ethernet frame
BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend )
{
    static volatile uint32_t count=0;
    bool link;

	PHY_GetLinkSpeedDuplex(ENET, ENET_PHY_ADDR, &phy_speed, &phy_duplex);
	
    if( kStatus_Success == PHY_GetLinkStatus(ENET, ENET_PHY_ADDR, &link) )
    {
        if( link )
        {
            if( kStatus_Success != ENET_SendFrame(ENET, &g_handle, pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength) )
            {
                zprintf(LOW_IMPORTANCE, "Frame dropped (send failure)\r\n");
            }
        } else
		{
            zprintf(LOW_IMPORTANCE, "Link down\r\n");
		}
    } else
    {
        zprintf(HIGH_IMPORTANCE, "Failure to communicate with Phy over RMII\r\n");
    }
    /* Call the standard trace macro to log the send event. */
    iptraceNETWORK_INTERFACE_TRANSMIT();

    if( xReleaseAfterSend != pdFALSE )
    {
        /* The Ethernet buffer is no longer needed, and must be
        freed for re-use. */
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
    }

    return pdTRUE;
}

// This function is used by the BufferAllocation1.c subsystem
/* First statically allocate the buffers, ensuring an additional ipBUFFER_PADDING
bytes are allocated to each buffer.  This example makes no effort to align
the start of the buffers, but most hardware will have an alignment requirement.
If an alignment is required then the size of each buffer must be adjusted to
ensure it also ends on an alignment boundary.  Below shows an example assuming
the buffers must also end on an 8-byte boundary. */
#define BUFFER_SIZE ( ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING )
#define BUFFER_SIZE_ROUNDED_UP ( ( BUFFER_SIZE + 7 ) & ~0x07UL )
static uint8_t ucBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ][ BUFFER_SIZE_ROUNDED_UP ] @ "ITCM";

/* Next provide the vNetworkInterfaceAllocateRAMToBuffers() function, which
simply fills in the pucEthernetBuffer member of each descriptor. */
void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    BaseType_t x;

    for( x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++ )
    {
        /* pucEthernetBuffer is set to point ipBUFFER_PADDING bytes in from the
        beginning of the allocated buffer. */
        pxNetworkBuffers[ x ].pucEthernetBuffer = &( ucBuffers[ x ][ ipBUFFER_PADDING ] );

        /* The following line is also required, but will not be required in
        future versions. */
        *( ( uint32_t * ) &ucBuffers[ x ][ 0 ] ) = ( uint32_t ) &( pxNetworkBuffers[ x ] );
    }
}

bool NetworkInterface_IsActive(void)
{
	return (0 != (CONN_STATUS_NETWORK_UP & xEventGroupGetBits(xConnectionStatus_EventGroup)));
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    switch (eNetworkEvent)
    {
        case eNetworkDown:
            zprintf(LOW_IMPORTANCE,"Network down.\r\n");
			xEventGroupClearBits(xConnectionStatus_EventGroup, CONN_STATUS_NETWORK_UP);
            break;
        case eNetworkUp:
//            zprintf(LOW_IMPORTANCE,"Network up.\r\n");
			xEventGroupSetBits(xConnectionStatus_EventGroup, CONN_STATUS_NETWORK_UP);
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
    NetworkBufferDescriptor_t *pxBufferDescriptor;
    size_t xBytesReceived;
    /* Used to indicate that xSendEventStructToIPTask() is being called because
    of an Ethernet receive event. */
    IPStackEvent_t xRxEvent;
	status_t status;

    for( ;; )
    {
        /* Wait for the Ethernet MAC interrupt to indicate that another packet
        has been received.  The task notification is used in a similar way to a
        counting semaphore to count Rx events, but is a lot more efficient than
        a semaphore. */
        ulTaskNotifyTake( pdFALSE, portMAX_DELAY );

        /* See how much data was received.  Here it is assumed ReceiveSize() is
        a peripheral driver function that returns the number of bytes in the
        received Ethernet frame. */

        do
        {
            // So this ought to only run if there actually is data...
            status = ENET_GetRxFrameSize(&g_handle, &xBytesReceived); /* Get the Frame size */

			if( kStatus_Success == status)
			{
				if( xBytesReceived > 0 )
				{
					/* Allocate a network buffer descriptor that points to a buffer
					large enough to hold the received frame.  As this is the simple
					rather than efficient example the received data will just be copied
					into this buffer. */
					pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( xBytesReceived, 0 );

					if( pxBufferDescriptor != NULL )
					{
						/* pxBufferDescriptor->pucEthernetBuffer now points to an Ethernet
						buffer large enough to hold the received data.  Copy the
						received data into pcNetworkBuffer->pucEthernetBuffer.  Here it
						is assumed ReceiveData() is a peripheral driver function that
						copies the received data into a buffer passed in as the function's
						parameter.  Remember! While is is a simple robust technique -
						it is not efficient.  An example that uses a zero copy technique
						is provided further down this page. */
						// We could switch to a zero copy system as we know that the data is stored in
						// g_rxDataBuff[0-3], however ENET_ReadFrame() sets ENET->RDAR which
						// tells the MAC that the buffer is free. So we'd need a mechanism to postpone
						// ENET->RDAR part.
						pxBufferDescriptor->stage = NB_FLAGGED_TO_RECEIVE;
						ENET_ReadFrame(ENET, &g_handle, pxBufferDescriptor->pucEthernetBuffer, xBytesReceived);
						pxBufferDescriptor->xDataLength = xBytesReceived;

						/* See if the data contained in the received Ethernet frame needs
						to be processed.  NOTE! It is preferable to do this in
						the interrupt service routine itself, which would remove the need
						to unblock this task for packets that don't need processing. */
						if( (eConsiderFrameForProcessing(pxBufferDescriptor->pucEthernetBuffer) == eProcessBuffer) &&
							(SureFilterFramesEarly(pxBufferDescriptor->pucEthernetBuffer) == eProcessBuffer) )
						{
							/* The event about to be sent to the TCP/IP is an Rx event. */
							xRxEvent.eEventType = eNetworkRxEvent;
							/* pvData is used to point to the network buffer descriptor that
							now references the received data. */
							xRxEvent.pvData = ( void * ) pxBufferDescriptor;

							pxBufferDescriptor->stage = NB_FLAGGED_FOR_HANDLING;
							/* Send the data to the TCP/IP stack. */
							if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE )
							{
								/* The buffer could not be sent to the IP task so the buffer
								must be released. */
								pxBufferDescriptor->stage = NB_FLAGGED_UNHANDLED;
								vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );

								/* Make a call to the standard trace macro to log the
								occurrence. */
	//                            iptraceETHERNET_RX_EVENT_LOST();
							}
							else
							{
								/* The message was successfully sent to the TCP/IP stack.
								Call the standard trace macro to log the occurrence. */
								iptraceNETWORK_INTERFACE_RECEIVE();
							}
						}
						else
						{
							/* The Ethernet frame can be dropped, but the Ethernet buffer
							must be released. */
							pxBufferDescriptor->stage = NB_FLAGGED_UNHANDLED;
							vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
						}
					}
					else
					{
						/* The event was lost because a network buffer was not available.
						Call the standard trace macro to log the occurrence. */
	//                    iptraceETHERNET_RX_EVENT_LOST();
						xBytesReceived = 0;
					}
				}
				else // a valid packet with no bytes was received
				{
					zprintf(LOW_IMPORTANCE,"Valid, zero length\r\n");
					ENET_ReadFrame(ENET, &g_handle, NULL, xBytesReceived);	// flush receive buffer
				}
			}
			else	// status != kStatusSuccess - so something went wrong with packet reception
			{
				//zprintf(CRITICAL_IMPORTANCE,"Bad packet\r\n"); // We get a lot of these
				ENET_ReadFrame(ENET, &g_handle, NULL, xBytesReceived);	// flush receive buffer				
			}
        } while(xBytesReceived!=0); // loop processing packets
    }

}

void Eth_RX_Callback(ENET_Type *base, enet_handle_t *g_handle, enet_event_t event, void *userData)
{ // this is where the ISR ends up.
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    switch (event)
    {
        case kENET_RxEvent:            /*!< Receive event. */
            vTaskNotifyGiveFromISR( xIpRxIsr_task_handle, &pxHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(pxHigherPriorityTaskWoken); // Once this ISR is finished,
                                        // PendSV will fire, and switch
                                        // to the highest priority task, which handles this data
            break;
        case kENET_TxEvent:            /*!< Transmit event. */
            //zprintf(CRITICAL_IMPORTANCE,"TX INT (SHOULD NEVER HAPPEN!)\r\n");
            break;
        case kENET_ErrEvent:           /*!< Error event: BABR/BABT/EBERR/LC/RL/UN/PLR . */
            //zprintf(MEDIUM_IMPORTANCE,"RX Error callback\r\n");
            break;
        default:
            break;
    }
}

void ENET_Receive_IRQHandler(void);
void ENET_IRQHandler(void)  // overrides PUBWEAK declaration in startup_MIMXRT1021.s with this function
{
  	LED_SET(LED_DISP_ETH);	
	last_ethernet_packet_timestamp = get_microseconds_tick();	
    ENET_Receive_IRQHandler();      // handler is in fsl_enet.c.
}


// This initialises the Phy and MAC. It is shamelessly based on code lifted from enet_txrx_transfer.c which
// is an example driver from NXP
BaseType_t InitialiseNetwork(void)
{
    enet_config_t config;
    bool link = false;
	bool FirstTime = true;
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t sysClock;
    status_t status;

    /* pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO_PORT_ENET_INT, GPIO_PIN_ENET_INT, 1);
    GPIO_WritePinOutput(GPIO_PORT_ENET_RST, GPIO_PIN_ENET_RST, 0);
    // Increase the delay from 20 to 100mS. Fix occasional TCP test failure on the Factory Programmer
    delay_ms(100);	// allow the PHY to initialise
    GPIO_WritePinOutput(GPIO_PORT_ENET_RST, GPIO_PIN_ENET_RST, 1);

    // Phy is now out of reset

    ENET_GetDefaultConfig(&config); // populate &config with some default values
    // NOTE THIS IS WHERE THE FIFO THRESHOLDS ARE SET - THIS FN SETS THEM TO ZERO

    sysClock = CORE_CLK_FREQ;
    status = PHY_Init(ENET, ENET_PHY_ADDR, sysClock); // try to initialise the Phy

	if( kStatus_Success == status)
    {
		PHY_GetLinkStatus(ENET, ENET_PHY_ADDR, &link);
		if (link)
		{
			/* Get the actual PHY link speed. */
			PHY_GetLinkSpeedDuplex(ENET, ENET_PHY_ADDR, &speed, &duplex);
			/* Change the MII speed and duplex for actual link status. */
			config.miiSpeed = (enet_mii_speed_t)speed;
			config.miiDuplex = (enet_mii_duplex_t)duplex;
			config.interrupt = ENET_RX_INTERRUPT;   // This means that ENET_Receive_IRQHandler() will eventually call
													// our callback that we set in ENET_SetCallback()
		}

		enet_buffer_config_t buffConfig[] = 
		{
			{
				ENET_RXBD_NUM,
				ENET_TXBD_NUM,
				SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
				SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
				&g_rxBuffDescrip[0],
				&g_txBuffDescrip[0],
				&g_rxDataBuff[0][0],
				&g_txDataBuff[0][0],
			}
		};

		// Note that this approach to getting the MAC address makes assumptions that the stack has initialised the
		// variable that is read by FreeRTOS_GetMACAddress()
		ENET_Init(ENET, &g_handle, &config, &buffConfig[0], (uint8_t *)FreeRTOS_GetMACAddress(), sysClock);
		ENET_SetCallback(&g_handle, Eth_RX_Callback, &RX_Int_Userdata);
		if( true == FirstTime)
		{	
			// Now set up interrupt for receive
			NVIC_SetPriority(ENET_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // we use this priority as NVIC_SetPriority
																						// wants a raw priority which it bitshifts to go into
																						// the NVIC priority register, whereas the
																						// configMAX_SYSCALL_INTERRUPT_PRIORITY has already
																						// been shifted.
			ENET_EnableInterrupts(ENET,kENET_RxFrameInterrupt); // Indicates a frame has been received and the last corresponding buffer descriptor has been updated.
			FirstTime = false;
		}

		ENET_ActiveRead(ENET);      // Activates the Ethernet Receiver......
//		zprintf(CRITICAL_IMPORTANCE,"InitialiseNetwork: Link up\r\n");
		return pdTRUE;  
	}
	else
	{
//		zprintf(CRITICAL_IMPORTANCE,"InitialiseNetwork: Link down\r\n");
		return pdFALSE;	// failed to set up link
	}
}


// Set a DHCP Hostname
#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
/* DHCP has an option for clients to register their hostname.  */
const char *pcApplicationHostnameHook( void )
{
    return HUB2_HOSTNAME; 
}
#endif /* ipconfigDHCP_REGISTER_HOSTNAME */


