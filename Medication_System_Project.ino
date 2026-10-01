#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <Servo.h>

RTC_DS3231 rtc;

// LCD Pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

Servo dispenserServo;

// ---------------- Pins ----------------
const int servoPin = 9;
const int buzzerPin = 8;
const int ledPin = 7;
const int joystickPin = 6;

// =======================================================
// SERVO START POSITION
// =======================================================

const int START_ANGLE = 0;
int servoAngle = START_ANGLE;

// =======================================================
// SINGLE DOSE TIME (hour, minute, second)
// =======================================================

int doseTime[3] = {15, 46, 10};

// =======================================================

bool doseTriggered = false;
bool alarmActive = false;

unsigned long lastLCDUpdate = 0;

// =======================================================

void setup() {

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();

  if (!rtc.begin()) {
    lcd.print("RTC ERROR");
    while (1);
  }

  // Set RTC time from computer (UPLOAD ONCE, THEN COMMENT OUT)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(joystickPin, INPUT_PULLUP);

  dispenserServo.attach(servoPin);

  // Force servo to start position
  servoAngle = START_ANGLE;
  dispenserServo.write(servoAngle);
  delay(500);

  lcd.print("Medication");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
  lcd.clear();
}

// =======================================================

void loop() {

  DateTime now = rtc.now();

  updateLCD(now);
  checkDose(now);
  handleAlarm();

}

// =======================================================
// LCD DISPLAY
// =======================================================

void updateLCD(DateTime now) {

  if (millis() - lastLCDUpdate < 500) return;
  lastLCDUpdate = millis();

  lcd.setCursor(0, 0);

  print2(now.hour());
  lcd.print(":");
  print2(now.minute());
  lcd.print(":");
  print2(now.second());
  lcd.print("   ");

  lcd.setCursor(0, 1);

  lcd.print("Dose ");
  print2(doseTime[0]);
  lcd.print(":");
  print2(doseTime[1]);
  lcd.print(":");
  print2(doseTime[2]);
  lcd.print("   ");
}

// helper
void print2(int num) {
  if (num < 10) lcd.print("0");
  lcd.print(num);
}

// =======================================================
// DOSE CHECK (ONCE ONLY)
// =======================================================

void checkDose(DateTime now) {

  if (doseTriggered) return;

  if (now.hour() == doseTime[0] &&
      now.minute() == doseTime[1] &&
      now.second() == doseTime[2]) {

    triggerDose();
  }
}

// =======================================================
// TRIGGER DOSE
// =======================================================

void triggerDose() {

  alarmActive = true;
  doseTriggered = true;

  servoAngle += 55;

  if (servoAngle > 180) servoAngle = 180;

  dispenserServo.write(servoAngle);
}

// =======================================================
// ALARM HANDLING
// =======================================================

void handleAlarm() {

  if (!alarmActive) return;

  lcd.setCursor(0, 0);
  lcd.print("TAKE MEDICATION ");
  lcd.setCursor(0, 1);
  lcd.print("Press Button   ");

  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, 1000);

  if (digitalRead(joystickPin) == LOW) {

    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);

    alarmActive = false;
    delay(300);

    // reset for next cycle (next day logic)
    doseTriggered = false;

    servoAngle = START_ANGLE;
    dispenserServo.write(servoAngle);
    delay(300);

    lcd.clear();
  }
}