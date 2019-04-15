#  180348 / 150500 - Soldering station
Software for the soldering station ( 180348 / 150500 ) based on the arduino framwork.

Shows the Temperature in °C and is limited to use 1.5Amp of current.

## Getting Started

Download the source and open it with the arduino ide ( >= 1.8.x ) and select Arduino Leonardo as target.
The station shall be detected via usb as serial port. Set the ide to use the port and compile the sketch.
If the leonardo bootloader is working you can just press upload and the software will be transfered to the station.
Be aware that after flashing the new software, your current temperature settings may be gone and set back to 50°C


### Prerequisites

You need TimerOne library installed in your libs path.


