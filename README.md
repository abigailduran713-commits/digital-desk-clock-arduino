# Digital Desk Clock (Arduino)

## Project Demo
![Clock Schematic](images/Clock_Schematic.jpg)
![Clock Running](images/Clock_Running.jpg)
![Reminder Active](images/Clock_Reminder.jpg)

## Project Overview
This project is a standalone digital desk clock built using an Arduino Uno.  
It features an LCD user interface, a three-button menu system, interval-based reminders, and a visual LED alert.  
The system was designed using non-blocking timing to remain responsive while handling user input and reminders.

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
