/***************************************************************************//**
 * @file app_config.c
 * @brief This file initializes configuration variables for the application.
 * @author Georgios Apostolakis
 ******************************************************************************/
#include "app_config.h"

bool graph[NUM_OF_BOARDS][NUM_OF_BOARDS] = {
    {true,  true,  false, false, false, true},
    {true,  true,  true,  false, false, true},
    {false, true,  true,  true,  false, false},
    {false, false, true,  true,  false, false},
    {false, false, false, false, true,  true},
    {true,  true,  false, false, true,  true}};

/*******************************************************************************
 * RULE 1: The path should include all nodes at least once.
 * RULE 2: Every node should have an edge (according to the graph) with its previous and next.
 * RULE 3: The last node should have an edge with the first one.
 * RULE 4: Every node should be easy to understand where to send the baton, depending on
 * the node from which it was received. For example, sequences like ...1,2,3... and ...1,2,4...
 * should not exist in the same path.
 ******************************************************************************/
int8_t baton_path[LENGTH_OF_BATON_PATH] = {3, 2, 1, 0, 5, 4, 5, 1, 2};

/*******************************************************************************
 * When |state-previous_state|<=STOP_THRESHOLD for every board, then the
 * execution of the Average Consensus algorithm can be terminated.
 ******************************************************************************/
const float STOP_THRESHOLD = 0.1;

///This array contains some pre-specified temperatures, to be used instead of the actual ones, when {@link SIMULATE_TEMPERATURE_MEASUREMENTS} equals to 1.
/*******************************************************************************
 * This array contains some pre-specified temperatures, to be used instead of
 * the actual ones, when {@link SIMULATE_TEMPERATURE_MEASUREMENTS} equals to 1.
 ******************************************************************************/
const float simulated_temperatures[NUM_OF_BOARDS] = {10, 20, 30, 20, 15, 25};
