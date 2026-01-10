#include <LiquidCrystal.h>

// ===== LCD: RS, E, D4, D5, D6, D7 =====
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// ===== Buttons =====
const int BTN_UP   = 2;
const int BTN_DOWN = 3;
const int BTN_MODE = 4;

// ===== Reminder LED =====
const int LED_REMINDER = 6;

// ===== Clock base time =====
int baseHour = 12;
int baseMin  = 0;
unsigned long baseMs = 0;

// ===== Reminder settings =====
bool remindersOn = true;
int intervalMin = 30;
int lastReminderMinute = -1;
bool showReminder = false;
unsigned long reminderUntilMs = 0;

// ===== LED blinking =====
unsigned long lastBlinkMs = 0;
bool ledState = false;
const unsigned long blinkIntervalMs = 500;

// ===== UI screens =====
enum Screen {
  SCR_TIME = 0,
  SCR_STATUS,
  SCR_SET_HOUR,
  SCR_SET_MIN,
  SCR_SET_INTERVAL,
  SCR_TOGGLE_REMINDERS,
  SCR_REMINDER
};
Screen screen = SCR_TIME;

// ===== Button cooldown =====
unsigned long lastActionMs = 0;
const unsigned long actionCooldownMs = 220;

// ===== Helpers =====
void print2(int v) {
  if (v < 10) lcd.print('0');
  lcd.print(v);
}

void getTime(int &hh, int &mm, int &ss) {
  unsigned long elapsed = (millis() - baseMs) / 1000UL;
  unsigned long total = (unsigned long)baseHour * 3600UL +
                        (unsigned long)baseMin * 60UL +
                        elapsed;
  total %= 86400UL;
  hh = total / 3600UL;
  mm = (total % 3600UL) / 60UL;
  ss = total % 60UL;
}

bool canAct() {
  return millis() - lastActionMs >= actionCooldownMs;
}

void markAct() {
  lastActionMs = millis();
}

void syncBaseTime(int h, int m) {
  baseHour = (h + 24) % 24;
  baseMin  = (m + 60) % 60;
  baseMs = millis();
}

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(LED_REMINDER, OUTPUT);

  lcd.begin(16, 2);
  baseMs = millis();

  lcd.print("Digital DeskClk");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  delay(700);
  lcd.clear();
}

void loop() {
  bool upNow   = digitalRead(BTN_UP)   == LOW;
  bool downNow = digitalRead(BTN_DOWN) == LOW;
  bool modeNow = digitalRead(BTN_MODE) == LOW;

  int hh, mm, ss;
  getTime(hh, mm, ss);

  // ===== Reminder trigger =====
  if (remindersOn && screen != SCR_REMINDER) {
    if (intervalMin > 0 && (mm % intervalMin == 0) && ss == 0) {
      if (lastReminderMinute != mm) {
        lastReminderMinute = mm;
        screen = SCR_REMINDER;
        showReminder = true;
        reminderUntilMs = millis() + 6000;
        lcd.clear();
      }
    }
  }

  if (screen == SCR_REMINDER && showReminder && millis() > reminderUntilMs) {
    showReminder = false;
    screen = SCR_TIME;
    lcd.clear();
  }

  // ===== LED blinking =====
  if (screen == SCR_REMINDER && showReminder) {
    if (millis() - lastBlinkMs >= blinkIntervalMs) {
      lastBlinkMs = millis();
      ledState = !ledState;
      digitalWrite(LED_REMINDER, ledState ? HIGH : LOW);
    }
  } else {
    digitalWrite(LED_REMINDER, LOW);
    ledState = false;
  }

  // ===== Button actions =====
  if (canAct()) {
    if (modeNow) {
      if      (screen == SCR_TIME) screen = SCR_STATUS;
      else if (screen == SCR_STATUS) screen = SCR_SET_HOUR;
      else if (screen == SCR_SET_HOUR) screen = SCR_SET_MIN;
      else if (screen == SCR_SET_MIN) screen = SCR_SET_INTERVAL;
      else if (screen == SCR_SET_INTERVAL) screen = SCR_TOGGLE_REMINDERS;
      else if (screen == SCR_TOGGLE_REMINDERS) screen = SCR_TIME;
      else if (screen == SCR_REMINDER) {
        showReminder = false;
        screen = SCR_TIME;
      }
      lcd.clear();
      markAct();
    }

    if (screen == SCR_SET_HOUR) {
      if (upNow)   { syncBaseTime(baseHour + 1, baseMin); lcd.clear(); markAct(); }
      if (downNow) { syncBaseTime(baseHour - 1, baseMin); lcd.clear(); markAct(); }
    }
    else if (screen == SCR_SET_MIN) {
      if (upNow)   { syncBaseTime(baseHour, baseMin + 1); lcd.clear(); markAct(); }
      if (downNow) { syncBaseTime(baseHour, baseMin - 1); lcd.clear(); markAct(); }
    }
    else if (screen == SCR_SET_INTERVAL) {
      if (upNow)   { intervalMin = intervalMin >= 120 ? 5 : intervalMin + 5; lcd.clear(); markAct(); }
      if (downNow) { intervalMin = intervalMin <= 5 ? 120 : intervalMin - 5; lcd.clear(); markAct(); }
    }
    else if (screen == SCR_TOGGLE_REMINDERS) {
      if (upNow || downNow) { remindersOn = !remindersOn; lcd.clear(); markAct(); }
    }
    else if (screen == SCR_REMINDER) {
      if (upNow)   { reminderUntilMs = millis() + 6000; markAct(); }
      if (downNow) { showReminder = false; screen = SCR_TIME; lcd.clear(); markAct(); }
    }
  }

  // ===== Display =====
  if (screen == SCR_TIME) {
    lcd.setCursor(0,0);
    lcd.print("Time ");
    print2(hh); lcd.print(":"); print2(mm); lcd.print(":"); print2(ss);
    lcd.print("   ");
    lcd.setCursor(0,1);
    lcd.print("MODE:Menu ");
    lcd.print(remindersOn ? "ON " : "OFF");
  }
  else if (screen == SCR_STATUS) {
    lcd.setCursor(0,0);
    lcd.print("Reminders: ");
    lcd.print(remindersOn ? "ON " : "OFF");
    lcd.setCursor(0,1);
    lcd.print("Every ");
    lcd.print(intervalMin);
    lcd.print(" min     ");
  }
  else if (screen == SCR_SET_HOUR) {
    lcd.setCursor(0,0);
    lcd.print("SET HOUR");
    lcd.setCursor(0,1);
    lcd.print("Hour: ");
    print2(baseHour);
  }
  else if (screen == SCR_SET_MIN) {
    lcd.setCursor(0,0);
    lcd.print("SET MIN");
    lcd.setCursor(0,1);
    lcd.print("Min: ");
    print2(baseMin);
  }
  else if (screen == SCR_SET_INTERVAL) {
    lcd.setCursor(0,0);
    lcd.print("SET INTERVAL");
    lcd.setCursor(0,1);
    lcd.print("Every ");
    lcd.print(intervalMin);
    lcd.print(" min");
  }
  else if (screen == SCR_TOGGLE_REMINDERS) {
    lcd.setCursor(0,0);
    lcd.print("REMINDERS");
    lcd.setCursor(0,1);
    lcd.print(remindersOn ? "ON " : "OFF");
    lcd.print(" UP/DN");
  }
  else if (screen == SCR_REMINDER) {
    lcd.setCursor(0,0);
    lcd.print("REMINDER!");
    lcd.setCursor(0,1);
    lcd.print("Hydrate  MODE");
  }

  delay(40);
}
