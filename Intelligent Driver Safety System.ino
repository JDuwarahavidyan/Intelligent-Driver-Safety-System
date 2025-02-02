#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pin connections
const int rs = 12, en = 11, d4 = 10, d5 = 6, d6 = A1, d7 = A2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// GSM module pins
SoftwareSerial mySerial(7, 8);

// Define pin numbers for components
const int eyeSensorPin = 2;        // Eye sensor pin
const int alcoholSensorPin = A0;   // MQ3 alcohol sensor pin
const int motorPin = 3;            // Motor pin
const int buzzerPin = 4;           // Buzzer pin
const int Led = 5;                 // LED pin
const int safeToDrivePin = 9;      // Pin used to power SIM900 module

const int THRESHOLD_VALUE = 400;
bool eyesClosed = false;
unsigned long eyesClosedStartTime = 0;

void setup() {
    // Initialize LCD
    lcd.begin(16, 2);

    // Start serial communication
    Serial.begin(9600);
    mySerial.begin(9600);

    // Pin setup
    pinMode(eyeSensorPin, INPUT);
    pinMode(alcoholSensorPin, INPUT);
    pinMode(motorPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(Led, OUTPUT);

    // Power on GSM module
    SIM900power();

    // Initialize GSM module
    mySerial.println("AT");
    updateSerial();
    delay(1000);
    mySerial.println("AT+CMGF=1"); // Set SMS mode to text
    updateSerial();

    // Welcome message on LCD
    lcd.print("Welcome");
    delay(500);
    lcd.clear();
    lcd.print("Starting.....");
    delay(500);
}

void loop() {
    int eyeSensorValue = digitalRead(eyeSensorPin);
    int alcoholSensorValue = analogRead(alcoholSensorPin);

    if (eyeSensorValue == LOW) {
        if (!eyesClosed) {
            eyesClosedStartTime = millis();
            eyesClosed = true;
        }

        if (millis() - eyesClosedStartTime >= 1000) {
            activateAlert("Driver asleep!", "+94779891954", "Driver fell asleep!");
        }
    } else {
        eyesClosed = false;
    }

    if (alcoholSensorValue > THRESHOLD_VALUE) {
        activateAlert("Driver intoxicated!", "+94779891954", "Driver is intoxicated!");
    } else {
        digitalWrite(motorPin, LOW);
        digitalWrite(Led, HIGH);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Safe to drive");
        delay(2000);
    }
}

// Function to activate alerts (Buzzer, LCD, SMS)
void activateAlert(String lcdMessage, String phoneNumber, String smsMessage) {
    digitalWrite(motorPin, HIGH);
    digitalWrite(Led, LOW);
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(lcdMessage);
    delay(1000);

    sendSMS(phoneNumber, smsMessage);
    delay(5000);
}

// Function to power on SIM900 module
void SIM900power() {
    pinMode(safeToDrivePin, OUTPUT);
    digitalWrite(safeToDrivePin, LOW);
    delay(1000);
    digitalWrite(safeToDrivePin, HIGH);
    delay(2000);
    digitalWrite(safeToDrivePin, LOW);
    delay(3000);
}

// Function to update serial communication
void updateSerial() {
    delay(500);
    while (Serial.available()) {
        mySerial.write(Serial.read());
    }
    while (mySerial.available()) {
        Serial.write(mySerial.read());
    }
}

// Function to send SMS
void sendSMS(String number, String message) {
    mySerial.println("AT+CMGS=\"" + number + "\"");
    updateSerial();
    delay(1000);
    mySerial.print(message);
    mySerial.write(26); // CTRL+Z to send the message
    updateSerial();
}
