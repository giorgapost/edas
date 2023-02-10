/***************************************************************************//**
 * @file app_cli.c
 * @brief Contains functions which determine the effect of each CLI command.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "em_chip.h"
#include "app_log.h"
#include "sl_cli.h"
#include "app_config.h"
#include "app_process.h"
#include "app_tools.h"

/** CLI - info: Prints the unique ID of the board to the console.
 *
 * @date 10/12/2022
 * @param arguments A pointer to the arguments provided by the user through the
 * console (no arguments should be provided for this command).
 */
void cli_info(sl_cli_command_arg_t *arguments) {
	(void) arguments;
	app_log_info("  MCU Id:       0x%llx\n", SYSTEM_GetUnique());
}

/** CLI - average: Wakes up the system and starts the execution of the Average
 * Consensus algorithm in a distributed manner.
 *
 * @date 01/02/2023
 * @param arguments A pointer to the arguments provided by the user through the
 * console (no arguments should be provided for this command).
 */
void cli_avg_consensus(sl_cli_command_arg_t *arguments) {
	(void) arguments;
	if(!app_is_ok_to_sleep()){
		app_log_info("Boards are busy. Try again in a while.\n");
		return;
	}
	wake_up();
	average_command = true;

	starting_board = BOARD_ID;
	baton = true;    //Acquire the baton
	baton_cntr = 1;
	for(int i=0;i<LENGTH_OF_BATON_PATH;i++){ //Compute the destination of the baton, after being released from this board
		if(baton_path[i]==BOARD_ID){
			dst_of_baton = i+1;
			break;
		}
	}
	if(dst_of_baton==LENGTH_OF_BATON_PATH)
		dst_of_baton = 0;
	dst_of_baton = baton_path[dst_of_baton];

	app_log_info("CLI command was given to execute Distributed Average Consensus.\n");
}
