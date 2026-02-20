#include <Wire.h>
#include "RTClib.h"
#include <math.h>

RTC_DS3231 rtc;

// --- 1. GLOBAL SETTINGS ---
const float MyLat = 44.654380;
const float MyLong = -84.136089;

// --- BUTTON & LIGHT SETTINGS ---
const int buttonPin = 2;     // Push button on Pin 2
const int ledPin = 6;        // MUST be PWM (3, 5, 6, 9, 10, or 11)
int brightnessLevel = 0;     
bool lastButtonState = HIGH; 

// --- 2. JULIAN DAY FUNCTION ---
long getJulianDay(int year, int month, int day) {
  if (month <= 2) {
    year -= 1;
    month += 12;
  }
  long A = year / 100;
  long B = 2 - A + (A / 4);
  return (long)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, 255); // Start at Full Power

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1); 
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("Cosmic Clock Initialized...");
}

void loop() {
  DateTime now = rtc.now();
  static int lastMinute = -1; 

  // --- BUTTON LOGIC ---
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    delay(50); // Debounce
    brightnessLevel++;
    if (brightnessLevel > 6) brightnessLevel = 0;

    if (brightnessLevel == 0) analogWrite(ledPin, 255); // 100%
    else if (brightnessLevel == 1) analogWrite(ledPin, 128); // 50%
    else if (brightnessLevel == 2) analogWrite(ledPin, 64);  // 25%
    else if (brightnessLevel == 3) analogWrite(ledPin, 32);  // 12.5%
    else if (brightnessLevel == 4) analogWrite(ledPin, 16);  // 6.25%
    else if (brightnessLevel == 5) analogWrite(ledPin, 8);  // 3.125%
    else if (brightnessLevel == 6) analogWrite(ledPin, 0);   // OFF
    
    Serial.print("Brightness Level: "); Serial.println(brightnessLevel);
  }
  lastButtonState = currentButtonState;

  // --- 3. THE 3:00 AM TRIGGER ---
  if (now.hour() == 3 && now.minute() == 0 && now.second() == 0) {
    float fractionalDay = (now.hour() / 24.0) + (now.minute() / 1440.0) + (now.second() / 86400.0);
    double JD = getJulianDay(now.year(), now.month(), now.day()) + fractionalDay - 0.5;
    float t = (JD - 2451545.0) / 36525.0;

    float L0 = 280.46646 + t * (36000.76983 + t * 0.0003032);
    float M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
    float C = (1.914602 - t * (0.004817 + 0.000014 * t)) * sin(M * DEG_TO_RAD) + (0.019993 - 0.000101 * t) * sin(2 * M * DEG_TO_RAD);
    float lambda = L0 + C;
    float epsilon = 23.439291 - t * (0.0130041);
    float delta = asin(sin(epsilon * DEG_TO_RAD) * sin(lambda * DEG_TO_RAD)) * RAD_TO_DEG;

    Serial.println("--- 3:00 AM DAILY CALCULATION ---");
    Serial.print("Declination: "); Serial.println(delta, 4);
    delay(1000); 
  }

  if (now.minute() != lastMinute) {
    lastMinute = now.minute();
    Serial.print("I'm still alive at: ");
    Serial.print(now.hour()); Serial.print(":"); 
    if(now.minute() < 10) Serial.print("0");
    Serial.println(now.minute());
  }
}