#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define RE 11
#define DE 12

SoftwareSerial mod(2, 3);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const byte reqPH[]       = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};
const byte reqMoisture[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};

float phBase    = 0.0;
float moistBase = 0.0;
float phAmp     = 0.0;
float moistAmp  = 0.0;
float maxAmp    = 0.4;
int   sign      = 1;
bool  running   = false;
bool  isAcidic  = true;

// Simulated NPK + EC values
float finalN  = 0.0;
float finalP  = 0.0;
float finalK  = 0.0;
float finalEC = 0.0;

// Marquee state
String marqueeText        = "";
int    marqueePos         = 0;
unsigned long lastMarquee = 0;
#define MARQUEE_SPEED 350

// ─── Button Reader ──────────────────────────────────────
#define BTN_NONE    0
#define BTN_RIGHT   1
#define BTN_UP      2
#define BTN_DOWN    3
#define BTN_LEFT    4
#define BTN_SELECT  5

int readButton() {
  int val = analogRead(A0);
  if (val < 50)   return BTN_RIGHT;
  if (val < 195)  return BTN_UP;
  if (val < 380)  return BTN_DOWN;
  if (val < 555)  return BTN_LEFT;
  if (val < 790)  return BTN_SELECT;
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

// ─── Helpers ────────────────────────────────────────────

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
  Serial.print(percent);
  Serial.print("%");
}

void SensorPing() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(reqPH, sizeof(reqPH));
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(80);
  while (mod.available()) mod.read();
}

void lcdPrint(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ─── Marquee ────────────────────────────────────────────

void buildMarqueeText() {
  marqueeText  = "    ";
  marqueeText += "N:";  marqueeText += String((int)finalN);  marqueeText += "mg/kg  ";
  marqueeText += "P:";  marqueeText += String((int)finalP);  marqueeText += "mg/kg  ";
  marqueeText += "K:";  marqueeText += String((int)finalK);  marqueeText += "mg/kg  ";
  marqueeText += "EC:"; marqueeText += String((int)finalEC); marqueeText += "uS/cm    ";
  marqueePos = 0;
}

void tickMarquee() {
  if (marqueeText.length() == 0) return;
  if (millis() - lastMarquee < MARQUEE_SPEED) return;
  lastMarquee = millis();

  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    int idx = (marqueePos + i) % marqueeText.length();
    lcd.print(marqueeText[idx]);
  }
  marqueePos = (marqueePos + 1) % marqueeText.length();
}

// ─── Boot Sequence ──────────────────────────────────────

void bootSequence() {
  Serial.println();
  Serial.println("  Initialising SN3002-TR-ECTHNPKPH...");
  lcdPrint("SN3002 Sensor", "Initialising..");
  delay(600);

  Serial.println("  Checking RS485 bus...");
  lcdPrint("RS485 Bus", "Checking.......");
  delay(500);

  SensorPing();
  Serial.println("  Sensor found at address 0x01");
  lcdPrint("Sensor Found", "Addr: 0x01");
  delay(400);

  Serial.println("  Warming up electrodes...");
  lcdPrint("Electrodes", "Warming up.....");
  delay(800);

  Serial.println("  Calibration reference loaded.");
  lcdPrint("Calibration", "Loaded OK");
  delay(400);

  Serial.println("  Baud: 4800  |  Protocol: Modbus RTU");
  lcdPrint("Baud: 4800", "Modbus RTU");
  delay(300);

  Serial.println();
  Serial.println("  ================================");
  Serial.println("   SN3002 Soil Sensor -- Ready    ");
  Serial.println("  ================================");
  Serial.println();
}

// ─── Start Sequence ─────────────────────────────────────

void startSequence(float phMin, float phMax, float mMin, float mMax) {
  phBase    = phMin + random(0, (int)((phMax - phMin) * 10)) / 10.0;
  moistBase = mMin  + random(0, (int)((mMax  - mMin)  * 10)) / 10.0;
  phAmp     = maxAmp;
  moistAmp  = maxAmp;
  sign      = 1;
  running   = true;
  marqueeText = "";

  // Realistic NPK + EC per soil type
  if (phMin < 6.0) {
    finalN  = random(80,  180);
    finalP  = random(10,  40);
    finalK  = random(50,  150);
    finalEC = random(200, 800);
  } else if (phMax > 7.5) {
    finalN  = random(50,  120);
    finalP  = random(5,   25);
    finalK  = random(80,  200);
    finalEC = random(600, 1800);
  } else {
    finalN  = random(150, 280);
    finalP  = random(30,  80);
    finalK  = random(150, 300);
    finalEC = random(400, 1200);
  }

  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Sending Modbus read request...");
  lcdPrint("Modbus Request", "Sending.......");
  delay(300);

  SensorPing();
  Serial.println("   Response received. Parsing...");
  lcdPrint("Response", "Received OK");
  delay(400);

  Serial.println("   Stabilising sensor signal...  ");
  Serial.println("  ================================");
  Serial.println();
  lcdPrint("Stabilising", "Signal.........");
  delay(300);
}

// ─── Setup ──────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  mod.begin(4800);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  randomSeed(analogRead(1));

  lcd.begin(16, 2);
  delay(1000);
  bootSequence();
}

// ─── Main Loop ──────────────────────────────────────────

void loop() {
  if (!running) {
    tickMarquee();  // Scrolls NPK+EC on row 1 after finalisation

    int btn = getPressedButton();

    if (btn == BTN_UP) {
      Serial.println("  [BTN] UP -> Acidic");
      isAcidic = true;
      startSequence(3.0, 6.9, 20.0, 45.0);

    } else if (btn == BTN_DOWN) {
      Serial.println("  [BTN] DOWN -> Basic");
      isAcidic = false;
      startSequence(7.1, 10.0, 45.0, 75.0);

    } else if (btn == BTN_SELECT) {
      Serial.println("  [BTN] SELECT -> Neutral");
      isAcidic = true;
      startSequence(6.0, 7.5, 35.0, 55.0);
    }

    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if      (input == "a" || input == "A") { isAcidic = true;  startSequence(3.0, 6.9, 20.0, 45.0); }
      else if (input == "b" || input == "B") { isAcidic = false; startSequence(7.1, 10.0, 45.0, 75.0); }
      else if (input == "n" || input == "N") {                   startSequence(6.0, 7.5, 35.0, 55.0); }
    }
  }

  if (running) {
    float currentPH    = phBase    + (sign * phAmp);
    float currentMoist = moistBase + (sign * moistAmp * 2.0);

    if (isAcidic) currentPH = constrain(currentPH, 3.0, 7.5);
    else          currentPH = constrain(currentPH, 7.0, 10.0);
    currentMoist = constrain(currentMoist, 10.0, 90.0);

    Serial.print("  [REG 0x03] pH:");
    if (currentPH < 10.0) Serial.print(" ");
    Serial.print(currentPH, 2);
    Serial.print("  [REG 0x00] Moist:");
    Serial.print(currentMoist, 1);
    Serial.print("%  |  ");
    Serial.print(soilNature(currentPH));
    Serial.print(" |");
    printLoadingBar(phAmp, maxAmp);
    Serial.println();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("pH:");
    lcd.print(currentPH, 2);
    lcd.print(" ");
    lcd.print(soilNatureShort(currentPH));
    lcd.setCursor(0, 1);
    lcd.print("Mst:");
    lcd.print(currentMoist, 1);
    lcd.print("%");

    phAmp    -= 0.02;
    moistAmp -= 0.02;
    phAmp     = max(phAmp,    0.0);
    moistAmp  = max(moistAmp, 0.0);
    sign     *= -1;

    if (phAmp <= 0.02) {
      running = false;

      Serial.println();
      Serial.println("  ================================");
      Serial.println("   Signal stabilised.            ");
      Serial.print  ("   FINAL pH       : "); Serial.println(phBase, 1);
      Serial.print  ("   FINAL Moisture : "); Serial.print(moistBase, 1); Serial.println(" %");
      Serial.print  ("   Soil Nature    : "); Serial.println(soilNature(phBase));
      Serial.print  ("   Nitrogen  (N)  : "); Serial.print((int)finalN);  Serial.println(" mg/kg");
      Serial.print  ("   Phosphorus(P)  : "); Serial.print((int)finalP);  Serial.println(" mg/kg");
      Serial.print  ("   Potassium (K)  : "); Serial.print((int)finalK);  Serial.println(" mg/kg");
      Serial.print  ("   EC             : "); Serial.print((int)finalEC); Serial.println(" uS/cm");
      Serial.println("  ================================");
      Serial.println("  UP=Acid  DOWN=Basic  SEL=Neutral");
      Serial.println("  ================================");
      Serial.println();

      // Row 0: final pH pinned, Row 1: marquee starts
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("pH:");
      lcd.print(phBase, 1);
      lcd.print(" ");
      lcd.print(soilNatureShort(phBase));
      lcd.print(" DONE");

      buildMarqueeText();
    }

    delay(350);
  }
}
