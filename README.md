# CPE301FINALPROJECT
CPE 301 Final Project | Jerard Cabeguin

## OVERVIEW:
### Project Description:
This final project aims to create an energy-efficient evaporative cooling system suitable for dry, hot climates, using the ArduinoMEGA2560 board. This cooler draws air through a water-soaked pad, which is cooled by evaporation and then circulated. 
### Objective:
Design and implement a system that can autonomously monitor environmental conditions and adjust cooling mechanisms accordingly. 

## SYSTEM COMPONENTS AND FUNCTIONALITIES
**ArduinoMEGA2560**
The central control unit for managing sensor data inputs and outputs to fulfill the code. 

### _Sensors:_
- **DHT11 Temperature and HUmiditiy Sensor:** Monitors ambient temperature and humidity, providing data to adjust the different modes (disabled, idle, error, and running), the fan, and vent mechanisms.
- **Water Level Sensor:** Detects the reservoir's water level, ensuring enough water for effective cooling.

### _Motors:_
- **Stepper Motors:** Adjusts the angle of the output vent to direct the cooled air.
- **DC Motor with Fan Blade:** Circulates air through the water-soaked pad and out into the room.
**LCD Display:** Displays the real-time temperature, humidity, and system status messages.
**Real-Time Clock Module:** Logs the time and date of significant system events, such as changes in motor states or system errors.
### LED Indicators:
- **Yellow LED:** Indicates the system is in disabled mode.
- **Green LED:** Indicates the system is in idle mode.
- **Red LED:** Indicates an error; low water level.
- **Blue LED:** Indicates the system is running/cooling.

## Circuit Diagram and Schematic
![FinalProjectSchematic](https://github.com/jcabeguin/CPE301FINALPROJECT/assets/112597766/14c9dccb-68c4-470d-8477-6bc0421e083e)

## Functional State Description
- **Disabled:** The system is off; only the start button is monitored to start the system into the idle state. Yellow LED is on. Vents are adjustable.
-  **Idle:** Monitors the temperature and water levels, ready to activate cooling as needed. Green LED is on. Vents are adjustable. 
-  **Running:** Active cooling in progress; the fan is turned on, and the vent is adjustable. Blue LED is on. Vents are adjustable. Fan is on.
-  **Error:** Indicator that the water level is low. Requires user intervention to resume normal operation and press the reset button to the idle state or the stop button to the disabled state. 

## PROJECT IMPLEMENTATION
- ### Libraries:
  Some of the libraries that were included would be:
  - **LiquidCrystal.h:** Library for the LCD monitor
  - **DHT11.h:** Library for the temperature and humidity sensor
  - **Wire.H & RTClib.h:** Libraries for the Clock module library
  - **Stepper.h:** Libraries for the Stepper motor
    






















