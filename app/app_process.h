/***************************************************************************//**
 * @file app_process.h
 * @brief The header file for the main process of this application.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#include "rail_types.h"

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/** Contains the status of RAIL Calibration */
volatile RAIL_Status_t calibration_status;
/**In case of a RX/TX error, it contains the code of the error.*/
volatile uint64_t error_code;
/**Becomes true when a new packet has been received.*/
volatile bool packet_received;
/**Become true when the transmission of a packet has been completed.*/
volatile bool packet_sent ;
/**Becomes true when an error was encountered during the reception of a new packet.*/
volatile bool rx_error ;
/**Becomes true when an error was encountered during the transmission of a new packet.*/
volatile bool tx_error ;
/**Becomes true when an error was encountered during the calibration of this board.*/
volatile bool cal_error;

/**Stores the board which started the execution of the distributed Average Consensus algorithm.*/
int8_t starting_board;

/**The baton, moving from one board to another and granting the priviledge of transmission.*/
bool baton;
/**The board which will receive the baton, when releazed by this one. Does not need initialization.*/
int8_t dst_of_baton;
/**Counts the number of batons received until now. It is initialized at the beginning of every task.*/
int baton_cntr;



// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/** The function is used for Application logic.
 *
 * @date 02/02/2023
 * @param rail_handle The RAIL instance to be used for TX-RX of packets.
 *
 * The function is used for Application logic and contains the application's
 * state machine. It is called infinitely.
 */
void app_process_action(RAIL_Handle_t rail_handle);

/** The function handles the payload of a received message.
 *
 * @date 02/02/2023
 * @param rx_buffer The buffer with the payload of the received message.
 */
void handle_rx_packet_payload(const uint8_t * const rx_buffer);

/** Initializes the variables of this application and prepares it for execution.
 *
 * @date 02/02/2023
 * @param rail_handle The RAIL instance to be used for TX-RX of packets.
 *
 * The function initializes the variables of this application and calls the
 * necessary functions in order for the application to be ready to be executed.
 */
void initialize_app(RAIL_Handle_t rail_handle);

#endif  // APP_PROCESS_H
