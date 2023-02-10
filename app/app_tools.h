/***************************************************************************//**
 * @file app_tools.h
 * @brief Headers of utility functions for the system.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_TOOLS_H
#define APP_TOOLS_H

#include <rail_types.h>

///The maximum delay for a board to release the baton, after it has received it (in milliseconds).
#define MAX_DELAY_PER_BATON_STEP_MILISECS 1000

///If the baton has not completed at least one cycle in the specified time (in milliseconds), the system will restart.
#define RESTART_TIMEOUT_MILISECS ((LENGTH_OF_BATON_PATH-1)*MAX_DELAY_PER_BATON_STEP_MILISECS)

//These constants will be used as indexes for the TX/RX messages.
//When 2 constants are equal, they cannot be both included in the same message (obviously).
///The index in the message payload where the type of the message is specified.
#define MSGIDX_TYPE 0
///The index in the message payload where the source board is specified.
#define MSGIDX_SRC_BOARD 1
///The index in the message payload where the destination board is specified.
#define MSGIDX_DST_BOARD 2
///The index in the message payload where the current task is specified.
#define MSGIDX_TASK 3
///The index in the message payload where the number of boards which agree to terminate the algorithm is specified.
#define MSGIDX_BOARDS_OVER 3
///The index in the message payload where the restart id (a number related with the restarting of the system) is specified.
#define MSGIDX_RESTART_ID 3

///Responsible to count the time between 2 batons passed from the board which started the averaging task. If it alarms, a restart of the system is initiated.
RAIL_MultiTimer_t tmr0;

///When it is true, the average consensus algorithm has to be executed, starting from the current board.
volatile bool average_command;

/// When it is true, the system has to restart, starting from the current board.
volatile bool restart_command;

///A parameter used to determine when to restart. Re-initialization of the board takes place only if an id greater than the current value of this parameter is received from another board.
int restart_id;

///Stores the last measured temperature by the current board, retrieved by the {@link measure_temperature()} function.
float temperature;

/** The callback function for {@link tmr0} timer. It re-initializes this board,
 * as part of a general restart which is starting for the whole distributed system.
 *
 * @date 20/01/2023
 * @param tmr The timer which expired (i.e., {@link tmr0}).
 * @param expectedTimeOfEvent Is not used.
 * @param cbArg Is not used.
 */
void enable_alarm(RAIL_MultiTimer_t *tmr, RAIL_Time_t expectedTimeOfEvent, void *cbArg);

/** This function is automatically called by the PowerManager API, to determine
 * whether the board will go into sleep mode or not. Its return value is
 * determined from whether {@link wake_up()} or {@link sleep()} was called last.
 * When the board is initially powered on (or the reset button is pressed), it
 * returns false (until {@link sleep()} is called for first time). Notice that
 * its result is not affected by restarts of the distributed system.
 *
 * @date 20/01/2023
 * @return True if the board should sleep, false if the board should not.
 */
bool app_is_ok_to_sleep(void);

//  Check if the MCU can sleep at that time.

/** Forces the board to sleep. From the call of this method (and until
 * {@link wake_up()} is called), {@link app_is_ok_to_sleep()} will return true.
 *
 * @date 20/01/2023
 */
void sleep();

/** Forces the board to wake up. From the call of this method (and until
 * {@link sleep()} is called), {@link app_is_ok_to_sleep()} will return false.
 *
 * @date 20/01/2023
 */
void wake_up();

/** Stores the current temperature, as measured by this board's thermistor (or
 * a simulated value, depending on the {@code SIMULATE_TEMPERATURE_MEASUREMENTS}
 * constant) in the {@link temperature} variable.
 *
 * @date 20/01/2023
 */
void measure_temperature();

/** Initializes all tools provided by this module, except from the
 * {@link temperature} variable, which has to be initialized by the
 * {@link measure_temperature()} function.
 *
 * @date 20/01/2023
 */
void initialize_tools();

#endif  // APP_TOOLS_H
