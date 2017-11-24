## Firmware

This folder contains the firmware source code related to the project :

version | name             | description
--------|------------------|------------------------------------------------------------------------
v1.0    | central_cube.ino | cleaned-up and commented version, already containing minor improvements
v1.0    | feynman_cube.ino | cleaned-up and commented version, as used during the presentation
v1.0    | pinball_cube.ino | cleaned-up and commented version, as used during the presentation

The above Arduino sketches are to be loaded in the ATmega328p chip of the respective µ-controller board.

### External libraries
Some of these sketches require external libraries. Please check below table for cross-references. This source folder also contains snapshots from these libraries at the time of built (tested version).

library        | to be used in    | origin                                               | notes
---------------|------------------|------------------------------------------------------|------------------------------------
lib-tm1638.zip | central_cube.ino | [link](https://github.com/rjbatista/tm1638-library/) | to be used for the 16-digit display

