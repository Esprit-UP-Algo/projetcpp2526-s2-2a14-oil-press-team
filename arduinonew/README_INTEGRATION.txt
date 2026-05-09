================================================================================
MAINTENANCE TAB ARDUINO INTEGRATION GUIDE
================================================================================

This folder contains everything you need to implement the Arduino MQ-2 gas sensor
and hardware alarm system in your updated project.

Files included:
1. smoke_gas_intervention.ino (Upload this to your Arduino UNO/Mega)
2. qt_sync_logic.cpp          (C++ code snippets to add to your new Qt App)

--------------------------------------------------------------------------------
STEP 1: ARDUINO HARDWARE SETUP
--------------------------------------------------------------------------------
Wire your Arduino components as follows:
- MQ-2 Gas Sensor (A0) (Analog Out)
- 5V Relay          (Digital Pin 6) -> Connects to fan or exhaust.
- Active Buzzer     (Digital Pin 7)
- Red LED           (Digital Pin 8) -> Indicates "Danger" state
- Green LED         (Digital Pin 9) -> Indicates "Safe" state

Upload `smoke_gas_intervention.ino` using the Arduino IDE. 
Make sure the baud rate in the serial monitor is 115200.

--------------------------------------------------------------------------------
STEP 2: QT PROJECT CONFIGURATION
--------------------------------------------------------------------------------
In your NEW project, you MUST add the serial port module.
1. Open your `.pro` file.
2. Find the line starting with `QT += ...` and add `serialport`.
   Example: 
   QT += core gui sql serialport

--------------------------------------------------------------------------------
STEP 3: QT C++ INTEGRATION
--------------------------------------------------------------------------------
In your new project's `mainwindow.h`:
1. Include the necessary headers:
   #include <QSerialPort>
   #include <QTimer>
   #include <QSqlQuery>
   #include <QDebug>

2. Add the QSerialPort pointer to your MainWindow class:
   private:
       QSerialPort *serial;

In your new project's `mainwindow.cpp`:
1. Copy the code from `qt_sync_logic.cpp` into your MainWindow constructor.
2. Make sure your COM port matches your Arduino (Change "COM9" to whatever port your Arduino is on).
3. Ensure your database connection is open before the QTimer starts polling.

How it works:
- The Qt app listens for "ALERT_GAS:1" from the Arduino.
- When it hears it, it updates the MACHINE table in your database to 'En danger'.
- The polling timer (checking every 2s) sees the database is 'En danger' and sends "DANGER" back to Arduino to sound the buzzer/relay.
- When an admin marks the machine as normal in the Maintenance Tab, Qt sends "SAFE", turning off the alarm.
