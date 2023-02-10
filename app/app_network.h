/***************************************************************************//**
 * @file app_network.h
 * @brief This file contains headers which facilitate the wireless communication
 * between different Thunderboards.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_NETWORK_H
#define APP_NETWORK_H

#include "rail_types.h"
#include "app_config.h"

///The size of the payload (i.e., bytes with useful information) in the exchanged packets. It should get values between 16 or greater
#define TX_PAYLOAD_LENGTH 16

/// The size of the TX & RX FIFOs.
#define RAIL_FIFO_SIZE (256U)

///A buffer with the payload of the transmitted packet.
uint8_t tx_packet[TX_PAYLOAD_LENGTH];

/** Set up the rail TX FIFO for later usage.
 *
 * @date 10/01/2023
 * @param rail_handle A handle to the RAIL instance to be updated.
 */
void set_up_tx_fifo(RAIL_Handle_t rail_handle);

/** This function prepares a packet with the payload stored in {@link tx_packet}
 * buffer, and transmits it.
 *
 * @date 10/01/2023
 * @param rail_handle The RAIL instance to be used for TX FIFO writing.
 * @param destination The channel that will be used to transmit the message.
 */
void send_packet(RAIL_Handle_t rail_handle, uint16_t destination);
  
/** This function opens this board's channel for receiving.
 *
 * @date 10/01/2023
 * @param rail_handle The RAIL instance to be used for receiving packets.
 */
void start_receiving (RAIL_Handle_t rail_handle);
  
/** This function receives the packet, processes it and frees the RX FIFO.
 *
 * @date 10/01/2023
 * @param rail_handle The RAIL instance used for receiving packets.
 */
void handle_received_packet (RAIL_Handle_t rail_handle);

/** The RAIL callback, which is called if a RAIL event occurs.
 *
 * @date 10/01/2023
 * @param rail_handle The RAIL handle associated with the RAIL event
 * notification.
 * @param events The RAIL events having occurred.
 */
void sl_rail_util_on_event(RAIL_Handle_t rail_handle, RAIL_Events_t events);

#endif  // APP_NETWORK_H
