Rc Car

References:
-------------
- Nrf24: https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
- Voltage indicator: https://www.instructables.com/id/Arduino-Battery-Voltage-Indicator/
- ESC: Youtube videos.

Components:
-----------
- Arduino Mega 256
- 6x 18650 batteries
- Nikko Ford F-150 Rc car
- Genuine Motor (Maybe Mabuchi RS-540 brushed DC motor)
- ESC F05428
- MG-995 Servo motor (Genuine was complicated to use)
- NRF24L01+PA+LNA 
- 2 capacitor 2200µF 16V
- LM1117-3.3V
- 2 diodes
- 2x2 resistors (10K). They must have the exact same resistance to measure the voltage

Notes:
-------------
- Nrf24 is very sensitive to the power voltage of 3.3V. A voltage regulator is mandatory.
- I wasted a lot of time having the radio not available because of the ESC. I fixed it with a diode...

Photos + Schemas
-----------------

![Breadboard](schemas/CarDuino2020_bb.png?raw=true "Breadboard")
![Photo2](schemas/Photo1.jpg?raw=true "Photo1")
![Photo2](schemas/Photo2.jpg?raw=true "Photo2")