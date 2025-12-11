MediAlert – Automated Medication Dispensing System

MediAlert is a smart, semi-automatic medication dispensing device designed to assist elderly individuals, patients with chronic conditions, and hospitals in managing medication schedules accurately and safely.
It opens the correct compartment at the correct time, detects whether medicine was taken, alerts users if a dose is missed, and ensures obstacle-free operation using motion and ultrasonic sensors.

📌 Project Overview

Medication non-adherence is a major global problem, often leading to health complications and hospitalizations.
MediAlert provides a hardware + software solution that automates reminders, compartment opening, and dose tracking using sensors and relays.

This project uses:

ESP32 as the main microcontroller

Linear actuators to slide compartments

PIR motion sensors to detect hand presence

Ultrasonic sensors to detect obstacles

RGB LED modules (red only) for missed-dose indication

Buttons for manual override and menu navigation

LCD display for user interface

Mini UPS for power backup

✨ Key Features

✔️ Automatically opens the correct medicine compartment at the scheduled time
✔️ Detects if user takes medicine (motion sensor)
✔️ Detects obstructions (ultrasonic sensors) before moving actuators
✔️ Alerts user via buzzer and display
✔️ Missed-dose indication through red LEDs
✔️ Manual compartment open/close buttons
✔️ 5–6 main control buttons (Power, Up, Down, OK, Reset)
✔️ Bluetooth-based mobile app/web interface (future update)
✔️ Battery backup for uninterrupted function

📦 Hardware Components
Main Electronics

ESP32 Development Board

6× Relay modules (control linear actuators)

LCD1602 Display with I2C

Piezo buzzer

RGB LED modules (using red only)

Push buttons (matrix + compartment buttons)

Sensors

3× PIR motion sensors

3× US-100 ultrasonic sensors

Movement System

3× Linear actuators (for compartment sliding)

Power System

12V/5V power adapter

Mini UPS (RouterUPS)

DC-DC Buck converters

Power switch (ON/OFF)

Mechanical Assembly

MDF boards

Screws, bolts, nuts

Zip ties, glue, duct tape

Drill kit

⚙️ System Architecture
1. Input Components

Button matrix (UP, DOWN, OK, RESET, etc.)

Compartment buttons (manual open/close)

Motion sensors

Ultrasonic sensors

2. Processing

ESP32 handles:

Timer logic

Sensor readings

Actuator control via relays

LED + buzzer alerts

LCD display interface

3. Output Components

Actuator movement

Alerts (LED + buzzer + LCD)

Compartment opening

🔌 Wiring Summary
Powered by ESP32

Sensors (PIR + ultrasonic)

Buttons

LEDs

Powered Externally (via relays)

Linear actuators

Important Wiring Notes

All grounds (sensors, relays, ESP32, power supplies) must be common ground.

Use series resistors for LED pins

Use voltage divider for ultrasonic echo pins (5V → 3.3V ESP32 safety)

Button matrix uses 3 rows + 2 columns = 5 GPIO pins

🧠 Software Flow

System boots → LCD shows current time

Timer triggers → Buzzer plays

Correct compartment automatically opens

Ultrasonic sensor checks for obstacles

Motion sensor checks if user takes medicine

If user doesn’t take medicine → LED turns red + buzzer alert

Any button press resets warnings

Compartment buttons allow manual opening/closing

📱 Future Mobile/Web App (Bluetooth)

Planned features:

Connect to ESP32 via Bluetooth

Set medication schedules

Choose compartments

View missed-dose history

Toggle alarms

🛠 Setup & Assembly

Assemble MDF box

Install compartments & actuators

Mount sensors near compartments

Install LED modules & buttons

Complete wiring (leave ESP32 wires labeled until it arrives)

Upload firmware to ESP32

Test each module individually

Integrate full system

🧪 Testing Procedure

Test relays & actuators

Test button matrix

Test ultrasonic obstacle detection

Test PIR motion detection

Test LED & buzzer

Test full timed cycle with mock medicine

🚀 Why MediAlert?

Reduces nursing workload in hospitals

Helps elderly patients remember medication

Prevents missed doses

Provides semi-automation without high cost

Practical, scalable, and manufacturable

📚 License

This project is open for educational and prototyping use.
