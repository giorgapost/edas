/***************************************************************************//**
 * @file app_tools.c
 * @brief Implementation of utility functions for the system.
 * @author Georgios Apostolakis
 ******************************************************************************/
#include "app_tools.h"
#include "app_log.h"
#include "sl_si70xx.h"
#include "sl_i2cspm_instances.h"
#include "app_config.h"
#include "app_process.h"

//=========================================================================
//-------------------- SLEEP MECHANISM ------------------------------------
//=========================================================================

/** This variable is the core of the sleep mechanism. It cannot be directly changed by the user, but only through the {@link wake_up()}, {@link sleep()} functions.*/
static bool ready_to_sleep = false;

/*******************************************************************************
 * Returns whether the board should sleep or not.
 ******************************************************************************/
bool app_is_ok_to_sleep(void) {
  return ready_to_sleep;
}

/*******************************************************************************
 * Forces the current board to sleep.
 ******************************************************************************/
void sleep(){
	ready_to_sleep = true;
}

/*******************************************************************************
 * Forces the current board to wake up.
 ******************************************************************************/
void wake_up(){
	if(ready_to_sleep)
		app_log_info("Woke up!\n");
	ready_to_sleep = false;
}
//*************************************************************************
//*************************************************************************

//=========================================================================
//---------------------- GENERIC TOOLS ------------------------------------
//=========================================================================

/*******************************************************************************
 * Stores the current temperature at the {@link temperature} variable.
 ******************************************************************************/
void measure_temperature(){
	int32_t temp_data;
	uint32_t rh_data;
	sl_si70xx_measure_rh_and_temp(sl_i2cspm_sensor, SI7021_ADDR, &rh_data, &temp_data);
	float temp = (float) temp_data/1000.0;
	if(SIMULATE_TEMPERATURE_MEASUREMENTS)
		temperature = simulated_temperatures[BOARD_ID];
	else
		temperature = temp;
	app_log_info("Actual temperature now is %.2f degrees of Celsius. However, a (simulated) value of %.2f degrees will be used instead.\n",temp, temperature);
}

/*******************************************************************************
 * The timers' callback function.
 ******************************************************************************/
void enable_alarm(RAIL_MultiTimer_t *tmr, RAIL_Time_t expectedTimeOfEvent, void *cbArg){
	(void)cbArg; (void)expectedTimeOfEvent;
	if(tmr==(&tmr0)){
		baton = true;
		for(int i=0;i<LENGTH_OF_BATON_PATH;i++){
			if(baton_path[i]==BOARD_ID){
				dst_of_baton = i+1;
				break;
			}
		}
		if(dst_of_baton==LENGTH_OF_BATON_PATH)
			dst_of_baton = 0;
		dst_of_baton = baton_path[dst_of_baton];

		restart_command = true;
		restart_id++;
	}
}


/*******************************************************************************
 * Initializer for all tools provided by this module.
 ******************************************************************************/
void initialize_tools(){
	temperature = MIN_TEMPERATURE-1;

	average_command = false;
	restart_command = false;
	restart_id = 0;
}
