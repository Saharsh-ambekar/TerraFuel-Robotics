// ── DISPLAY ARDUINO — pH + LCD ──────────────
// Handles LCD display and keypad buttons
// Receives data from Master Arduino via SoftwareSerial
// Sends commands back to Master via SoftwareSerial
//
// PIN MAP:
//   LCD Keypad Shield → D4,D5,D6,D7,D8,D9 (built-in)
//   Buttons           → A0 (built-in on shield)
//   RX from Master    → D10
//   TX to Master      → D11
//
// BUTTON SHORTCUTS:
//   UP     → Simulate Acidic  soil
//   DOWN   → Simulate Basic   soil
//   SELECT → Simulate Neutral soil
// ─────────────────────────────────────────────

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
SoftwareSerial fromMaster(10, 11); // RX=10, TX=11

// ─── Simulation State ────────────────────────
float phBase = 0.0;
float phAmp  = 0.0;
float maxAmp = 0.4;
int   sign   = 1;
bool  running = false;

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
String soilNature(float ph) {
  if (ph < 6.0)       return "Acidic  ";
  else if (ph <= 7.5) return "Neutral ";
  else                return "Alkaline";
}

String soilNatureShort(float ph) {
  if (ph < 6.0)       return "Acid";
  else if (ph <= 7.5) return "Neut";
  else                return "Alka";
}

void lcdPrint(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

void printLoadingBar(float current, float maxVal) {
  int percent = (int)(((maxVal - current) / maxVal) * 100);
  int filled  = percent / 5;
  Serial.print(" [");
  for (int i = 0; i < 20; i++) {
    if (i < filled)       Serial.print("=");
    else if (i == filled) Serial.print(">");
    else                  Serial.print(".");
  }
  Serial.print("] ");
  if (percent < 10)       Serial.print("  ");
  else if (percent < 100) Serial.print(" ");
  Serial.print(percent); Serial.print("%");
}

// ─── pH Simulation ───────────────────────────
void startSequence(float phMin, float phMax) {
  phBase  = phMin + random(0, (int)((phMax - phMin) * 10)) / 10.0;
  phAmp   = maxAmp;
  sign    = 1;
  running = true;

  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Sending read request...");
  lcdPrint("Request", "Sending.......");
  delay(300);
  Serial.println("   Response received. Parsing...");
  lcdPrint("Response", "Received OK");
  delay(400);
  Serial.println("   Stabilising sensor signal...  ");
  Serial.println("  ================================");
  lcdPrint("Stabilising", "Signal.........");
  delay(300);
}

// ─── Boot ────────────────────────────────────
void setup() {
  Serial.begin(9600);
  fromMaster.begin(9600);
  randomSeed(analogRead(1));
  lcd.begin(16, 2);
  delay(500);

  lcdPrint("pH Display", "Initialising..");
  delay(600);
  lcdPrint("Electrodes", "Warming up.....");
  delay(800);
  lcdPrint("Calibration", "Loaded OK");
  delay(400);

  Serial.println("  ================================");
  Serial.println("   pH Display Arduino Ready       ");
  Serial.println("   UP=Acid DOWN=Basic SEL=Neutral ");
  Serial.println("  ================================");

  lcdPrint("Display Ready", "UP/DN/SEL");
}

// ─── Main Loop ───────────────────────────────
void loop() {
  // Receive status from Master and show on LCD
  if (fromMaster.available()) {
    String data = fromMaster.readStringUntil('\n');
    data.trim();
    int comma = data.indexOf(',');
    if (comma != -1) {
      String line1 = data.substring(0, comma);
      String line2 = data.substring(comma + 1);
      lcdPrint(line1, line2);
    }
  }

  if (!running) {
    int btn = getPressedButton();

    if      (btn == BTN_UP)     { Serial.println("  [BTN] UP -> Acidic");   startSequence(3.0, 6.9);  }
    else if (btn == BTN_DOWN)   { Serial.println("  [BTN] DOWN -> Basic");  startSequence(7.1, 10.0); }
    else if (btn == BTN_SELECT) { Serial.println("  [BTN] SEL -> Neutral"); startSequence(6.0, 7.5);  }

    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if      (input == "a" || input == "A") startSequence(3.0, 6.9);
      else if (input == "b" || input == "B") startSequence(7.1, 10.0);
      else if (input == "n" || input == "N") startSequence(6.0, 7.5);
    }
  }

  if (running) {
    float currentPH = phBase + (sign * phAmp);
    currentPH = constrain(currentPH, 0.0, 14.0);

    Serial.print("  [pH] ");
    if (currentPH < 10.0) Serial.print(" ");
    Serial.print(currentPH, 2);
    Serial.print("  |  ");
    Serial.print(soilNature(currentPH));
    Serial.print(" |");
    printLoadingBar(phAmp, maxAmp);
    Serial.println();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("pH: "); lcd.print(currentPH, 2);
    lcd.print(" "); lcd.print(soilNatureShort(currentPH));
    lcd.setCursor(0, 1);
    lcd.print("Stabilising...");

    phAmp -= 0.02;
    phAmp  = max(phAmp, 0.0);
    sign  *= -1;

    if (phAmp <= 0.02) {
      running = false;

      Serial.println();
      Serial.println("  ================================");
      Serial.println("   Signal stabilised.            ");
      Serial.print  ("   FINAL pH     : "); Serial.println(phBase, 2);
      Serial.print  ("   Soil Nature  : "); Serial.println(soilNature(phBase));
      Serial.println("  ================================");
      Serial.println("  UP=Acid  DOWN=Basic  SEL=Neutral");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("pH:"); lcd.print(phBase, 2);
      lcd.print(" "); lcd.print(soilNatureShort(phBase));
      lcd.print(" DONE");
      lcd.setCursor(0, 1);
      lcd.print(soilNature(phBase));
    }

    delay(350);
  }
}
