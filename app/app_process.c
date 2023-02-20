/***************************************************************************//**
 * @file app_process.c
 * @brief The implementation file for the state machine of this application.
 * @author Georgios Apostolakis
 ******************************************************************************/
#include "app_process.h"
#include <math.h>
#include "rail.h"
#include "app_log.h"
#include "app_network.h"
#include "app_stack.h"
#include "app_tools.h"
#include "app_consensus.h"

// -----------------------------------------------------------------------------
//                   Definitions of Constants and Typedefs
// -----------------------------------------------------------------------------
/** The various states from the state machine of the application.
* - S_RESTART_COMPLETED: When the board enters this state, it has completed a re-initialization and is going to start the average consensus task from the beginning.
* - S_START_AVG_CONSENSUS: The first state of the average consensus task, where the algorithm is initialized.
* - S_SEND_AVG_CONSENSUS_MSGS: The board enters this state during the average consensus task, and sends its state to all the commuting boards (according to the system's graph).
* - S_UPDATE_AVG_CONSENSUS_STATE: The board enters this state during the average consensus task, and updates its state.
* - S_INIT_AND_SLEEP: The last state of the board before it sleeps, where it initializes itself.
* - S_PACKET_TX: A generic state where the board transmits a message (whose exact type depends on the {@link tx_operation_to_achieve} variable.
* - S_PACKET_RECEIVED: A generic state which handles the received packets.
* - S_PACKET_SENT: A generic state which makes some necessary actions after a transmission has been completed.
* - S_RX_PACKET_ERROR: A generic state which handles the case of an error to the reception of a packet.
* - S_TX_PACKET_ERROR: A generic state which handles the case of an error to the transmission of a packet.
* - S_CALIBRATION_ERROR: A generic state which handles the case of an error to the calibration of this board.
* - S_IDLE: A generic state where the board performs no action (necessary while, e.g., waits for a transmission to be completed).
*/
typedef enum {
	S_RESTART_COMPLETED,
	S_START_AVG_CONSENSUS,
	S_SEND_AVG_CONSENSUS_MSGS,
	S_UPDATE_AVG_CONSENSUS_STATE,
	S_INIT_AND_SLEEP,
	S_PACKET_TX,
	S_PACKET_RECEIVED,
	S_PACKET_SENT,
	S_RX_PACKET_ERROR,
	S_TX_PACKET_ERROR,
	S_CALIBRATION_ERROR,
	S_IDLE
} state_t;

/** The various operations that can be performed at the {@link state_t S_PACKET_TX} state.
* - O_GLB_RESTART: Send a message of type {@link message_t MSG_RESTART}.
* - O_GLB_START_TASK: Send a message of type {@link message_t MSG_START_TASK}.
* - O_GLB_SEND_STATE: Send a message of type {@link message_t MSG_CONSENSUS_STATE}.
* - O_GIVE_BATON: Send a message of type {@link message_t MSG_BATON}.
*/
typedef enum {
	O_GLB_RESTART,
	O_GLB_START_TASK,
	O_GLB_SEND_STATE,
	O_GIVE_BATON
} tx_operation_t;

/** The various (independent) tasks to be performed by the application.
* - T_NONE: No specific task except from answering incoming requests & handling incoming messages.
* - T_CONSENSUS: Contribute to the execution of distributed Average Consensus.
*/
typedef enum {
	T_NONE,
	T_CONSENSUS
} task_t;

/** The various types of messages exchanged between the boards.
 * - MSG_RESTART: A message indicating that the system is restarting at the moment.
 * - MSG_START_TASK: A message indicating that the system is starting a new task at the moment.
 * - MSG_CONSENSUS_STATE: A message with another board's current state.
 * - MSG_BATON: A message with the baton.
 */
typedef enum {
	MSG_RESTART,
	MSG_START_TASK,
	MSG_CONSENSUS_STATE,
	MSG_BATON
} message_t;

/// This constant at a specific index of some messages indicates that the distributed system is currently transitioning to sleep state.
#define SEND_SYSTEM_TO_SLEEP -1

// -----------------------------------------------------------------------------
//                              Static Variables
// -----------------------------------------------------------------------------
///Indicates the current state of the state machine.
static volatile state_t state = S_INIT_AND_SLEEP;

///Indicates the current task of the board.
static task_t current_task;

///Determines the exact kind of transmission which will be performed when the board is in the {@link state_t S_PACKET_TX} state. It does not need initialization.
static tx_operation_t tx_operation_to_achieve;

///Counts the number of pending messages for transmission, and is mainly used when the {@link state_t S_PACKET_TX} state has to repetitively send many messages. It does not need initialization.
static int num_of_pending_msgs_for_tx;

///Indicates how many batons are required to pass from this board in order for the baton to complete a full cycle and return to the beginning (see the baton_path variable).
static int batons_per_cycle;

///Counts the number of boards whose current state compared to the previous one gives a value under the STOP_THRESHOLD. When this variable is equal to NUM_OF_BOARDS, the system can stop the execution of the algorithm and sleep.
static int8_t boards_completed_their_task;

///This variable becomes true when this board believes that the execution of average consensus can be terminated, according to its own current & previous state.
static bool consensus_is_over;

// -----------------------------------------------------------------------------
//                        Static Function Declaration
// -----------------------------------------------------------------------------
/** The function transmits a packet to another board. The destination and the
 * exact structure of the packet are determined by the desired operation,
 * passed as an argument.
 *
 * @date 02/02/2023
 * @param rail_handle The RAIL instance to be used for TX.
 * @param oper The desired TX operation, which determines the destination and
 * the packet's structure.
 */
static bool packet_transmission (RAIL_Handle_t rail_handle, volatile tx_operation_t oper);

/** The function handles the unexpected events which affect the normal sequence
 * of the states (e.g., timer alarms, interrupts, cli commands).
 *
 * @date 02/02/2023
 * @param rail_handle The RAIL instance to be used for TX - RX.
 */
void handle_app_events(RAIL_Handle_t rail_handle);

/**
 * The function implements the state machine of this application.
 *
 * @date 02/02/2023
 * @param rail_handle The RAIL instance to be used for TX - RX.
 */
void execute_app_state(RAIL_Handle_t rail_handle);

// -----------------------------------------------------------------------------
//                          Function Implementation
// -----------------------------------------------------------------------------
/*******************************************************************************
 * The 'main' function of this application, called infinitely.
 ******************************************************************************/
void app_process_action(RAIL_Handle_t rail_handle){
	handle_app_events(rail_handle);
	execute_app_state(rail_handle);
}

/*******************************************************************************
  * Handles the unexpected events that affect the normal sequence of the states.
*******************************************************************************/
void handle_app_events(RAIL_Handle_t rail_handle){ //Handles 1 event per call. More than 1 may cause extreme edge cases that will crash the application.
	if (packet_received) { //EVENT WITH PRIOR. 1 - RECEIVED A NEW PACKET - HANDLE IT IMMEDIATELY
		packet_received = false;
		push(state);
		state = S_PACKET_RECEIVED;
	} else if (packet_sent) { //EVENT WITH PRIOR. 2 - COMPLETED TX OF A PACKET - NOT NECESSARY TO BE IDLE ANYMORE
		packet_sent = false;
		state = S_PACKET_SENT;
	} else if (rx_error) {  //EVENT WITH PRIOR. 3 - RECEIVED PACKET WITH ERRORS - FIND A WAY TO HANDLE THE SITUATION
		rx_error = false;
		state = S_RX_PACKET_ERROR;
	} else if (tx_error) { //EVENT WITH PRIOR. 4 - TRANSMITTED PACKET WITH ERRORS - FIND A WAY TO HANDLE THE SITUATION
		tx_error = false;
		state = S_TX_PACKET_ERROR;
	} else if (cal_error) { //EVENT WITH PRIOR. 5 - ERROR ON CALIBRATION OF THE BOARD - FIND A WAY TO HANDLE THE SITUATION
		cal_error = false;
		state = S_CALIBRATION_ERROR;
	} else if(baton && boards_completed_their_task==SEND_SYSTEM_TO_SLEEP && baton_cntr%batons_per_cycle==0){ //EVENT WITH PRIOR. 6 - THIS IS THE LAST BATON RECEIVED BY THE CURRENT BOARD - TRANSMIT THE BATON AND GO TO SLEEP
		clear(); //Clear any remaining states in the stack
		temperature = MIN_TEMPERATURE - 1; //Initialize any remaining variables
		current_task = T_NONE;
		baton_cntr = 0;
		consensus_is_over = false;
		push(S_INIT_AND_SLEEP);
		tx_operation_to_achieve = O_GIVE_BATON;
		state = S_PACKET_TX;
		app_log_info("\n\n=====================================================\n");
		app_log_info("Estimated average temperature: %.2f degrees Celsius.\n", consensus_states[BOARD_ID]);
		app_log_info("=====================================================\n\n\n");
	} else if(restart_command && baton){ //EVENT WITH PRIOR. 7 - THIS BOARD HAS TO RE-INITIALIZE SINCE THE WHOLE SYSTEM IS RESTARTING - RE-INITIALIZE IMMEDIATELY.
		app_log_info("=========================================================\n");
		app_log_info("Restarting...\n");
		app_log_info("=========================================================\n");

		int8_t hold_dst_of_baton = dst_of_baton;
		int8_t hold_start_brd = starting_board;
		int hold_restart_id = restart_id;
		initialize_app(rail_handle);
		starting_board = hold_start_brd;
		dst_of_baton = hold_dst_of_baton;
		baton_cntr = 1;
		baton = true;
		restart_id = hold_restart_id;

		num_of_pending_msgs_for_tx = NUM_OF_BOARDS;
		state = S_PACKET_TX;
		tx_operation_to_achieve = O_GLB_RESTART;
		push(S_RESTART_COMPLETED);
	} else if(baton && (baton_cntr-1)%batons_per_cycle!=0){ //EVENT WITH PRIOR. 8 - NO ACTION SHOULD BE PERFORMED ON THIS BATON - BYPASS THE BATON BY RELEASING IT IMMEDIATELY.
			push(state);
			tx_operation_to_achieve = O_GIVE_BATON;
			state = S_PACKET_TX;
	} else if(average_command && baton){ //EVENT WITH PRIOR. 9 - THE WHOLE SYSTEM IS STARTING THE EXECUTION OF THE DISTRIBUTED AVERAGE CONSENSUS ALGORITHM - START THE AVERAGE CONSENSUS ALGORITHM ON THE CURRENT BOARD.
		app_log_info("Starting the execution of Distributed Average Consensus.\n");
		average_command = false;
		num_of_pending_msgs_for_tx = NUM_OF_BOARDS;
		current_task = T_CONSENSUS;
		state = S_PACKET_TX;
		tx_operation_to_achieve = O_GLB_START_TASK;
		push(S_START_AVG_CONSENSUS);
	}
}

/*******************************************************************************
 * The state machine of this application.
 ******************************************************************************/
void execute_app_state(RAIL_Handle_t rail_handle){
	switch (state) {
	case S_RESTART_COMPLETED: //When the board enters this state, it has completed a re-initialization and is going to start the average consensus task from the beginning.
		if(starting_board==BOARD_ID)
			average_command = true;
		state = S_IDLE;
		break;
	case S_START_AVG_CONSENSUS: //The first state of the average consensus task, where the algorithm is initialized.
		if(baton){
			measure_temperature();
			initialize_consensus_setup();
			state = S_SEND_AVG_CONSENSUS_MSGS;
			app_log_info("Initialization complete!\n");
		}
		break;
	case S_SEND_AVG_CONSENSUS_MSGS:{ //The board enters this state during the average consensus task, and sends its state to all the commuting boards (according to the {@link ../config/app_config.h#graph graph}).
		if(baton){
				app_log_info("=========================================================\n");
			app_log_info("Iteration %d:\n", consensus_iters+1);
			app_log_info("   - Now sending my state to my neighbors.\n");
			push(S_UPDATE_AVG_CONSENSUS_STATE);
			num_of_pending_msgs_for_tx = NUM_OF_BOARDS;
			state = S_PACKET_TX;
			tx_operation_to_achieve = O_GLB_SEND_STATE;
		}

		break;}
	case S_UPDATE_AVG_CONSENSUS_STATE:{ //The board enters this state during the average consensus task, and updates its state.
		if(baton){
			float prev_state = consensus_states[BOARD_ID];
			update_consensus_state();
			app_log_info("   - Now updating my state, to %f.\n", consensus_states[BOARD_ID]);
			push(S_SEND_AVG_CONSENSUS_MSGS);
			state = S_PACKET_TX;
			tx_operation_to_achieve = O_GIVE_BATON;
			if(fabs(prev_state-consensus_states[BOARD_ID])<=STOP_THRESHOLD && !consensus_is_over){
				app_log_info("     This board has reached to a value below the threshold, and it agrees for the algorithm to be terminated.\n");
				consensus_is_over = true;
				boards_completed_their_task++;
			}
			else if(fabs(prev_state-consensus_states[BOARD_ID])>STOP_THRESHOLD && consensus_is_over && boards_completed_their_task!=SEND_SYSTEM_TO_SLEEP){
				app_log_info("     This board is no longer below the threshold, and it does NOT agree for the algorithm to be terminated.\n");
				consensus_is_over = false;
				boards_completed_their_task--;
			}
		}
		break;}
	case S_INIT_AND_SLEEP: //The last state of the board before it sleeps, where it initializes itself.
		initialize_app(rail_handle);
		app_log_info("Now going to sleep...\n");
		state = S_IDLE;
		sleep();
		break;
	case S_PACKET_TX:{ //A generic state where the board transmits a message (whose exact type depends on the {@link tx_operation_to_achieve} variable.
		bool msg_sent = packet_transmission(rail_handle, tx_operation_to_achieve); //this method actually builds & transmits the packet

		switch(tx_operation_to_achieve){ //post-transmission jobs to be done
		case O_GLB_START_TASK: //After starting a task or sending state messages, release baton immediately so that the rest of the neighbors do the same job too.
		case O_GLB_RESTART:
		case O_GLB_SEND_STATE:
			num_of_pending_msgs_for_tx--;
			push(S_PACKET_TX);
			if(num_of_pending_msgs_for_tx==0)
				tx_operation_to_achieve = O_GIVE_BATON;
			break;
		case O_GIVE_BATON:{
			baton=false;
			if(starting_board==BOARD_ID)
				RAIL_SetMultiTimer(&tmr0, RESTART_TIMEOUT_MILISECS*1000, RAIL_TIME_DELAY, &enable_alarm, NULL);
			app_log_info("                              Released BATON %d! %d boards have reached to a result under the threshold.\n", baton_cntr, boards_completed_their_task<0?NUM_OF_BOARDS:boards_completed_their_task);
			boards_completed_their_task = 0;
			break;}
		}

		if(msg_sent) //if a message was actually transmitted, wait until transmission is completed
			state = S_IDLE;
		else //no transmission is being executed, continue immediately to the next state
			state = pop();
		break;}
	case S_PACKET_RECEIVED: //A generic state to handle the received packets
		handle_received_packet (rail_handle);
		state = pop();
		break;
	case S_PACKET_SENT: //A generic state to handle the board after a transmission has been completed
		start_receiving(rail_handle);
		state = pop();
		break;
	case S_RX_PACKET_ERROR:  //A generic state to handle cases of an error to the reception of a packet.
		app_log_error("Radio RX Error occurred\nEvents: %llX\n", error_code);
		state = S_IDLE;
		break;
	case S_TX_PACKET_ERROR:  //A generic state to handle cases of an error to the transmission of a packet.
		app_log_error("Radio TX Error occurred\nEvents: %llX\n", error_code);
		state = S_IDLE;
		break;
	case S_CALIBRATION_ERROR: //A generic state to handle cases of an error during the calibration of this board.
		app_log_error("Radio Calibration Error occurred\nEvents: %llX\nRAIL_Calibrate() result:%d\n", error_code, calibration_status);
		state = S_IDLE;
		break;
	case S_IDLE: //A generic state where the board performs no action (necessary while, e.g., waits for a transmission to be completed).
		break;
	default:     //Unexpected state, should never reach here
		app_log_error("Unexpected state occurred: %d\n", state);
		state = S_IDLE;
		break;
	}
}

/*******************************************************************************
 * Handles a received RX message by performing the necessary actions.
 ******************************************************************************/
 void handle_rx_packet_payload(const uint8_t * const rx_buffer){
	switch(rx_buffer[MSGIDX_TYPE]){
	case MSG_RESTART:{ //A message indicating that the system is restarting at the moment.
		if(rx_buffer[MSGIDX_RESTART_ID]>restart_id){
			wake_up();
			restart_command = true;
			restart_id = rx_buffer[MSGIDX_RESTART_ID];
		}
		break;}
	case MSG_START_TASK:{ //A message indicating that the system is starting a new task at the moment.
		wake_up();
		if(rx_buffer[MSGIDX_TASK]!=current_task && rx_buffer[MSGIDX_TASK]==T_CONSENSUS)
			average_command = true;
		break;}
	case MSG_CONSENSUS_STATE:{ //A message with another board's current state.
		if(app_is_ok_to_sleep()) //if the board is sleeping, do nothing
			break;
		uint8_t buffer[4] = { rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6] };
		float num = *((float*) buffer);
		consensus_states[rx_buffer[MSGIDX_SRC_BOARD]] = num;
		break;}
	case MSG_BATON:{  //A message with the baton.
		if(app_is_ok_to_sleep()) //if the board is sleeping, do nothing
			break;
		dst_of_baton = -1;
		if(baton_path[LENGTH_OF_BATON_PATH-1]==rx_buffer[MSGIDX_SRC_BOARD] && baton_path[0]==BOARD_ID)
			dst_of_baton = baton_path[1];
		else if(baton_path[LENGTH_OF_BATON_PATH-2]==rx_buffer[MSGIDX_SRC_BOARD] && baton_path[LENGTH_OF_BATON_PATH-1]==BOARD_ID)
			dst_of_baton = baton_path[0];
		else {
			for(int i=1;i<LENGTH_OF_BATON_PATH-1;i++){
				if(baton_path[i]==BOARD_ID && baton_path[i-1]==rx_buffer[MSGIDX_SRC_BOARD]){
					dst_of_baton = baton_path[i+1];
					break;
				}
			}
		}
		if(dst_of_baton<0) //Shouldn't have received the baton from this board
			break;

		if(starting_board==BOARD_ID)
			RAIL_CancelMultiTimer(&tmr0);
		baton=true;
		baton_cntr++;

		boards_completed_their_task = rx_buffer[MSGIDX_BOARDS_OVER];
		if(boards_completed_their_task>=NUM_OF_BOARDS && starting_board==BOARD_ID && (baton_cntr-1)%batons_per_cycle==0) //marks that the next is the last cycle of the baton, and the boards can sleep when they are not going to receive the baton again
			boards_completed_their_task = SEND_SYSTEM_TO_SLEEP; //the boards can now sleep
		app_log_info("                              Received BATON %d! %d boards have reached to a result under the threshold.\n", baton_cntr, boards_completed_their_task<0?NUM_OF_BOARDS:boards_completed_their_task);
		break;}
	}
}


/*******************************************************************************
 * Transmits a packet to another board.
 ******************************************************************************/
bool packet_transmission (RAIL_Handle_t rail_handle, volatile tx_operation_t oper){
	bool ret = false; //the value to return. It will be true only if the message was actually transmitted.

	switch (oper) {
	case O_GLB_RESTART:{ //Send a message of type MSG_RESTART.
		int send_addr = NUM_OF_BOARDS - num_of_pending_msgs_for_tx;
		tx_packet[MSGIDX_TYPE]=MSG_RESTART;
		tx_packet[MSGIDX_SRC_BOARD]=BOARD_ID;
		tx_packet[MSGIDX_DST_BOARD] = send_addr;
		tx_packet[MSGIDX_RESTART_ID] = restart_id;
		if(send_addr!=BOARD_ID && graph[BOARD_ID][send_addr]){
			send_packet(rail_handle, tx_packet[MSGIDX_DST_BOARD]);
			ret = true;
		}
		break;}
	case O_GLB_START_TASK:{ //Send a message of type MSG_START_TASK.
		int send_addr = NUM_OF_BOARDS - num_of_pending_msgs_for_tx;
		tx_packet[MSGIDX_TYPE]=MSG_START_TASK;
		tx_packet[MSGIDX_SRC_BOARD]=BOARD_ID;
		tx_packet[MSGIDX_DST_BOARD] = send_addr;
		tx_packet[MSGIDX_TASK] = current_task;
		if(send_addr!=BOARD_ID && graph[BOARD_ID][send_addr]){
			send_packet(rail_handle, tx_packet[MSGIDX_DST_BOARD]);
			ret = true;
		}
		break;}
	case O_GLB_SEND_STATE:{ ////Send a message of type MSG_CONSENSUS_STATE.
		int send_addr = NUM_OF_BOARDS - num_of_pending_msgs_for_tx;
		tx_packet[MSGIDX_TYPE] = MSG_CONSENSUS_STATE;
		tx_packet[MSGIDX_SRC_BOARD] = BOARD_ID;
		tx_packet[MSGIDX_DST_BOARD] = send_addr;

		uint8_t *conv = (uint8_t*) &consensus_states[BOARD_ID]; //convert the 4-byte float to a uint8_t array of length 4.
		tx_packet[3] = conv[0];
		tx_packet[4] = conv[1];
		tx_packet[5] = conv[2];
		tx_packet[6] = conv[3];
		if(send_addr!=BOARD_ID && graph[BOARD_ID][send_addr]){
			send_packet(rail_handle, tx_packet[MSGIDX_DST_BOARD]);
			ret = true;
		}
		break;}
	case O_GIVE_BATON:{  //Release the baton.
		tx_packet[MSGIDX_TYPE]=MSG_BATON;
		tx_packet[MSGIDX_SRC_BOARD]=BOARD_ID;
		tx_packet[MSGIDX_DST_BOARD]= dst_of_baton;
		tx_packet[MSGIDX_BOARDS_OVER] = boards_completed_their_task;
		send_packet(rail_handle, tx_packet[MSGIDX_DST_BOARD]);
		ret = true;
		break;}
	default: //Should never reach here
		app_log_error("Error. Invalid TX operation %d.\n", tx_operation_to_achieve);
		break;
	}
	return ret;
}

/*******************************************************************************
 * Initializes the current board.
 ******************************************************************************/
void initialize_app(RAIL_Handle_t rail_handle){
	start_receiving(rail_handle); //start receiving in this board's channel.
	initialize_tools(); //initialize the tools provided by the app_tools.h module.
	clear(); //Delete any existing states in the stack.
	RAIL_CancelMultiTimer(&tmr0); //Stop the tmr0 timer (which counts for a timeout).

	//Variables related to the board's status
	calibration_status = 0;
	error_code = 0;

    // Flags to update state machine from interrupts
    packet_received = false;
    packet_sent = false;
    rx_error = false;
    tx_error = false;
    cal_error = false;

    //Baton - related variables
	baton = false;
	batons_per_cycle = 0;
	baton_cntr = 0;

	for(int i=0; i<LENGTH_OF_BATON_PATH; i++)
		if(baton_path[i]==BOARD_ID)
			batons_per_cycle++;
	current_task = T_NONE;

	//Other variables
	boards_completed_their_task = 0;
	consensus_is_over = false;
	starting_board = -1;
}
