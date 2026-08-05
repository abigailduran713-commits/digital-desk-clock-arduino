#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

// ======================================================
// PIN ASSIGNMENTS
// ======================================================

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Buttons connect between these pins and GND.
const byte BTN_UP   = 2;
const byte BTN_DOWN = 3;
const byte BTN_MODE = 4;

// LED: D6 -> 220-ohm resistor -> LED anode
// LED cathode -> GND
const byte LED_REMINDER = 6;

// ======================================================
// RTC
// ======================================================

RTC_DS3231 rtc;

// ======================================================
// USER INTERFACE
// ======================================================

enum Screen {
  SCREEN_CLOCK,
  SCREEN_STATUS,
  SCREEN_INTERVAL,
  SCREEN_REMINDER_TOGGLE,
  SCREEN_SET_HOUR,
  SCREEN_SET_MINUTE,
  SCREEN_REMINDER
};

Screen screen = SCREEN_CLOCK;

// ======================================================
// REMINDER SETTINGS
// ======================================================

bool remindersEnabled = true;
byte reminderIntervalMinutes = 30;

bool reminderActive = false;
unsigned long reminderStartedAt = 0;
const unsigned long REMINDER_DURATION_MS = 6000;

int lastTriggeredMinute = -1;
int lastTriggeredHour = -1;

// ======================================================
// LED BLINK SETTINGS
// ======================================================

bool ledState = false;
unsigned long lastLedChange = 0;
const unsigned long LED_BLINK_INTERVAL_MS = 500;

// ======================================================
// BUTTON HANDLING
// ======================================================

struct Button {
  byte pin;
  bool stableState;
  bool previousReading;
  unsigned long changedAt;
  unsigned long pressedAt;
  bool longPressHandled;
};

Button upButton = {
  BTN_UP, HIGH, HIGH, 0, 0, false
};

Button downButton = {
  BTN_DOWN, HIGH, HIGH, 0, 0, false
};

Button modeButton = {
  BTN_MODE, HIGH, HIGH, 0, 0, false
};

const unsigned long DEBOUNCE_MS = 35;
const unsigned long LONG_PRESS_MS = 1500;

// ======================================================
// TIME-SETTING VALUES
// ======================================================

byte editHour24 = 0;
byte editMinute = 0;

// ======================================================
// DISPLAY HELPERS
// ======================================================

const char *dayNames[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void clearLine(byte row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void printTwoDigits(byte value) {
  if (value < 10) {
    lcd.print('0');
  }

  lcd.print(value);
}

byte convertTo12Hour(byte hour24) {
  byte hour12 = hour24 % 12;

  if (hour12 == 0) {
    hour12 = 12;
  }

  return hour12;
}

bool isPM(byte hour24) {
  return hour24 >= 12;
}

// ======================================================
// BUTTON FUNCTIONS
// ======================================================

void updateButton(Button &button) {
  bool reading = digitalRead(button.pin);

  if (reading != button.previousReading) {
    button.changedAt = millis();
    button.previousReading = reading;
  }

  if (millis() - button.changedAt >= DEBOUNCE_MS) {
    if (reading != button.stableState) {
      button.stableState = reading;

      if (button.stableState == LOW) {
        button.pressedAt = millis();
        button.longPressHandled = false;
      }
    }
  }
}

bool buttonWasReleased(Button &button) {
  static bool upPreviouslyPressed = false;
  static bool downPreviouslyPressed = false;
  static bool modePreviouslyPressed = false;

  bool *previouslyPressed = nullptr;

  if (button.pin == BTN_UP) {
    previouslyPressed = &upPreviouslyPressed;
  } else if (button.pin == BTN_DOWN) {
    previouslyPressed = &downPreviouslyPressed;
  } else {
    previouslyPressed = &modePreviouslyPressed;
  }

  if (button.stableState == LOW) {
    *previouslyPressed = true;
  }

  if (*previouslyPressed && button.stableState == HIGH) {
    *previouslyPressed = false;

    if (!button.longPressHandled) {
      return true;
    }
  }

  return false;
}

bool buttonLongPressed(Button &button) {
  if (
    button.stableState == LOW &&
    !button.longPressHandled &&
    millis() - button.pressedAt >= LONG_PRESS_MS
  ) {
    button.longPressHandled = true;
    return true;
  }

  return false;
}

// ======================================================
// TIME SETTING
// ======================================================

void beginTimeSetting() {
  DateTime now = rtc.now();

  editHour24 = now.hour();
  editMinute = now.minute();

  screen = SCREEN_SET_HOUR;
  lcd.clear();
}

void saveTimeToRTC() {
  DateTime now = rtc.now();

  // Preserve the current date and change only the time.
  // Seconds reset to zero.
  rtc.adjust(
    DateTime(
      now.year(),
      now.month(),
      now.day(),
      editHour24,
      editMinute,
      0
    )
  );

  lastTriggeredMinute = -1;
  lastTriggeredHour = -1;

  screen = SCREEN_CLOCK;
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Time saved!");

  delay(800);
  lcd.clear();
}

// ======================================================
// REMINDER FUNCTIONS
// ======================================================

void beginReminder() {
  reminderActive = true;
  reminderStartedAt = millis();
  screen = SCREEN_REMINDER;

  ledState = true;
  digitalWrite(LED_REMINDER, HIGH);

  lcd.clear();
}

void endReminder() {
  reminderActive = false;
  ledState = false;
  digitalWrite(LED_REMINDER, LOW);

  screen = SCREEN_CLOCK;
  lcd.clear();
}

void updateReminderLED() {
  if (!reminderActive) {
    ledState = false;
    digitalWrite(LED_REMINDER, LOW);
    return;
  }

  if (millis() - lastLedChange >= LED_BLINK_INTERVAL_MS) {
    lastLedChange = millis();
    ledState = !ledState;
    digitalWrite(LED_REMINDER, ledState);
  }
}

void checkReminder(const DateTime &now) {
  if (!remindersEnabled || reminderActive) {
    return;
  }

  bool correctMinute =
    now.minute() % reminderIntervalMinutes == 0;

  // Use a small time window rather than requiring the loop to
  // execute at exactly second zero.
  bool triggerWindow = now.second() < 3;

  bool notAlreadyTriggered =
    now.minute() != lastTriggeredMinute ||
    now.hour() != lastTriggeredHour;

  if (correctMinute && triggerWindow && notAlreadyTriggered) {
    lastTriggeredMinute = now.minute();
    lastTriggeredHour = now.hour();
    beginReminder();
  }
}

// ======================================================
// SCREEN DRAWING
// ======================================================

void drawClockScreen(const DateTime &now) {
  byte hour12 = convertTo12Hour(now.hour());

  clearLine(0);
  lcd.setCursor(0, 0);

  lcd.print(hour12);
  lcd.print(':');
  printTwoDigits(now.minute());
  lcd.print(isPM(now.hour()) ? " PM" : " AM");

  clearLine(1);
  lcd.setCursor(0, 1);

  lcd.print(dayNames[now.dayOfTheWeek()]);
  lcd.print(' ');
  printTwoDigits(now.month());
  lcd.print('/');
  printTwoDigits(now.day());
  lcd.print('/');
  lcd.print(now.year() % 100);
}

void drawStatusScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("Reminders: ");
  lcd.print(remindersEnabled ? "ON" : "OFF");

  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print("Every ");
  lcd.print(reminderIntervalMinutes);
  lcd.print(" min");
}

void drawIntervalScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("SET INTERVAL");

  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print(reminderIntervalMinutes);
  lcd.print(" min  UP/DOWN");
}

void drawReminderToggleScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("REMINDERS");

  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print(remindersEnabled ? "ON" : "OFF");
  lcd.print("  UP/DOWN");
}

void drawSetHourScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("SET HOUR");

  clearLine(1);
  lcd.setCursor(0, 1);

  lcd.print(convertTo12Hour(editHour24));
  lcd.print(':');
  printTwoDigits(editMinute);
  lcd.print(isPM(editHour24) ? " PM" : " AM");
}

void drawSetMinuteScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("SET MINUTE");

  clearLine(1);
  lcd.setCursor(0, 1);

  lcd.print(convertTo12Hour(editHour24));
  lcd.print(':');
  printTwoDigits(editMinute);
  lcd.print(isPM(editHour24) ? " PM" : " AM");
}

void drawReminderScreen() {
  clearLine(0);
  lcd.setCursor(0, 0);
  lcd.print("REMINDER!");

  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print("MODE=OK");
}

// ======================================================
// BUTTON ACTIONS
// ======================================================

void handleUpPress() {
  switch (screen) {
    case SCREEN_INTERVAL:
      reminderIntervalMinutes += 5;

      if (reminderIntervalMinutes > 120) {
        reminderIntervalMinutes = 5;
      }
      break;

    case SCREEN_REMINDER_TOGGLE:
      remindersEnabled = !remindersEnabled;
      break;

    case SCREEN_SET_HOUR:
      editHour24 = (editHour24 + 1) % 24;
      break;

    case SCREEN_SET_MINUTE:
      editMinute = (editMinute + 1) % 60;
      break;

    case SCREEN_REMINDER:
      // Extend the current reminder.
      reminderStartedAt = millis();
      break;

    default:
      break;
  }
}

void handleDownPress() {
  switch (screen) {
    case SCREEN_INTERVAL:
      if (reminderIntervalMinutes <= 5) {
        reminderIntervalMinutes = 120;
      } else {
        reminderIntervalMinutes -= 5;
      }
      break;

    case SCREEN_REMINDER_TOGGLE:
      remindersEnabled = !remindersEnabled;
      break;

    case SCREEN_SET_HOUR:
      editHour24 =
        editHour24 == 0 ? 23 : editHour24 - 1;
      break;

    case SCREEN_SET_MINUTE:
      editMinute =
        editMinute == 0 ? 59 : editMinute - 1;
      break;

    case SCREEN_REMINDER:
      endReminder();
      break;

    default:
      break;
  }
}

void handleModeShortPress() {
  switch (screen) {
    case SCREEN_CLOCK:
      screen = SCREEN_STATUS;
      break;

    case SCREEN_STATUS:
      screen = SCREEN_INTERVAL;
      break;

    case SCREEN_INTERVAL:
      screen = SCREEN_REMINDER_TOGGLE;
      break;

    case SCREEN_REMINDER_TOGGLE:
      screen = SCREEN_CLOCK;
      break;

    case SCREEN_SET_HOUR:
      screen = SCREEN_SET_MINUTE;
      break;

    case SCREEN_SET_MINUTE:
      saveTimeToRTC();
      return;

    case SCREEN_REMINDER:
      endReminder();
      return;
  }

  lcd.clear();
}

// ======================================================
// SETUP
// ======================================================

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);

  pinMode(LED_REMINDER, OUTPUT);
  digitalWrite(LED_REMINDER, LOW);

  Wire.begin();
  lcd.begin(16, 2);

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC NOT FOUND");

    while (true) {
      delay(10);
    }
  }

  lcd.clear();
  lcd.print("Desk Clock");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(800);
  lcd.clear();
}

// ======================================================
// MAIN LOOP
// ======================================================

void loop() {
  updateButton(upButton);
  updateButton(downButton);
  updateButton(modeButton);

  DateTime now = rtc.now();

  checkReminder(now);
  updateReminderLED();

  if (
    reminderActive &&
    millis() - reminderStartedAt >= REMINDER_DURATION_MS
  ) {
    endReminder();
  }

  // Holding MODE enters the clock-setting menu.
  if (
    screen != SCREEN_REMINDER &&
    buttonLongPressed(modeButton)
  ) {
    beginTimeSetting();
  }

  if (buttonWasReleased(upButton)) {
    handleUpPress();
    lcd.clear();
  }

  if (buttonWasReleased(downButton)) {
    handleDownPress();
    lcd.clear();
  }

  if (buttonWasReleased(modeButton)) {
    handleModeShortPress();
  }

  switch (screen) {
    case SCREEN_CLOCK:
      drawClockScreen(now);
      break;

    case SCREEN_STATUS:
      drawStatusScreen();
      break;

    case SCREEN_INTERVAL:
      drawIntervalScreen();
      break;

    case SCREEN_REMINDER_TOGGLE:
      drawReminderToggleScreen();
      break;

    case SCREEN_SET_HOUR:
      drawSetHourScreen();
      break;

    case SCREEN_SET_MINUTE:
      drawSetMinuteScreen();
      break;

    case SCREEN_REMINDER:
      drawReminderScreen();
      break;
  }

  delay(10);
}
