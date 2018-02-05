/* LOX Code 0.1
   Delayed Gratification Actuator by Maximilian Wolfs
   13 - 10 - 2017
   Pin 2  is connected to the Interrupt Top Button
   Pin A2 is connected to the Rotary Button Pin B
   Pin A3 is connected to the Rotary Button Pin A
   Pin A4 is connected to the SDA of DS3231
   Pin A5 is connected to the SCL of DS3231
   Pin 5  is connected to the Rotary Button
   Pin 8  is connected to the Solenoid Control Circuit
   Pin 10 is connected to the LOAD(/CS)-pin of the first MAX7221
   Pin 11 is connected to the CLK-pin of the first MAX7221
   Pin 12 is connected to the DATA IN-pin of the first MAX7221
*/

#include <Wire.h>
#include <DS3231low.h>
#include <avr/sleep.h>
#include <LedControl.h>
#include <elapsedMillis.h>
#include <RotaryEncoder.h>
#include <Bounce2.h>

#define SECONDSINDAY 86400
#define SECONDSINHOUR 3600
#define SECONDSINMINUTE 60

const byte BUTTON = 5;
const byte TOPBUTTON = 2;
const int SOLENOID =  8;

//TODO: DS3231 Daily Interrupt Counter
// volatile int dailyInterrupt;

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(A2, A3);

//  global delay time
int delaytime = 250;

// Put your target date here.
/*
int targetYear;
int targetMonth;
int targetDay;
int targetHour;
int targetMinute;
int targetSecond;
*/
  int targetYear = 17;
  int targetMonth = 10;
  int targetDay = 13;
  int targetHour = 21;
  int targetMinute = 0;
  int targetSecond = 0;

// Numbers for calculations
int v;
int r;
int ones;
int tens;
int hundreds;
int thousands;
int tenthousands;
int hundredthousands;
int millions;
int tenmillions;
int hundredmillions;
unsigned int monthdays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
bool century, h24, ampm;
byte year, month, day, hour, minute, second, DoW;
unsigned long remSeconds, remMinutes, remHours, remDays;
unsigned long targetDate;
unsigned long Now, dT;

unsigned long secondsSince2000(unsigned int year, byte month, byte day, byte hour, byte minute, byte second) {
  unsigned long sse = (((unsigned long)year) * 365 * 24 * 60 * 60)   +   ((((unsigned long)year + 3) >> 2) +
                      ((unsigned long)year % 4 == 0 && (unsigned long)month > 2)) * 24 * 60 * 60   +   \
                      ((unsigned long)monthdays[month - 1] + (unsigned long)day - 1) * 24 * 60 * 60   +
                      ((unsigned long)hour * 60 * 60)   +   ((unsigned long)minute * 60)   + (unsigned long)second;

  return sse;
}

unsigned int interval = 1000;
elapsedMillis timeElapsed;
Bounce debouncer = Bounce();

DS3231 RTC;

LedControl lc = LedControl(12, 11, 10, 1);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(SOLENOID, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  debouncer.attach(5);
  debouncer.interval(50);
  digitalWrite (TOPBUTTON, HIGH);                                               // internal pull-up resistor
  attachInterrupt (digitalPinToInterrupt (TOPBUTTON), switchPressed, CHANGE);   // attach interrupt handler
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  targetDate = secondsSince2000(targetYear, targetMonth, targetDay, targetHour, targetMinute, targetSecond);
}

void switchPressed () {
  if (digitalRead (TOPBUTTON) == HIGH)
    sleepNow();
  else
    loop();
}  // end of switchPressed

void idle() {
  long idleStart = millis();
  while (idleStart + 10000 > millis())  {
    encoder.tick();
    debouncer.update();
    static int pos = 0;

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      dateSetter();
    }
  }
  sleepNow();

}

void dateSetted () {

  lc.clearDisplay(0);
  lc.setDigit(0, 7, 1, false);
  delay(800);
  lc.setDigit(0, 6, 1, false);
  delay(550);
  lc.setDigit(0, 5, 1, false);
  delay(525);
  lc.setDigit(0, 4, 1, false);
  delay(425);
  lc.setDigit(0, 3, 1, false);
  delay(325);
  lc.setDigit(0, 2, 1, false);
  delay(225);
  lc.setDigit(0, 1, 1, false);
  delay(125);
  lc.setDigit(0, 0, 1, false);
  delay(500);
  lc.clearDisplay(0);
  updateTime();

}

void stateHours() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetHour = abs(encoder.getPosition()) % 24;

    if (pos != targetHour) {
      pos = targetHour;

      int ones = targetHour % 10;
      int tens = targetHour / 10 % 10;
      //Serial.print("targetHour: "); Serial.printgetHour);

      lc.setRow(0, 7, B00010111);
      lc.setRow(0, 6, B00000101);
      lc.setDigit(0, 5, 5, false);

      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);

    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void stateMinutes() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetMinute = abs(encoder.getPosition()) % 60;

    if (pos != targetMinute) {
      pos = targetMinute;

      int ones = targetMinute % 10;
      int tens = targetMinute / 10 % 10;
      //Serial.print("targetMinute: "); Serial.println(targetMinute);

      lc.setRow(0, 7, B00010101);
      lc.setRow(0, 6, B00010000);
      lc.setRow(0, 5, B00010101);


      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);
    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void stateSeconds() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetSecond = abs(encoder.getPosition()) % 60;

    if (pos != targetSecond) {
      pos = targetSecond;

      int ones = targetSecond % 10;
      int tens = targetSecond / 10 % 10;

      //Serial.print("targetSecond: "); Serial.println(targetSecond);

      lc.setDigit(0, 7, 5, false);
      lc.setRow(0, 6, B01001111);
      lc.setRow(0, 5, B00001101);

      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);
    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void stateDays() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetDay = abs(encoder.getPosition()) % 32;

    if (pos != targetDay) {
      pos = targetDay;

      int ones = targetDay % 10;
      int tens = targetDay / 10 % 10;
      //Serial.print("targetDay: "); Serial.println(targetDay);

      lc.setChar(0, 7, 'd', false);
      lc.setRow(0, 6, B01110111);
      lc.setRow(0, 5, B00111011);

      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);
    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void stateMonths() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetMonth = abs(encoder.getPosition()) % 13;

    if (pos != targetMonth) {
      pos = targetMonth;

      int ones = targetMonth % 10;
      int tens = targetMonth / 10 % 10;
      //Serial.print("targetMonth: "); Serial.println(targetMonth);

      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);

      lc.setRow(0, 7, B00010101);
      lc.setRow(0, 6, B00011101);
      lc.setRow(0, 5, B00010101);

    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void stateYears() {
  while (1) {
    encoder.tick();
    debouncer.update();
    static int pos = 0;
    targetYear = abs(encoder.getPosition()) % 100;

    if (pos != targetYear) {
      pos = targetYear;

      int ones = targetYear % 10;
      int tens = targetYear / 10 % 10;
      //Serial.print("targetYear: "); Serial.println(targetYear);

      lc.setRow(0, 7, B00111011);
      lc.setRow(0, 6, B00000101);
      lc.setDigit(0, 5, 5, false);

      lc.setDigit(0, 2, 0, false);
      lc.setDigit(0, 3, 2, false);

      lc.setDigit(0, 0, ones, false);
      lc.setDigit(0, 1, tens, false);
    }

    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (timeElapsed >= interval) {
        dateSetted();
      }
      else {
        break;
      }
    }
  }
}

void dateSetter() {
  boolean flag = true;

  while (flag) {
    lc.setDigit(0, 7, 5, false);
    lc.setChar(0, 6, 'e', false);
    lc.setDigit(0, 5, 7, false);
    delay(500);
    stateHours();
    lc.clearDisplay(0);
    stateMinutes();
    lc.clearDisplay(0);
    stateSeconds();
    lc.clearDisplay(0);
    stateDays();
    lc.clearDisplay(0);
    stateMonths();
    lc.clearDisplay(0);
    stateYears();
    lc.clearDisplay(0);
    /*Serial.print("Date: ");
      Serial.print(targetHour);
      Serial.print(":");
      Serial.print(targetMinute);
      Serial.print(":");
      Serial.print(targetSecond);
      Serial.print(" - ");
      Serial.print(targetDay);
      Serial.print(".");
      Serial.print(targetMonth);
      Serial.print(".20");
      Serial.println(targetYear);*/
    targetDate = secondsSince2000(targetYear, targetMonth, targetDay, targetHour, targetMinute, targetSecond);
    if (debouncer.fell()) timeElapsed = 0;
    if (debouncer.rose()) {
      if (true) {
        lc.clearDisplay(0);
        flag = false;
        dateSetted();
      }
      else {
        // dateSetter();
      }
    }
  }
}



void updateTime() {

  for (int i = 0; i <= 20; i++) {               // refresh Display 20 times, roughly 20 seconds, then go to sleep mode

    year = RTC.getYear();
    month = RTC.getMonth(century);
    if (century) year += 100;
    day = RTC.getDate();
    hour = RTC.getHour(h24, ampm);
    if ((h24) && (ampm)) hour += 12;
    minute = RTC.getMinute();
    second = RTC.getSecond();
    Now = secondsSince2000(year, month, day, hour, minute, second);

    if (Now >= targetDate) {
      celebrate();
    }

    dT = targetDate - Now;                     // Here's the time remaining.
    remDays = dT / SECONDSINDAY;               // Figure out whole days remaining
    remSeconds = dT % SECONDSINDAY;            // Here's the leftover HMS
    remHours = remSeconds / SECONDSINHOUR;     // Figure out the whole hours remaining
    remSeconds = remSeconds % SECONDSINHOUR;   // Here's the leftover MS
    remMinutes = remSeconds / SECONDSINMINUTE; // Figure out the whole minutes remaining
    remSeconds = remSeconds % SECONDSINMINUTE; // Here's the leftover S

    ones = remSeconds % 10;
    remSeconds = remSeconds / 10;
    tens = remSeconds % 10;

    hundreds = remMinutes % 10;
    remMinutes = remMinutes / 10;
    thousands = remMinutes % 10;

    tenthousands = remHours % 10;
    remHours = remHours / 10;
    hundredthousands = remHours % 10;

    millions = remDays % 10;
    remDays = remDays / 10;
    tenmillions = remDays % 10;
    remDays = remDays / 10;
    hundredmillions = remDays % 10;

      Serial.print("remaining Seconds: ");Serial.println(dT);
    /*Serial.print("Now: ");Serial.println(Now);                            //Debug Time and Countdown in Serial Monitor
      Serial.print("targetDate: ");Serial.println(targetDate);
      Serial.print("targetDate - Now: "); Serial.println(dT);
      Serial.print("remaining Seconds: ");Serial.println(remSeconds);
      Serial.print("ones: ");Serial.println(ones);
      Serial.print("tens: ");Serial.println(tens);
      Serial.print("hundreds: ");Serial.println(hundreds);
      Serial.print("thousands: ");Serial.println(thousands);
      Serial.print("tenthousands: ");Serial.println(tenthousands);
      Serial.print("hundredthousands: ");Serial.println(hundredthousands);
      Serial.print("millions: ");Serial.println(millions);
      Serial.print("tenmillions: ");Serial.println(tenmillions);
      Serial.print("hundredmillions: ");Serial.println(hundredmillions);
      Serial.println();*/

    lc.clearDisplay(0);                                                    // Clear Display before new Numbers are being displayed

    lc.setDigit(0, 7, (byte)tenmillions, false);
    lc.setDigit(0, 6, (byte)millions, true);
    lc.setDigit(0, 5, (byte)hundredthousands, false);
    lc.setDigit(0, 4, (byte)tenthousands, true);
    lc.setDigit(0, 3, (byte)thousands, false);
    lc.setDigit(0, 2, (byte)hundreds, true);
    lc.setDigit(0, 1, (byte)tens, false);
    lc.setDigit(0, 0, (byte)ones, false);

    delay(500);
  }
  sleepNow();
}


void wake () {                                      // interrupt service routine in sleep mode
  sleep_disable ();                                 // first thing after waking from sleep:
  detachInterrupt (digitalPinToInterrupt (2));      // stop LOW interrupt on D2
}                                                   // end of wake

void sleepNow ()
{
  lc.clearDisplay(0);
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  noInterrupts ();                                            // make sure we don't get interrupted before we sleep
  sleep_enable ();                                            // enables the sleep bit in the mcucr register
  attachInterrupt (digitalPinToInterrupt (2), wake, LOW);     // wake up on low level on D2
  interrupts ();                                              // interrupts allowed now, next instruction WILL be executed
  sleep_cpu ();                                               // here the device is put to sleep
}                                                             // end of sleepNow

void celebrate() {
  // Display Message when Countdown is over
  lc.clearDisplay(0);
  lc.setDigit(0, 7, 1, false);
  delay(800);
  lc.setDigit(0, 6, 1, false);
  delay(550);
  lc.setDigit(0, 5, 1, false);
  delay(525);
  lc.setDigit(0, 4, 1, false);
  delay(425);
  lc.setDigit(0, 3, 1, false);
  delay(325);
  lc.setDigit(0, 2, 1, false);
  delay(225);
  lc.setDigit(0, 1, 1, false);
  delay(125);
  lc.setDigit(0, 0, 1, false);
  delay(3000);
  delay(delaytime);
  lc.clearDisplay(0);
  unlock();
  //dateSetter();
  idle();

}

void unlock() {
  digitalWrite(SOLENOID, HIGH);
  delay(3000);
  digitalWrite(SOLENOID, LOW);
}

void loop() {
  updateTime();
}
