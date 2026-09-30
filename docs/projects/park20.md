# PARK20

A battery-powered IoT sensor that detects whether a vehicle is parked in a space, and reports it over NB-IoT.

- **Company:** a DATAKORUM S.L. device ([www.datakorum.es](https://www.datakorum.es)).
- **My role:** hardware and firmware.
- **Microcontroller:** STM32U575, an ultra-low-power Arm Cortex-M33.
- **Sensor:** Acconeer A111, a 60 GHz pulsed coherent radar.
- **Connectivity:** Quectel BC660K, NB-IoT.
- **Battery life:** more than 12 years.

## How it is put together.

The STM32U575 runs the device. The A111 is a pulsed coherent radar: it measures reflections and the distance they come from, which is what tells an occupied space from an empty one. The BC660K carries the result to the network.

Twelve years on one battery is the requirement everything else answers to. The microcontroller spends nearly all of its life in its lowest-power modes, and the radar and the modem are awake only while they work. NB-IoT suits that for the same reason: its power-saving mode lets the modem stay registered to the network while it sleeps.
