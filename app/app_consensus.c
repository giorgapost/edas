/***************************************************************************//**
 * @file app_consensus.c
 * @brief Implementation file of the Average Consensus algorithm.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "app_consensus.h"
#include "app_log.h"
#include "app_tools.h"

///These weights are used by the algorithm to update the current board's state.
static float weights[NUM_OF_BOARDS];

/** Initializes the weights array, required for the update of this board's state.
 *
 * @date 01/02/2023
 */
static void initialize_weights(){
    int deg[NUM_OF_BOARDS]; //the degree of each node of the graph
    int gr_deg = 0; //the degree of the graph

    for(int i=0;i<NUM_OF_BOARDS;i++){
        deg[i] = 0;
        for(int j=0;j<NUM_OF_BOARDS;j++){
            if(graph[i][j] && i!=j)
                deg[i]++;
        }
        if(deg[i]>gr_deg)
            gr_deg = deg[i];
    }

	for(int j=0;j<NUM_OF_BOARDS;j++){
		if(!graph[BOARD_ID][j])
			weights[j] = 0;
		else if(j==BOARD_ID)
			weights[j] = 1.0 - deg[BOARD_ID]/(1.0*(gr_deg+1));
		else
			weights[j] = 1.0/(gr_deg+1);
	}
}

/*******************************************************************************
 * It initializes the consensus setup.
 ******************************************************************************/
void initialize_consensus_setup(){
	if(temperature<MIN_TEMPERATURE){
		app_log_error("Error. Temperature not yet measured. Consensus setup cannot be initialized.\n");
		return;
	}

	initialize_weights();
	consensus_iters = 0;

	consensus_states[BOARD_ID] = temperature;
	//The states of the other boards do not need initialization.
	//They will be set when a message from those boards will be received.
}

/*******************************************************************************
 * It updates the state of this board.
 ******************************************************************************/
void update_consensus_state(){
	float next_state = 0;
	for(int i=0;i<NUM_OF_BOARDS;i++)
		next_state += weights[i]*consensus_states[i];
	consensus_states[BOARD_ID] = next_state;
	consensus_iters++;
}
