/***************************************************************************//**
 * @file app_init.c
 * @brief Implementation file with function for the initialization of the system.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "rail.h"
#include "sl_rail_util_init.h"
#include "sl_rail_util_init_inst0_config.h"
#include "sl_rail_util_protocol_types.h"
#include "rail_config.h"
#include "sl_simple_led_instances.h"
#include "app_log.h"
#include "sl_power_manager.h"
#include "app_config.h"
#include "app_network.h"
#include "app_process.h"


/** Checks phy settings to avoid errors at packet sending.
*
* @date 15/01/2023
*/
static void validation_check() {
#if defined(RAIL0_CHANNEL_GROUP_1_PROFILE_WISUN_OFDM) && !defined(HARDWARE_BOARD_HAS_EFF)
  _Static_assert(SL_RAIL_UTIL_PA_SELECTION_SUBGHZ == RAIL_TX_POWER_MODE_OFDM_PA,
                 "Please use the OFDM PA settings in the sl_rail_util_pa_config.h for OFDM phys.");
#endif
#if defined(RAIL0_CHANNEL_GROUP_1_PROFILE_WISUN_OFDM) && RAIL_SUPPORTS_EFF && defined(HARDWARE_BOARD_HAS_EFF)
  _Static_assert(SL_RAIL_UTIL_PA_SELECTION_SUBGHZ >= RAIL_TX_POWER_MODE_OFDM_PA_EFF_30DBM,
                 "Please use the OFDM PA for EFF settings in the sl_rail_util_pa_config.h for OFDM phys.");
#endif
}

/// A mask with the EM transition events.
#define EM_EVENT_MASK_ALL  (  SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM0 \
							| SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM0  \
							| SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM1 \
							| SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM1  \
							| SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2 \
							| SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM2  \
							| SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM3 \
							| SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM3)

/** The callback function after an EM transition.
 *
 * @date 01/02/2023
 * @param from The previous EM state of the current board.
 * @param to The new EM state of the current board.
 */
static void em_callback(sl_power_manager_em_t from, sl_power_manager_em_t to){
	(void)from;
	if(to==2){ //EM2
		sl_led_turn_off(&sl_led_led0);
		sl_led_turn_off(&sl_led_led1);
	}
	else if(to==1){ //EM1
		sl_led_turn_off(&sl_led_led0);
		sl_led_turn_on(&sl_led_led1);
	}
	else if(to==0){ //EM0
		sl_led_turn_on(&sl_led_led0);
		sl_led_turn_off(&sl_led_led1);
	}
}

/// The handle of the EM transition events.
static sl_power_manager_em_transition_event_handle_t event_handle;

/// Stores info about the EM transition events.
static sl_power_manager_em_transition_event_info_t event_info = {
  .event_mask = EM_EVENT_MASK_ALL,
  .on_event = em_callback,
};

/******************************************************************************
 * The function is used for some basic initialization related to the app.
 *****************************************************************************/
RAIL_Handle_t app_init() {
	validation_check();

	RAIL_Handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0); // Get RAIL handle, used later by the application
	set_up_tx_fifo(rail_handle); //Prepare a FIFO structure utilized by the tx mechanism

	if(USE_EM_TRANSITION_LEDS==1){
		sl_led_turn_on(&sl_led_led0); //Turn on LED 0, until it's time to sleep
		sl_power_manager_subscribe_em_transition_event(&event_handle, &event_info);
	}

	RAIL_InitPowerManager(); //Initialize the power manager API, which will let the board sleep
	sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

	RAIL_ConfigMultiTimer(true); //Initialize the multitimer API, which will provide as many timers as needed.

	initialize_app(rail_handle); //Initialize the application's variables.

	app_log_info("Embedded Distributed Averaging System (EDAS) - Temperature\n");  // CLI info message
	return rail_handle;
}
