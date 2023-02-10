/***************************************************************************//**
 * @file sl_cli_command_table.c
 * @brief Declarations of command structs for CLI framework.
 * @author Georgios Apostolakis
 ******************************************************************************/
#include <stdlib.h>
#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

/** CLI - info: Prints the unique ID of the board to the console.
 *
 * @date 10/12/2022
 * @param arguments A pointer to the arguments provided by the user through the
 * console (no arguments should be provided for this command).
 */
void cli_info(sl_cli_command_arg_t *arguments);

/** CLI - average: Wakes up the system and starts the execution of the Average
 * Consensus algorithm in a distributed manner.
 *
 * @date 01/02/2023
 * @param arguments A pointer to the arguments provided by the user through the
 * console (no arguments should be provided for this command).
 */
void cli_avg_consensus(sl_cli_command_arg_t *arguments);


///This struct determines the exact syntax of the 'info' CLI command.
static const sl_cli_command_info_t cli_cmd__info = \
  SL_CLI_COMMAND(cli_info,
                 "Prints the unique ID of this Thunderboard, set from Silicon Labs company.",
                  "",
                 {SL_CLI_ARG_END, });

///This struct determines the exact syntax of the 'average' CLI command.
static const sl_cli_command_info_t cli_cmd__average = \
  SL_CLI_COMMAND(cli_avg_consensus,
                 "Starts the execution of Average Consensus and returns the average temperature of the system.",
                  "",
                 {SL_CLI_ARG_END, });

///This table determines the commands to be used in the CLI.
const sl_cli_command_entry_t sl_cli_default_command_table[] = {
  { "info", &cli_cmd__info, false },
  { "average", &cli_cmd__average, false },
  { NULL, NULL, false }
};

#ifdef __cplusplus
}
#endif
