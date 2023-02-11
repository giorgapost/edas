# Embedded Distributed Averaging System (EDAS) - Temperature

This is a distributed embedded system, which returns the average of the temperatures measured by its components. The [Average Consensus](https://www.sciencedirect.com/science/article/abs/pii/S0743731506001808) algorithm is executed on [Thunderboard Sense 2](https://www.silabs.com/documents/public/user-guides/ug309-sltb004a-user-guide.pdf) devices.

## Table of Contents

- [Description](#description)
- [Requirements](#requirements)
- [Configuration](#configuration)
- [Compilation and deployment](#compilation-and-deployment)
- [Documentation](#documentation)
- [Usage](#usage)
- [Status](#status)
- [License](#license)
- [Authors](#authors)



## Description
EDAS is a distributed embedded system implemented by [Thunderboard Sense 2](https://www.silabs.com/documents/public/user-guides/ug309-sltb004a-user-guide.pdf) devices (from now on called *nodes*). 
Every node has a [thermistor](https://en.wikipedia.org/wiki/Thermistor) to measure the air temperature, as well as a TX/RX antenna to exchange messages wirelessly. 
[Average Consensus](https://www.sciencedirect.com/science/article/abs/pii/S0743731506001808) algorithm relies on iterative exchange of messages between the nodes, until all nodes converge to a fixed point (which is the average of all measured temperatures).

> **Warning**  
> The graph of the commuting nodes (i.e., the graph with an edge between every pair of nodes which can exchange messages, based on the system's topology) has to be connected, i.e., a path has to exist from any node to any other node in the graph.

The user communicates with the system via console commands given through a serial protocol. Thanks to its distributed nature, the user can connect, start the averaging process and get the result from any node of the network.

## Requirements
- [Thunderboard Sense 2](https://www.silabs.com/documents/public/user-guides/ug309-sltb004a-user-guide.pdf) devices which will be used as the nodes of the distributed network.
- Efficient power supply for all the nodes of the network. The average temperature can be returned only if all nodes are operational.
- [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio) 5 (tested) or newer (adaptations may be required).
- [Gecko SDK](https://www.silabs.com/developers/gecko-software-development-kit) 4.1.3 (tested) or newer (adaptations may be required). It can be downloaded directly from the [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio).
- [GNU ARM Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) 7.2.1 (tested) or newer (adaptations may be required). It can be downloaded directly from the [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio).


## Configuration

Configuration of the system has to take place before its deployment, because the user cannot make any changes at runtime. All configuration parameters are located in [config/app_config.h](config/app_config.h) and [config/app_config.c](config/app_config.c) files.

- [`BOARD_ID`](config/app_config.h#L13): The (unique) identity of every node. It gets values from $0$ to [`NUM_OF_BOARDS`](config/app_config.h#L18)$-1$ (also see the [Compilation and deployment](#compilation-and-deployment) section below).
- [`NUM_OF_BOARDS`](config/app_config.h#L18): The total number of nodes which comprise the system.
- [`graph`](config/app_config.c#L8): The graph of the commuting nodes. For every pair of nodes $i$ and $j$, `graph[i][j]` has to be set `true` if node $i$ can exchange messages with node $j$, or if $i=j$. Otherwise, it has to be set `false`.
- [`LENGTH_OF_BATON_PATH`](config/app_config.h#L24): Set this parameter equal to the length of the [`baton_path`](config/app_config.c#L24) array (see below).
- [`baton_path`](config/app_config.c#L24): It should contain a sequence of nodes which create a path and obey the following rules:
    - RULE $1$: The array should include every node at least once.
    - RULE $2$: Every node should have an edge (according to the graph) with its previous and next node (according to their ordering in the array).
    - RULE $3$: The last node of the array should have an edge with the first one.
    - RULE $4$: Every subarray of $2$ or more elements should be unique. For example, sequences like $...1,2,3...$ and $...1,2,4...$ should not exist in the same path.
- [`MIN_TEMPERATURE`](config/app_config.h#L38): The minimum temperature that can be possibly measured (in Celsius degrees).
- [`STOP_THRESHOLD`](config/app_config.c#L30): Determines when the execution of the Average Consensus algorithm will be terminated. More in detail, the execution will be terminated if $\left|\text{currentState}_i - \text{previousState}_i\right|\leq$[`STOP_THRESHOLD`](config/app_config.c#L30) for every node $i=0,...,$[`NUM_OF_BOARDS`](config/app_config.h#L18)$-1$. A smaller threshold results in a better estimation of the average but also more iterations of the algorithm before it terminates.
- [`USE_EM_TRANSITION_LEDS`](config/app_config.h#L44): Set to $0$ to deactivate the LEDs of the nodes. Set to $1$ to activate the LEDs of the nodes (red indicates an awake and fully-functional node in [EM0](https://www.silabs.com/mcu/32-bit-microcontrollers/efm32-energy-modes) mode, green indicates a node in [EM1](https://www.silabs.com/mcu/32-bit-microcontrollers/efm32-energy-modes) sleep mode). This parameter plays no role on the energy states & transitions of the nodes, but only on the activation/deactivation of the indicative LEDs.
- [`SIMULATE_TEMPERATURE_MEASUREMENTS`](config/app_config.h#L47): Set to $0$ to use the actual temperatures measured by the thermistors of the nodes. Set to $1$ to use some predetermined (by the [`simulated_temperatures`](config/app_config.c#L37) parameter, see below) values for the temperatures (mainly for testing purposes).
- [`simulated_temperatures`](config/app_config.c#L37): This array has a length equal to [`NUM_OF_BOARDS`](config/app_config.h#L18). The element at position $i$ is the (simulated) temperature used by the $i$-th node.

    > **Note**  
    > If [`SIMULATE_TEMPERATURE_MEASUREMENTS`](config/app_config.h#L47)$=0$, then the contents of [`simulated_temperatures`](config/app_config.c#L37) are useless.


## Compilation and deployment

To compile and deploy the project, follow the instructions below:
- Download and install the Simplicity Studio software (see [Requirements](#requirements) section).
- From the Simplicity Studio, install a suitable version of the Gecko SDK and the GNU ARM Toolchain (see [Requirements](#requirements) section).
- Clone the project and import it to the Simplicity Studio (File - Import).
- Configure the imported project appropriately (see [Configuration](#configuration) section).
- Set the parameter [`BOARD_ID`](config/app_config.h#L13) to $0,...,$[`NUM_OF_BOARDS`](config/app_config.h#L18)$-1$ successively. For each value:
    - Build the project (Project - Build Project).
    - Flash the arised binary file (edas.hex) to the respective Thunderboard device:
        - Connect the Thunderboard device to the computer via USB cable.
        - Right click on the .hex file (in Simplicity Studio's project explorer)
        - Flash to Device - select device - Program.


## Usage

## Documentation

To generate detailed documentation of the source code, the [Doxygen](https://www.doxygen.nl/) tool can be utilized. Execute it from the project's directory and use [Doxyfile.cfg](Doxyfile.cfg) for configuration.


## Status

Under maintenance.

## License

Distributed under the GPL-3.0 License. See [`LICENSE`](LICENSE) for more information.

## Authors

[Georgios Apostolakis](https://www.linkedin.com/in/giorgapost)
