// added by Chris Cowdery to configure MAC
// 09/05/2019
//
#ifndef __MAC_USER_BUILD_CONFIG__
#define __MAC_USER_BUILD_CONFIG__

/**
 * FEATURE SET FOR FFD BUILD - taken from mac_build_config for an FFD and amended for Hub2
 */
#define MAC_ASSOCIATION_INDICATION_RESPONSE     (1)
#define MAC_ASSOCIATION_REQUEST_CONFIRM         (1)
#define MAC_BEACON_NOTIFY_INDICATION            (1)
#define MAC_DISASSOCIATION_BASIC_SUPPORT        (1)
#define MAC_DISASSOCIATION_FFD_SUPPORT          (1)
#define MAC_GET_SUPPORT                         (1)
#define MAC_INDIRECT_DATA_BASIC                 (1)
#define MAC_INDIRECT_DATA_FFD                   (1)
#define MAC_ORPHAN_INDICATION_RESPONSE          (1)
#define MAC_PAN_ID_CONFLICT_AS_PC               (1)
#define MAC_PAN_ID_CONFLICT_NON_PC              (1)
#define MAC_PURGE_REQUEST_CONFIRM               (0)
#define MAC_RX_ENABLE_SUPPORT                   (0)
#define MAC_SCAN_ACTIVE_REQUEST_CONFIRM         (1)
#define MAC_SCAN_ED_REQUEST_CONFIRM             (1)
#define MAC_SCAN_ORPHAN_REQUEST_CONFIRM         (1)
#ifdef BEACON_SUPPORT
#define MAC_SCAN_PASSIVE_REQUEST_CONFIRM        (1)
#else
#define MAC_SCAN_PASSIVE_REQUEST_CONFIRM        (0) /* No Passive Scan in
	                                             * nonbeacon networks */
#endif  /* BEACON_SUPPORT / No BEACON_SUPPORT */
#define MAC_START_REQUEST_CONFIRM               (1)

#ifdef BEACON_SUPPORT

/**
 * FEATURE SET FOR BUILD WITH BEACON ENABLED NETWORK SUPPORT
 */
#define MAC_SYNC_REQUEST                        (0) // changed by Chris
#else

/**
 * FEATURE SET FOR BUILD WITH PURE NONBEACON ONLY NETWORK SUPPORT
 */
#define MAC_SYNC_REQUEST                        (0) /* No Tracking of beacon
	                                             * frames for nonbeacon
	                                             * networks */
#endif /* BEACON_SUPPORT / No BEACON_SUPPORT */

#if (defined BEACON_SUPPORT) && (defined GTS_SUPPORT)

/**
 * FEATURE SET FOR BUILD WITH BEACON ENABLED NETWORK SUPPORT
 */
#define MAC_GTS_REQUEST                        (1)
#else

/**
 * FEATURE SET FOR BUILD WITH PURE NONBEACON ONLY NETWORK SUPPORT
 */
#define MAC_GTS_REQUEST                        (0)

#endif /* BEACON_SUPPORT / No BEACON_SUPPORT */

/*
 * Sync Loss Indication primitive is always required:
 *
 * In Beacon-enabled networks this is required in case
 * MAC_SYNC_REQUEST is set to 1.
 *
 * In Beacon- and Nonbeacon-enabled networks this is required in
 * case the peer node, acting as coordinator has MAC_START_REQUEST_CONFIRM
 * set to 1.
 */
#define MAC_SYNC_LOSS_INDICATION                    (1)


#endif
