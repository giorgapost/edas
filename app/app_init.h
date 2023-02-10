/***************************************************************************//**
 * @file app_init.h
 * @brief Header file with functions for the initialization of the application.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_INIT_H
#define APP_INIT_H

#include "rail_types.h"

/**************************************************************************//**
 * The function is used for some basic initialization related to the app.
 *
 * @date 01/02/2023
 * @param None
 * @returns A handle to the RAIL instance that will be used by the application.
 *
 * It ensures the followings:
 * - Start RAIL reception.
 * - Set the LEDs, depending on the EM state and the user's configuration.
 * - Initialize the application, the timers and the EM transition mechanism.
 * - Print start message to the console.
 *****************************************************************************/
RAIL_Handle_t app_init();

#endif  // APP_INIT_H
