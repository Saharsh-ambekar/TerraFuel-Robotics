// ── DISPLAY ARDUINO — FULL MERGED ───────────
// Handles LCD display and keypad buttons
// Receives all status from Master Arduino
// Sends button commands to Master via SoftwareSerial
//
// PIN MAP:
//   LCD Keypad Shield → D4,D5,D6,D7,D8,D9 (built-in)
//   Buttons           → A0 (built-in on shield)
//   RX from Master    → D10
//   TX to Master      → D11
//
// BUTTON SHORTCUTS:
//   UP     → Tell Master: Simulate Acidic
//   DOWN   → Tell Master: Simulate Basic
//   SELECT → Tell Master: Simulate Neutral
//   LEFT   → Tell Master: Pump ON
//   RIGHT  → Tell Master: Pump OFF
// ─────────────────────────────────────────────

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
SoftwareSerial toMaster(10, 11); // RX=10, TX=11

// ─── Button Reader ───────────────────────────
#define BTN_NONE   0
#define BTN_RIGHT  1
#define BTN_UP     2
#define BTN_DOWN   3
#define BTN_LEFT   4
#define BTN_SELECT 5

int readButton() {
  int val = analogRead(A0);
  if (val < 50)  return BTN_RIGHT;
  if (val < 195) return BTN_UP;
  if (val < 380) return BTN_DOWN;
  if (val < 555) return BTN_LEFT;
  if (val < 790) return BTN_SELECT;
  return BTN_NONE;
}

int getPressedButton() {
  int btn = readButton();
  if (btn != BTN_NONE) {
    delay(50);
    while (readButton() != BTN_NONE);
    delay(50);
    return btn;
  }
  return BTN_NONE;
}

// ─── Helpers ─────────────────────────────────
void lcdPrint(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

// ─── Boot ────────────────────────────────────
void setup() {
  Serial.begin(9600);
  toMaster.begin(9600);
  lcd.begin(16, 2);
  delay(500);

  lcdPrint("Display Arduino", "Initialising..");
  delay(600);

  Serial.println("  ================================");
  Serial.println("   Display Arduino Ready          ");
  Serial.println("   UP=Acid DOWN=Basic SEL=Neutral ");
  Serial.println("   LEFT=PumpON RIGHT=PumpOFF       ");
  Serial.println("  ================================");

  lcdPrint("Display Ready", "Waiting Master");
}

// ─── Main Loop ───────────────────────────────
void loop() {
  // Receive and display data from Master
  if (toMaster.available()) {
    String data = toMaster.readStringUntil('\n');
    data.trim();
    int comma = data.indexOf(',');
    if (comma != -1) {
      String line1 = data.substring(0, comma);
      String line2 = data.substring(comma + 1);
      lcdPrint(line1, line2);
    }
  }

  // Button press → send command to Master
  int btn = getPressedButton();

  if      (btn == BTN_UP)     { toMaster.println("a"); Serial.println("  [BTN] UP    → Acidic  sent to Master"); }
  else if (btn == BTN_DOWN)   { toMaster.println("b"); Serial.println("  [BTN] DOWN  → Basic   sent to Master"); }
  else if (btn == BTN_SELECT) { toMaster.println("n"); Serial.println("  [BTN] SEL   → Neutral sent to Master"); }
  else if (btn == BTN_LEFT)   { toMaster.println("pump on");  Serial.println("  [BTN] LEFT  → Pump ON  sent to Master"); }
  else if (btn == BTN_RIGHT)  { toMaster.println("pump off"); Serial.println("  [BTN] RIGHT → Pump OFF sent to Master"); }
}
