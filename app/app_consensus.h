/***************************************************************************//**
 * @file app_consensus.h
 * @brief Header file with functions which implement the Average Consensus algorithm.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_CONSENSUS_H
#define APP_CONSENSUS_H

#include "rail_types.h"
#include "app_config.h"

///The counter of the iterations. It is automatically updated by the {@link update_consensus_state()} function.
uint8_t consensus_iters;

///The knowledge of this board for the states of the other boards (used to update its state).
float consensus_states[NUM_OF_BOARDS];

/** This function initializes the consensus setup. It has to be called after a
 * temperature has been measured (with the
 * {@link app_tools#measure_temperature() measure_temperature()} function).
 *
 * @date 01/02/2023
 */
void initialize_consensus_setup(); //Has to be called after a temperature has been measured

/** It updates the state of this board based on the previous states of the
 * system. It has to be called only when the states from the commuting
 * (according to the graph) boards have been received.
 *
 * @date 01/02/2023
 */
void update_consensus_state();

#endif  // APP_CONSENSUS_H
