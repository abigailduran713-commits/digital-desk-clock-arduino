# Arduino Digital Desk Clock

## Project Overview
This project is a standalone digital desk clock built using an Arduino Uno, a DS3231 real-time clock module, and a 16x2 LCD.

The clock features a three-button menu system, user-adjustable time settings, interval-based reminders, and a visual LED alert. A DS3231 RTC provides accurate timekeeping, while non-blocking timing allows the system to remain responsive while handling user input, display updates, and reminders.

After testing the circuit on a breadboard, the complete system was converted into a custom PCB designed in KiCad. The PCB has been designed and is currently awaiting fabrication and assembly.

## Features
- 16x2 LCD display
- Time and date display
- Accurate timekeeping using a DS3231 RTC module
- User-adjustable time through the LCD menu
- Menu navigation using three push buttons: UP, DOWN, and MODE
- Interval-based reminders
- Enable and disable reminder settings
- Blinking LED alert during active reminders
- Custom PCB designed in KiCad

## Hardware Used for Prototype
- Arduino Uno R3
- 16x2 LCD display
- DS3231 Real-time clock module
- Push buttons (x3)
- LED + resistor
- Potentiometer (LCD contrast)
- Breadboard and jumper wires

## Hardware Used for PCB
- Custom PCB designed in KiCad
- Arduino Uno R3 and headers
- RTC module and headers
- LCD module and headers
- Push-button inputs
- LED + resistors
- Potentiometer

## How It Works
The DS3231 real-time clock module maintains the current time independently of the Arduino. The Arduino reads the time from the RTC through I2C communication and displays it on the 16x2 LCD.

The user interacts with the clock through three push buttons. A menu-driven state machine allows the buttons to perform different actions depending on the active screen, including navigating settings, changing the displayed time, selecting reminder intervals, and enabling or disabling reminders.

Reminder timing is handled without long blocking delays. The program uses `millis()` to manage LCD updates, button input, reminder checks, and the blinking LED alert while keeping the system responsive.

The circuit was initially developed and tested on a breadboard. After confirming the main hardware and software functions, the schematic and PCB layout were created in KiCad for a more permanent version of the project.

## PCB Design
After completing the breadboard prototype, the circuit was recreated as a schematic in KiCad and converted into a custom PCB layout.

The PCB design includes connections for:
- The Arduino
- RTC module
- LCD
- 3 Buttons
- LED
- Potentiometer
- Resistors

The design process included:
- Assignming schematic symbols and footprints
- Running electrical rules checks
- Definin the board outline
- Placing components
- Routing signal and power traces
- Adding a ground plane
- Running design rules checks
- Generating Gerber and drill files for fabrication

## Future Improvements
- Fabricate and assemble the custom PCB
- Test and validate the assembled board
- Design a compact enclosure
- Revise the PCB if issues are found during physical testing
