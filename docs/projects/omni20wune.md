# OMNI20WUNE

A battery-powered IoT device that reads water meters and reports their readings over NB-IoT.

- **Company:** a DATAKORUM S.L. device ([www.datakorum.com](https://www.datakorum.com/)).
- **My role:** hardware and firmware.
- **Microcontroller:** STM32U575, an ultra-low-power Arm Cortex-M33.
- **Connectivity:** Quectel BC660K, NB-IoT.
- **Meter interface:** the protocol defined by the Spanish standard UNE 82326:2010.
- **Capacity:** up to 50 water meters per device.
- **Service life:** 12 years.

## How it is put together.

The meters hang off one of the STM32U575's UARTs, through custom interface hardware between that UART and the meters. The firmware speaks the UNE 82326:2010 protocol over it, collects the readings, and hands them to the BC660K, which carries them to the network. A 12-year service life on battery sets the same rule as in [PARK20](park20.md): the microcontroller sleeps in its lowest-power modes between readings, and the modem is awake only while it reports.
