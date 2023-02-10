/***************************************************************************//**
 * @file app_network.c
 * @brief This file contains the implementation of functions which facilitate
 * the wireless communication between different Thunderboards.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "app_network.h"
#include "rail.h"
#include "app_assert.h"
#include "app_log.h"
#include "app_process.h"

///TX FIFO
static union {
  RAIL_FIFO_ALIGNMENT_TYPE align[RAIL_FIFO_SIZE / RAIL_FIFO_ALIGNMENT];  // Used to align this buffer as needed
  uint8_t fifo[RAIL_FIFO_SIZE];} tx_fifo;

///RX FIFO
uint8_t rx_fifo[RAIL_FIFO_SIZE];


/** Prints in the console the payload of the received packet in hex format.
 *
 * @date 10/01/2023
 * @param rx_buffer A buffer with the payload of the received packet.
 */
static void printf_rx_packet(const uint8_t * const rx_buffer){
	app_log_info("Packet has been received: ");
	for (uint16_t i = 0; i < TX_PAYLOAD_LENGTH; i++)
		app_log_info("0x%02X, ", rx_buffer[i]);
	app_log_info("\n");
}

/** Prints in the console the payload of the transmitted packet in hex format.
 *
 * @date 10/01/2023
 * @param tx_buffer A buffer with the payload of the transmitted packet.
 */
static void printf_tx_packet(const uint8_t * const tx_buffer){
	app_log_info("Packet has been transmitted: ");
	for (uint16_t i = 0; i < TX_PAYLOAD_LENGTH; i++)
		app_log_info("0x%02X, ", tx_buffer[i]);
	app_log_info("\n");
}

 /******************************************************************************
  * Set up the rail TX fifo for later usage
  *****************************************************************************/
 void set_up_tx_fifo(RAIL_Handle_t rail_handle) {
	uint16_t allocated_tx_fifo_size = 0;
	allocated_tx_fifo_size = RAIL_SetTxFifo(rail_handle, tx_fifo.fifo, 0, RAIL_FIFO_SIZE);
	app_assert(allocated_tx_fifo_size == RAIL_FIFO_SIZE,
			  "RAIL_SetTxFifo() failed to allocate a large enough fifo (%d bytes instead of %d bytes)\n",
			  allocated_tx_fifo_size,
			  RAIL_FIFO_SIZE);
 }

/** This function prepares the packet for tx, and loads it in the RAIL TX FIFO.
 *
 * @date 10/01/2023
 * @param rail_handle The RAIL instance to be used for TX FIFO writing.
 * @param out_data The buffer with the packet's payload.
 * @param length The length of the payload.
 */
static void prepare_package(RAIL_Handle_t rail_handle, uint8_t *out_data, uint16_t length){
	uint16_t bytes_writen_in_fifo = 0;
	bytes_writen_in_fifo = RAIL_WriteTxFifo(rail_handle, out_data, length, true);
	app_assert(bytes_writen_in_fifo == TX_PAYLOAD_LENGTH,
			  "RAIL_WriteTxFifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
			  bytes_writen_in_fifo,
			  TX_PAYLOAD_LENGTH);
}

/******************************************************************************
 * This function prepares the packet for transmission, and also transmits it.
 *****************************************************************************/
void send_packet(RAIL_Handle_t rail_handle ,uint16_t destination){
	RAIL_Status_t rail_status;
	RAIL_PrepareChannel(rail_handle,  destination);

	prepare_package(rail_handle, tx_packet, sizeof(tx_packet));
	rail_status = RAIL_StartTx(rail_handle, destination, RAIL_TX_OPTIONS_DEFAULT, NULL);
//	printf_tx_packet(tx_packet); //Uncomment for easier debugging

	if (rail_status != RAIL_STATUS_NO_ERROR)
		app_log_warning("RAIL_StartTx() result:%d ", rail_status);

	if(false) //just to suppress the warning of unused static function
		printf_tx_packet(tx_packet);
 }

/******************************************************************************
 * This function opens the channel of the current board for receiving.
 *****************************************************************************/
void start_receiving (RAIL_Handle_t rail_handle){
	RAIL_Status_t rail_status = RAIL_StartRx (rail_handle, BOARD_ID, NULL);
	if(rail_status!=0)
		app_log_warning("RAIL_StartRx() result:%d\n", rail_status);
}

/** This function helps to unpack the received packet, points to the payload,
 * and returns its length.
 *
 * @date 10/01/2023
 * @param rx_destination A buffer where the full packet will be stored.
 * @param packet_information Pointer to a structure where the packet's information
 * will be stored.
 * @param start_of_payload Pointer to the beginning of the payload.
 *
 * @return The length of the received payload.
 */
static uint16_t unpack_packet(uint8_t *rx_destination, const RAIL_RxPacketInfo_t *packet_information, uint8_t **start_of_payload){
	RAIL_CopyRxPacket(rx_destination, packet_information);
	*start_of_payload = rx_destination;
	return ((packet_information->packetBytes > RAIL_FIFO_SIZE)? RAIL_FIFO_SIZE : packet_information->packetBytes);
}

/******************************************************************************
 * This function receives the packet, processes it and frees the RX FIFO.
 *****************************************************************************/
void handle_received_packet (RAIL_Handle_t rail_handle){
	RAIL_RxPacketHandle_t rx_packet_handle;
	RAIL_RxPacketInfo_t packet_info;
	RAIL_Status_t rail_status = RAIL_STATUS_NO_ERROR;  // Status indicator of the RAIL API calls

	rx_packet_handle = RAIL_GetRxPacketInfo(rail_handle, RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
	while (rx_packet_handle != RAIL_RX_PACKET_HANDLE_INVALID) {
		uint8_t *start_of_packet = 0;
		uint16_t packet_size = unpack_packet(rx_fifo, &packet_info, &start_of_packet);
		if(packet_size!=TX_PAYLOAD_LENGTH){
			app_log_error("Error. Invalid length (%d) of the received packet's payload.\n", packet_size);
			rx_packet_handle = RAIL_GetRxPacketInfo(rail_handle, RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
			continue;
		}

		rail_status = RAIL_ReleaseRxPacket(rail_handle, rx_packet_handle);
		if (rail_status != RAIL_STATUS_NO_ERROR)
			app_log_warning("RAIL_ReleaseRxPacket() result:%d", rail_status);

//		printf_rx_packet(start_of_packet); //Uncomment for easier debugging
		if(start_of_packet[2]==BOARD_ID)  //Necessary check, to ensure that the message was transmitted for me.
			handle_rx_packet_payload(start_of_packet);

		rx_packet_handle = RAIL_GetRxPacketInfo(rail_handle, RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);

		if(false) //just to suppress the warning of unused static function
			printf_rx_packet(start_of_packet);
	}
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs.
 *****************************************************************************/
void sl_rail_util_on_event(RAIL_Handle_t rail_handle, RAIL_Events_t events){
	error_code = events;

	if(events & RAIL_EVENTS_RX_COMPLETION) { //Handle Rx events
		if (events & RAIL_EVENT_RX_PACKET_RECEIVED) { //Keep the packet in the radio buffer, download it later at the state machine
			RAIL_HoldRxPacket(rail_handle);
			packet_received = true;
		}
		else  //Handle Rx error
			rx_error = true;
	}

	if(events & RAIL_EVENTS_TX_COMPLETION) { // Handle Tx events
		if(events & RAIL_EVENT_TX_PACKET_SENT)
			packet_sent = true;
		else  // Handle Tx error
			tx_error = true;
	}

	if(events & RAIL_EVENT_CAL_NEEDED) { // Perform all calibrations when needed
		calibration_status = RAIL_Calibrate(rail_handle, NULL, RAIL_CAL_ALL_PENDING);
		if(calibration_status != RAIL_STATUS_NO_ERROR)
			cal_error = true;
	}
}
