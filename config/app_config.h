/***************************************************************************//**
 * @file app_config.h
 * @brief This file contains configuration parameters for the application.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdio.h>
#include <rail_types.h>

///The identity (also defining the RX channel) of the current board.
#define BOARD_ID 5


//---------------------------Grid topology constants----------------------------
///The (exact) total number of boards.
#define NUM_OF_BOARDS 6

///This graph specifies the commuting boards. Set graph[i][j] true if board i can send/receive messages from board j, or false otherwise. Also, graph[i][i] has to be set true for any board.
extern bool graph[NUM_OF_BOARDS][NUM_OF_BOARDS];

///The exact size of the {@link baton_path} array.
#define LENGTH_OF_BATON_PATH 9

/** A path inside the graph, which will be followed by the baton.
 * - RULE 1: The path should include all nodes at least once.
 * - RULE 2: Every node should have an edge (according to the {@link graph}) with its previous and next.
 * - RULE 3: The last node of the cycle should have an edge with the first one.
 * - RULE 4: Every node should be easy to understand where to send the baton, depending on
 * the node from which it was received. For example, subsequences like ...1,2,3... and ...1,2,4...
 * should not both exist.
 */
extern int8_t baton_path[LENGTH_OF_BATON_PATH];

//---------Configuration of parameters related to the application---------------
///The minimum temperature that can be measured by the thermal sensor.
#define MIN_TEMPERATURE -273

///When |state-previous_state|<=STOP_THRESHOLD for every board, then the execution of the Average Consensus algorithm can be terminated.
extern const float STOP_THRESHOLD;

///Set to 1 for the board's LEDs to indicate the EM transitions (awake or asleep). Set to 0 for deactivated LEDs.
#define USE_EM_TRANSITION_LEDS 1

///Set to 1 if the board has to use pre-specified temperature measurements (from the {@link simulated_temperatures} array), e.g., for debugging or performance measurement. Set to 0 for the board to use the actual temperature measured by its thermistor.
#define SIMULATE_TEMPERATURE_MEASUREMENTS 1

///This array contains some pre-specified temperatures, to be used instead of the actual ones, when {@link SIMULATE_TEMPERATURE_MEASUREMENTS} equals to 1.
extern const float simulated_temperatures[NUM_OF_BOARDS];

#endif  //APP_CONFIG_H
