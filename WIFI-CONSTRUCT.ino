#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "A.A.A 2";
const char* password = "ADVEETH@2011";
// LCD Initialization
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define LED 14
// Button Pins
#define BTN_HOUR   5  // Up button (increase hour, move < or >)
#define BTN_MIN    23 // Down button (increase minute, move < or >)
#define BTN_OK     18 // OK button
#define BTN_SETUP  15 // Setup button
#define BTN_ACT  12
#define R1  19
#define R2  4
#define R3  26
#define R4  25
#define R5  33
#define R6  32
#define PIR1 34
#define PIR2 35
#define PIR3 27

// Buzzer Pin
#define BUZZER 13
int day = 0;
int daysweek = 0;
int inaday = 1;
int perweek[3] = {0,0,0}; //assume 7, 3 , 2
int perweekcomp[7] = {0,0,0,0,0,0,0};
bool go = false;
int numAl = 0;
// Variables for time
int h = 12, m = 0, s = 0;
bool isPM = false;

int pirState1 = LOW;            // PIR sensor state
int lastPirState1 = LOW;  

int pirState2 = LOW;            // PIR sensor state
int lastPirState2 = LOW;  

int pirState3 = LOW;            // PIR sensor state
int lastPirState3 = LOW;  

// Alarm Variables
int alarmHour[3] = {6,12,7};
int alarmMinute[3] = {0,0,0};
bool alarmPM[3] = {false,false,false};
bool alarmEnabled[3] = {false,false,false};

bool inAlarmSetting = false;
bool settingHour = true;
bool settingAlarmState = false;
bool inResetTime = false;  // Flag for Reset Time interface

// Button Debounce
unsigned long lastButtonPress = 0;
const int debounceDelay = 200;  // 200ms delay

// Function prototypes


//void setAlarmState();
void resetTime();
void setupMenu();
void displayTime();
void alarmSetting();
void resetTime();
void setupMenu();
void numAlarm();
void openACT();
void closeACT();
bool isTouched(int threshold);
void TICBUZ(int frequency, int duration);
void Placing(int comp);
void Retract(int comp, int time);
void stopAllActuators();
void resumeActuators(int comp);
void PIRSENSOR1();
void PIRSENSOR2();
void PIRSENSOR3();
void PIRSENSOR(int comp);
void playMelody();
void TimeCal();

// For navigation between settings
enum State {
  TIME_DISPLAY,
  ALARM_SETTING,
  ALARM_STATE,
  RESET_TIME,
  SETUP_MENU,
  NUM_ALARM
};

State currentState = TIME_DISPLAY;  // Start in the time display state

// Menu navigation
int setupMenuIndex = 0;  // 0 = Alarm Settings, 1 = Time Settings

HardwareSerial sensor0Serial(0);
HardwareSerial sensor2Serial(2);
// Global variables
long previousDistance2 = -1;    // To track the last known distance2 (for debugging)
int invalidCount2 = 0;          // Count consecutive invalid readings

long previousDistance0 = -1;    // To track the last known distance2 (for debugging)
int invalidCount0 = 0;


int j = -1;
bool past = false;
void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Hostname can be changed; default is esp3232-[MAC]
  ArduinoOTA.setHostname("MedicalAlert");

  // Optional: set OTA password
  // ArduinoOTA.setPassword("your_password");

  // Event handlers for OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


      pinMode(PIR1, INPUT);
      pinMode(PIR2, INPUT);
      pinMode(PIR3, INPUT);
             // Set PIR pin as input
      delay(1000);
  Serial.begin(9600);
  sensor2Serial.begin(9600, SERIAL_8N1, 16, 17);  // RX on Pin 16, TX on Pin 17
  sensor0Serial.begin(9600, SERIAL_8N1, 3, 1);

    lcd.init();
    lcd.backlight();
    pinMode(BTN_HOUR, INPUT_PULLUP);
    pinMode(BTN_MIN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_SETUP, INPUT_PULLUP);  // Corrected SETUP button pin
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    pinMode(BTN_ACT, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    
    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(R3, OUTPUT);
    pinMode(R4, OUTPUT);
    pinMode(R5, OUTPUT);
    pinMode(R6, OUTPUT);
    digitalWrite(R1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(R3, LOW);
    digitalWrite(R4, LOW);
    digitalWrite(R5, LOW);
    digitalWrite(R6, LOW);



    displayTime(); // Start with time display
}

void loop() {
    // Handle each state based on the current state
    ArduinoOTA.handle();
    switch (currentState) {
        case TIME_DISPLAY:
            displayTime();
            break;
        case ALARM_SETTING:
            alarmSetting();
            break;
        case NUM_ALARM:
            numAlarm();
            break;
        case RESET_TIME:
            resetTime();
            break;
        case SETUP_MENU:
            setupMenu();
            break;
    }

    if(digitalRead(BTN_ACT) == LOW &&  millis() - lastButtonPress > debounceDelay){
     lastButtonPress = millis();
     past = true;
      openACT();

    }
     if(digitalRead(BTN_ACT) == LOW &&  millis() - lastButtonPress > debounceDelay){
      if(past == true){
     lastButtonPress = millis();
     past = false;
      closeACT();
      }
    }

    
  


 
    // Check if alarm should ring
    for (int i = 0; i < numAl; i++) {
        if (alarmEnabled[i] && h == alarmHour[i] && m == alarmMinute[i] && isPM == alarmPM[i]) {
         PIRSENSOR(i+1);
            TICBUZ(100, 100);
            delay(20);
            TICBUZ(10, 100);
            displayTime();
             
            if(m > alarmMinute[i]){
              TICBUZ(0,0);
              
            }
           j=i;
            if(s > 13 ){
              stopAllActuators();
              if(s>30){
              digitalWrite(BUZZER, LOW);
              alarmEnabled[i] = false; // Ensure buzzer is off when alarm condition is not met
              break;}

            }
            // Debounce for BTN_OK
            if (digitalRead(BTN_OK) == LOW) {
                if (millis() - lastButtonPress > debounceDelay) {
                    lastButtonPress = millis();
                    alarmEnabled[i] = false;  // Disable the alarm
                    digitalWrite(BUZZER, LOW); 
                    ;// Stop buzzer
                }
            }
        } else {
            digitalWrite(BUZZER, LOW); // Ensure buzzer is off when alarm condition is not met
            
            stopAllActuators();
            
        }
    }
if (alarmEnabled[j] && s <= 12){

    long distance2 = readUS2100();
    long distance0 = readUS0100();  // Get a fresh reading from the ultrasonic sensor
    TimeCal();
    // If the sensor returns -1 (real error), increment invalid counter.
    if(distance2 == -1) {
        invalidCount2++;
    } else {
        invalidCount2 = 0;
    }

    if(distance0 == -1) {
        invalidCount0++;
    } else {
        invalidCount0 = 0;
    }

    // If too many consecutive invalid readings occur, "restart" the sensor:
    if(invalidCount2 > 10) {
        // Serial.println("Prolonged invalid readings. Restarting sensor...");
        sensor2Serial.flush();
        sensor2Serial.end();
        delay(500);
        sensor2Serial.begin(9600, SERIAL_8N1, 16, 17);  // Restart the sensor
        invalidCount2 = 0;
    }


    if(invalidCount0 > 10) {
        // Serial.println("Prolonged invalid readings. Restarting sensor...");
        sensor0Serial.flush();
        sensor0Serial.end();
        delay(500);
        sensor0Serial.begin(9600, SERIAL_8N1, 16, 17);  // Restart the sensor
        invalidCount0 = 0;
    }

    // Actuation control:
    if(distance2 == -1) {
        Serial.println("Sensor Error");
    }
    else if(distance2 > 0 && distance2 <= 100) {
        stopAllActuators();  // Stop actuators if distance2 is valid and <= 100mm
    }
    else { 
        resumeActuators(j);   // Resume actuators for distance2s > 100mm or out-of-range (0)
    }


    if(distance0 == -1) {
        Serial.println("Sensor Error0");
    }
    else if(distance0 > 0 && distance0 <= 100) {
        stopAllActuators();  // Stop actuators if distance2 is valid and <= 100mm
    }
    else { 
        resumeActuators(j);   // Resume actuators for distance2s > 100mm or out-of-range (0)
    }


    // Debug: update previous distance2 if needed
    if(distance2 != previousDistance2) {
        previousDistance2 = distance2;
    }

    if(distance0 != previousDistance0) {
        previousDistance0 = distance0;
    }

    delay(200);
    }  // Small delay to prevent overwhelming the loop
}

// Function to read distance2 from the US100 sensor
long readUS2100() {
    byte sensorData[2];
    long distance2 = -1;  // Default to error state
    const long maxDistance2 = 200;  // Maximum valid distance2

    // Flush any old data before sending a new request
    while (sensor2Serial.available()) {
        sensor2Serial.read();
    }

    // Request a distance2 measurement
    sensor2Serial.write(0x55);

    unsigned long startTime = millis();
    while (sensor2Serial.available() < 2) {
        if (millis() - startTime > 1000) {
            return -1;  // Timeout error (real invalid)
        }
    }

    // Read response from the sensor
    sensorData[0] = sensor2Serial.read();
    sensorData[1] = sensor2Serial.read();
    distance2 = (sensorData[0] << 8) | sensorData[1];  // Convert to mm

    // If the reading is above maxDistance2, return 0 (imaginary invalid)
    if (distance2 > maxDistance2) {
        return 0;
    }

    return distance2;
}


long readUS0100() {
    byte sensorData0[2];
    long distance0 = -1;  // Default to error state
    const long maxDistance0 = 200;  // Maximum valid distance2

    // Flush any old data before sending a new request
    while (sensor0Serial.available()) {
        sensor0Serial.read();
    }

    // Request a distance2 measurement
    sensor0Serial.write(0x55);

    unsigned long startTime0 = millis();
    while (sensor0Serial.available() < 2) {
        if (millis() - startTime0 > 1000) {
            return -1;  // Timeout error (real invalid)
        }
    }
    // Read response from the sensor
    sensorData0[0] = sensor0Serial.read();
    sensorData0[1] = sensor0Serial.read();
    distance0 = (sensorData0[0] << 8) | sensorData0[1];  // Convert to mm

    // If the reading is above maxDistance2, return 0 (imaginary invalid)
    if (distance0 > maxDistance0) {
        return 0;
    }

    return distance0;
}
// Function to stop all actuators
void stopAllActuators() {
    digitalWrite(R1, HIGH);
    digitalWrite(R2, HIGH);
    digitalWrite(R3, HIGH);
    digitalWrite(R4, HIGH);
    digitalWrite(R5, HIGH);
    digitalWrite(R6, HIGH);
}


// bool isTouched(int threshold) {
//   int touchValue = touchRead(2);  // D2 is GPIO2, which is touch-sensitive
//   return (touchValue < threshold);
// }



// Function to resume actuators (for example, extend them)
void resumeActuators(int i) {
    // Process each actuator in turn (3 actuators, 2 relays each)
    
            //displayTime();

         int relayExtend;
         int relayRetract;
        
        if(i == 0){
          relayExtend = R1;
          relayRetract = R2;
        }
        else if( i== 1){
          relayExtend = R3;
          relayRetract = R4;
        }
        else{
          relayExtend = R5;
          relayRetract = R6;
        }


        // Extension Phase
        digitalWrite(relayExtend, LOW);
        
         // Activate extension
        unsigned long phaseStart = millis();
        while(millis() - phaseStart < 10000) 
        {  // 10-second extension phase
        displayTime();
        TICBUZ(100, 100);
            delay(20);
            TICBUZ(10, 100);
            
            if(s>13){
              
              stopAllActuators();
              if(s>50){
                alarmEnabled[i] = false;
                return;
              }
            }
            long currDist2 = readUS2100();
             long currDist0 = readUS0100();
              // Read distance2 during extension
            if((currDist2  > 0 && currDist2 != -1 && currDist2 <= 100) || (currDist0  > 0 && currDist0 != -1 && currDist0 <= 100)) {
                digitalWrite(relayExtend, HIGH); // Stop if within 100mm
                break;
            }
        }
        // Retract if needed
          // Activate retraction
        delay(200); // Allow retraction phase to complete
        //digitalWrite(relayRetract, HIGH);  // Stop retraction
    
}
    
//PIR1
void PIRSENSOR(int i){
  if(i == 1) PIRSENSOR1();
  if(i == 2) PIRSENSOR2();
  if(i == 3) PIRSENSOR3();
}


void PIRSENSOR1(){
pirState1 = digitalRead(PIR1); 
 // Read PIR sensor state
 
displayTime();
  // Wait for PIR sensor to settle (skip first detection)
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis < 3000) {
    return; // Skip the first second to allow PIR to stabilize
  }
  
  // Only act when motion state changes
  if (pirState1 == HIGH && lastPirState1 == LOW) {
    // Serial.println("Motion Started");
   
    playMelody(); // a quick buzzer to confirm motion
   digitalWrite(R2, HIGH);

     // Trigger the buzzer when motion is detected
    lastPirState1 = HIGH;
  } else if (pirState1 == LOW && lastPirState1 == HIGH) {
    
    delay(5000);
    digitalWrite(R2, LOW);
   delay(15000);
   digitalWrite(R2, HIGH);
    
    // Serial.println("Motion Stopped");
    //digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer
    lastPirState1 = LOW;
  }

  delay(200);  
}


void PIRSENSOR2(){
  
pirState2 = digitalRead(PIR2);  // Read PIR sensor state
displayTime();
  // Wait for PIR sensor to settle (skip first detection)
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis < 3000) {
    return; // Skip the first second to allow PIR to stabilize
  }
  
  // Only act when motion state changes
  if (pirState2 == HIGH && lastPirState2 == LOW) {
    
    // Serial.println("Motion Started");
    playMelody(); // Trigger the buzzer when motion is detected
    digitalWrite(R4, HIGH);
    lastPirState2 = HIGH;
  }  else if (pirState2 == LOW && lastPirState2 == HIGH) {
    
    delay(5000);
    digitalWrite(R4, LOW);
   delay(15000);
   digitalWrite(R4, HIGH);
    
    // Serial.println("Motion Stopped");
    //digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer
    lastPirState2 = LOW;
  }

  delay(200);  
}



void PIRSENSOR3(){
  
pirState3 = digitalRead(PIR3);  // Read PIR sensor state
displayTime();
  // Wait for PIR sensor to settle (skip first detection)
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis < 1000) {
    return; // Skip the first second to allow PIR to stabilize
  }
  
  // Only act when motion state changes
  if (pirState3 == HIGH && lastPirState3 == LOW) {
    // Serial.println("Motion Started");
    
    playMelody();
    digitalWrite(R6, HIGH); // Trigger the buzzer when motion is detected
    lastPirState3 = HIGH;
  } else if (pirState3 == LOW && lastPirState3 == HIGH) {
    // Serial.println("Motion Stopped");
    //digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer
     
    delay(5000);
    digitalWrite(R6, LOW);
   delay(15000);
   digitalWrite(R6, HIGH);
    
    lastPirState3 = LOW;
  }

  delay(200);  
}




void TimeCal(){
  static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate >= 1000) { // Update every 1 second
        lastUpdate = millis();
        s++;

        if (s == 60) {
            s = 0;
            m++;
        }
        if (m == 60) {
            m = 0;
            h++;
            if (h == 12) isPM = !isPM; // Toggle AM/PM at 12
            if (h > 12) h = 1;
        }

        if((h == 12 && m == 0)  && isPM == false){
          day++;
          if(day > 7){
            day = 1;
          }
        }
    }

}


  
// Function to display the default time screen
void displayTime() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) { // Update every 1 second
        // lastUpdate = millis();
        // s++;
        TimeCal();
        // if (s == 60) {
        //     s = 0;
        //     m++;
        // }
        // if (m == 60) {
        //     m = 0;
        //     h++;
        //     if (h == 12) isPM = !isPM; // Toggle AM/PM at 12
        //     if (h > 12) h = 1;
        // }

        // Display Time
        //lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" MEDICAL-ALERT ");
        lcd.setCursor(0, 1);
        lcd.print("  ");
        lcd.print(h);
        lcd.print(":");

        if (m < 10) lcd.print("0"); // Format minutes properly
        lcd.print(m);
        lcd.print(":");
        if (s < 10) lcd.print("0"); // Format seconds properly
        lcd.print(s);
        lcd.print(isPM ? " PM" : " AM");

        // Check for SETUP button to move to the setup menu
        if (digitalRead(BTN_SETUP) == LOW && millis() - lastButtonPress > debounceDelay) {
            lastButtonPress = millis();
            currentState = SETUP_MENU;  // Go to the Setup Menu
            lcd.clear();  // Clear the display when changing interface
            TICBUZ(50,40);  // Robotic sound when entering setup menu
        }
    }
}

// Function to handle setup menu
void setupMenu() {
    
    lcd.setCursor(0, 0);
    if (setupMenuIndex == 0) {
        lcd.print("> Alarm Settings ");
    } else {
        lcd.print("  Alarm Settings ");
    }

    lcd.setCursor(0, 1);
    if (setupMenuIndex == 1) {
        lcd.print("> Time Settings   ");
    } else {
        lcd.print("  Time Settings   ");
    }
    TimeCal();
    // Navigate through the menu (using Up/Down buttons)
    if (digitalRead(BTN_HOUR) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        setupMenuIndex = (setupMenuIndex + 1) % 2;  // Toggle between 0 and 1
        lcd.clear();  // Clear the display on navigation
        TICBUZ(50,30);  // Robotic sound for menu navigation
    }

    if (digitalRead(BTN_MIN) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        setupMenuIndex = (setupMenuIndex + 1) % 2;  // Toggle between 0 and 1
        lcd.clear();  // Clear the display on navigation
        TICBUZ(50,30);  // Robotic sound for menu navigation
    }

    // Confirm selection with OK button
    if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        if (setupMenuIndex == 0) {
            currentState = NUM_ALARM;  // Go to Alarm Setting interface
            TICBUZ(50,30);  // Robotic sound for confirming alarm settings
        } else {
            currentState = RESET_TIME;  // Go to Reset Time interface
            TICBUZ(50,30);  // Robotic sound for confirming time settings
        }
        lcd.clear();  // Clear the display when changing interface
    }
}



// Function to handle alarm setting interface
void alarmSetting() {
  TimeCal();
    int i = 0; // Start with the first alarm
    bool settingAlarm = true;
    if (numAl == 0){
      alarmEnabled[0] = false;
      alarmEnabled[1] = false;
      alarmEnabled[2] = false;
      currentState = TIME_DISPLAY;
      return;

    }

    while (settingAlarm) {
        lcd.setCursor(0, 0);
        lcd.print("Set Alarm ");
        lcd.print(i+1);
        lcd.print(": ");
        lcd.setCursor(0, 1);
        lcd.print(alarmHour[i]);
        lcd.print(":");
        if (alarmMinute[i] < 10) lcd.print("0");
        lcd.print(alarmMinute[i]);
        lcd.print(alarmPM[i] ? " PM" : " AM");

        // Adjust Hour
        if (digitalRead(BTN_HOUR) == LOW && millis() - lastButtonPress > debounceDelay) {
            lastButtonPress = millis();
            alarmHour[i]++;
            if (alarmHour[i] == 12) alarmPM[i] = !alarmPM[i]; // Toggle AM/PM at 12
            if (alarmHour[i] > 12) alarmHour[i] = 1; // Loop from 1 to 12
            TICBUZ(50, 40); // Robotic sound for hour adjustment
            lcd.clear(); // Update display only when needed
        }

        // Adjust Minutes
        if (digitalRead(BTN_MIN) == LOW && millis() - lastButtonPress > debounceDelay) {
            lastButtonPress = millis();
            alarmMinute[i]++;
            if (alarmMinute[i] == 60) alarmMinute[i] = 0;
            TICBUZ(50, 30); // Robotic sound for minute adjustment
            lcd.clear(); // Update display only when needed
        }

        // Confirm the alarm time setting
        if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
            lastButtonPress = millis();
            alarmEnabled[i] = true; // Enable the alarm after setting time
            TICBUZ(50, 30); // Robotic sound for confirming alarm time
            Placing(i+1);
            delay(10);

              if (i+1 >= numAl) { 
                currentState = TIME_DISPLAY; // Move to time display after setting all alarms
                settingAlarm = false;
            } else {
                i++; // Move to next alarm
                lcd.clear();
            }
        }
    }
}


void numAlarm(){
  TimeCal();
  lcd.setCursor(0,0);
  lcd.print("No.of Alarms: ");
  lcd.setCursor(0,1);
  lcd.print(numAl);
  //lcd.print(": ");

  if (digitalRead(BTN_HOUR) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        numAl++;
        if (numAl >3){
          numAl =0;
        }
        TICBUZ(50,40);  // Robotic sound for hour adjustment
    }

    if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        
        TICBUZ(50,30);  // Robotic sound for confirming alarm time
        currentState = ALARM_SETTING;  // Go to Alarm State
        lcd.clear();
    }

}

void Expand(int i){
  TimeCal();
  if(i == 1){
    digitalWrite(R1,LOW);
  delay(13000);
  digitalWrite(R1,HIGH);
  delay(10);

  }
  else if( i == 2){
    digitalWrite(R3,LOW);
  delay(13000);
  digitalWrite(R3,HIGH);
  delay(10);
  }
  else{
    if(i == 3){

      digitalWrite(R5,LOW);
  delay(13000);
  digitalWrite(R5,HIGH);
  delay(10);
    }
  }
}



void Placing(int i){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Please Wait..");
  lcd.setCursor(0,1);
  lcd.print("GIVE WAY!");
  TimeCal();
  Expand(i);
  delay(10);

  while(true){
    TimeCal();
  lcd.clear();
  delay(100);
  lcd.setCursor(0,0);
  lcd.print("Please drop your");
  lcd.setCursor(0,1);
  lcd.print("Medicine here...");
  delay(200);

  if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Please Wait...");
        Retract(i,14000);
        lcd.clear();
        lcd.setCursor(0,0);
       lcd.print("done!");
       delay(200);
       lcd.clear();
       TICBUZ(100,200);
        break;
              }            
}
 return;
}

void Retract(int i, int time){
  TimeCal();
  if(i == 1){
    digitalWrite(R2,LOW);
  delay(time);
  digitalWrite(R2,HIGH);
  delay(10);

  }
  else if( i == 2){
    digitalWrite(R4,LOW);
  delay(time);
  digitalWrite(R4,HIGH);
  delay(10);
  }
  else{
    if(i == 3){

      digitalWrite(R6,LOW);
  delay(time);
  digitalWrite(R6,HIGH);
  delay(10);
    }
  }
}

// Function to reset time (for setting current time)
void resetTime() {
    lcd.setCursor(0, 0);
    lcd.print("Reset Time:");

    lcd.setCursor(0, 1);
    lcd.print(h);
    lcd.print(":");
    if (m < 10) lcd.print("0"); 
    lcd.print(m);
    lcd.print(isPM ? " PM" : " AM");  // Display AM/PM

    // Adjust Hour
    if (digitalRead(BTN_HOUR) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        h++;
        if (h == 12) isPM = !isPM;  // Toggle AM/PM at 12
        if (h > 12) h = 1;          // Keep hours within 1-12
        lcd.clear(); // Update display only when needed
    }

    // Adjust Minutes
    if (digitalRead(BTN_MIN) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        m++;
        if (m >= 60) m = 0;
        lcd.clear(); // Update display only when needed
    }

    // Confirm Time Reset and go back to time display
    if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        currentState = TIME_DISPLAY;
        TICBUZ(2000, 100);
        lcd.clear();
    }
}

void timesaweek(int i){

  while(true){
    
  lcd.setCursor(0,0);
  lcd.print("How many times ");
  lcd.setCursor(0,1);
  lcd.print("a week: ");
  
  if (digitalRead(BTN_HOUR) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        daysweek++;
        lcd.setCursor(8,1);
        lcd.print(daysweek);

  }
  if (digitalRead(BTN_OK) == LOW && millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();  
        perweek[i-1] = daysweek;
        lcd.clear();
        break;

  }
  lcd.clear();
  }




}

// Function to play a robotic buzzer sound
void TICBUZ(int frequency,int duration) {
    // Slight delay between beeps for robotic feel
 

         tone(BUZZER, frequency + 2000, duration);  // Slightly higher frequency for the second beep
        delay(100);
        
    }

void openACT(){
  displayTime();
   digitalWrite(R2, LOW);
   delay(20000);
   digitalWrite(R2, HIGH);
    delay(10);
    digitalWrite(R4, LOW);
    delay(20000);
    digitalWrite(R4, HIGH);
    delay(10);
    digitalWrite(R6, LOW);
    delay(20000);
    digitalWrite(R6, HIGH);
    delay(10);

}

void closeACT(){
  displayTime();
  digitalWrite(R1, LOW);
   delay(20000);
   digitalWrite(R1, HIGH);
    delay(10);
    digitalWrite(R3, LOW);
    delay(20000);
    digitalWrite(R3, HIGH);
    delay(10);
    digitalWrite(R5, LOW);
    delay(20000);
    digitalWrite(R5, HIGH);
    delay(10);
}

// void try(int i){
//   switch(perweek[i]){
//     case 7:
//     go = true;
//     break;
//     case 4:
//     if((day)){
//       go =true;
//       day+2 = true;
//     }
//     else {
//       go = false;
//     }
//     break;
     
//   }
// }



void playMelody() {
  // Define the melody and durations inside the function
  
  // Melody notes (C major scale, simplified tune)
  
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    displayTime();
    tone(BUZZER, 1000, 200);
    delay(250);
    tone(BUZZER, 1200, 200);
    delay(250);
  }
  noTone(BUZZER);
}

