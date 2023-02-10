/***************************************************************************//**
 * @file main.c
 * @brief Contains the {@link main()} function of this system.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "sl_system_init.h"
#include "sl_power_manager.h"
#include "sl_system_process_action.h"
#include "app/app_init.h"
#include "app/app_process.h"

///A static handle of a RAIL instance
static RAIL_Handle_t rail_handle;

/**
 * The first function to be called when the system is being
 * executed.
 *
 * @date 01/02/2023
 * @return It never returns, as it enters an infinite loop which runs while the
 * board is active.
 */
int main(void) {
	// Initialize Silicon Labs device, system, service(s) and protocol stack(s).
	sl_system_init();

	// Initialize the application.
	rail_handle = app_init();

	while (1) {
		// Do not remove this call: Silicon Labs components process action
		// routine, which must be called here.
		sl_system_process_action();

		// Application process.
		app_process_action(rail_handle);

		// Let the CPU go to sleep if the system allows it.
		sl_power_manager_sleep();
	}
}
