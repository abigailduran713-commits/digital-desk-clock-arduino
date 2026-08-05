# Digital Desk Clock (Arduino)

## Project Demo
![Clock Schematic](images/clock_schematic2.jpg)
![Clock Running](images/clock_running.jpg)
![Reminder Active](images/clock_reminder.jpg)

## Project Overview
This project is a standalone digital desk clock built using an Arduino Uno, a DS3231 real-time clock module, and a 16x2 LCD.

The clock features a three-button menu system, user-adjustable time settings, interval-based reminders, and a visual LED alert. A DS3231 RTC provides accurate timekeeping, while non-blocking timing allows the system to remain responsive while handling user input, display updates, and reminders.

After testing the circuit on a breadboard, the complete system was converted into a custom PCB designed in KiCad. The PCB has been designed and is currently awaiting fabrication and assembly.

## Features
- 16x2 LCD time display
- Menu navigation using three push buttons (UP / DOWN / MODE)
- Interval-based reminders
- Blinking LED alert during reminders
- Non-blocking timing using `millis()`
- Designed to be extended with a real-time clock (RTC)

## Hardware Used
- Arduino Uno
- 16x2 LCD display
- Push buttons (x3)
- LED + resistor
- Potentiometer (LCD contrast)
- Breadboard and jumper wires

## How It Works
The system uses a menu-driven state machine displayed on the LCD.  
Users navigate settings with three buttons to adjust time, reminder intervals, and enable or disable reminders.  
Reminders are triggered at user-defined intervals and provide a visual alert via a blinking LED.

## Future Improvements
- Add DS3231 RTC for accurate timekeeping and date display
- Design PCB layout
- Save user settings to EEPROM
- Compact enclosure for standalone operation
