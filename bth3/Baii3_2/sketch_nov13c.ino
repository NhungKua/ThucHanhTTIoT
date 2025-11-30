#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define MODE_BTN 2
#define SET_BTN 3
#define UP_BTN 4
#define DOWN_BTN 5
#define BUZZER 6

volatile bool modePressed = false;
volatile bool setPressed = false;
unsigned long modePressStartTime = 0;

int mode = 0;
int subMode = 0;
int alarmHour = 0, alarmMin = 0;
bool alarmOn = false;

unsigned long lastUpdate = 0;
bool isRinging = false;

void modeISR() {
  static unsigned long last = 0;
  if (millis() - last > 200) modePressed = true;
  last = millis();
}

void setISR() {
  static unsigned long last = 0;
  if (millis() - last > 200) setPressed = true;
  last = millis();
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(SET_BTN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(MODE_BTN), modeISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SET_BTN), setISR, FALLING);

  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.print("RTC not found");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  alarmHour = EEPROM.read(0);
  alarmMin = EEPROM.read(1);
  alarmOn = EEPROM.read(2);
}

void displayTime(DateTime now) {
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  lcd.setCursor(0, 1);
  lcd.print("AL:");
  if (alarmHour < 10) lcd.print("0");
  lcd.print(alarmHour);
  lcd.print(":");
  if (alarmMin < 10) lcd.print("0");
  lcd.print(alarmMin);
  lcd.print(alarmOn ? " ON " : " OFF");
}

void setAlarm() {
  lcd.clear();
  lcd.print("Set Alarm");
  delay(800);
  lcd.clear();
  subMode = 0;

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Hour:");
    if (alarmHour < 10) lcd.print("0");
    lcd.print(alarmHour);

    lcd.setCursor(0, 1);
    lcd.print("Min :");
    if (alarmMin < 10) lcd.print("0");
    lcd.print(alarmMin);

    if (digitalRead(UP_BTN) == LOW) {
      if (subMode == 0) alarmHour = (alarmHour + 1) % 24;
      else alarmMin = (alarmMin + 1) % 60;
      delay(200);
    }

    if (digitalRead(DOWN_BTN) == LOW) {
      if (subMode == 0) alarmHour = (alarmHour + 23) % 24;
      else alarmMin = (alarmMin + 59) % 60;
      delay(200);
    }

    if (setPressed) {
      setPressed = false;
      subMode++;
      if (subMode > 1) {
        EEPROM.write(0, alarmHour);
        EEPROM.write(1, alarmMin);
        lcd.clear();
        lcd.print("Saved!");
        delay(800);
        lcd.clear();
        return;
      }
    }
  }
}

void setTime() {
  lcd.clear();
  lcd.print("Set Time");
  delay(800);
  lcd.clear();

  DateTime now = rtc.now();
  int h = now.hour(), m = now.minute();
  subMode = 0;

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Hour:");
    if (h < 10) lcd.print("0");
    lcd.print(h);

    lcd.setCursor(0, 1);
    lcd.print("Min :");
    if (m < 10) lcd.print("0");
    lcd.print(m);

    if (digitalRead(UP_BTN) == LOW) {
      if (subMode == 0) h = (h + 1) % 24;
      else m = (m + 1) % 60;
      delay(200);
    }

    if (digitalRead(DOWN_BTN) == LOW) {
      if (subMode == 0) h = (h + 23) % 24;
      else m = (m + 59) % 60;
      delay(200);
    }

    if (setPressed) {
      setPressed = false;
      subMode++;
      if (subMode > 1) {
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), h, m, 0));
        lcd.clear();
        lcd.print("Updated!");
        delay(800);
        lcd.clear();
        return;
      }
    }
  }
}

void checkAlarm(DateTime now) {
  if (alarmOn && now.hour() == alarmHour && now.minute() == alarmMin && now.second() < 10) {
    isRinging = true;
  }

  if (isRinging) {
    tone(BUZZER, 1000);
    if (setPressed || modePressed) {
      setPressed = false;
      modePressed = false;
      isRinging = false;
      noTone(BUZZER);
    }
  } else {
    noTone(BUZZER);
  }
}

void loop() {
  DateTime now = rtc.now();
  bool mode_is_down = (digitalRead(MODE_BTN) == LOW);

  if (mode_is_down) {
    if (modePressStartTime == 0) modePressStartTime = millis();
    else if (millis() - modePressStartTime > 2000) {
      alarmOn = !alarmOn;
      EEPROM.write(2, alarmOn);
      lcd.clear();
      lcd.print(alarmOn ? "Alarm ON" : "Alarm OFF");
      delay(800);
      lcd.clear();
      while (digitalRead(MODE_BTN) == LOW);
      modePressStartTime = 0;
      modePressed = false;
    }
  } else {
    if (modePressed && modePressStartTime > 0) {
      modePressed = false;
      mode++;
      if (mode > 2) mode = 0;
      switch (mode) {
        case 1: setAlarm(); break;
        case 2: setTime(); break;
      }
      lcd.clear();
    }
    modePressStartTime = 0;
  }

  if (mode == 0 && millis() - lastUpdate > 500) {
    lastUpdate = millis();
    displayTime(now);
  }

  checkAlarm(now);
}

